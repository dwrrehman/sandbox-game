// C client for the sandbox game, using GLFW3 and openGL.

#include <iso646.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>
#include <time.h>

#define GLFW_INCLUDE_GLCOREARB
#include <GLFW/glfw3.h>

#include "atlas.h"

#include "glad/glad.h"


// temporary:

static uint32_t gle = 0;
#define cc	do { gle = glGetError(); if (gle != GL_NO_ERROR) { printf("%s:l%u:e%u\n", __FILE__, __LINE__, gle);  exit(1); } } while(0);


static const float fovy = 1.22173f /*radians*/;
static const float znear = 0.01f;
static const float zfar = 1000.0f;

static const float camera_sensitivity = 0.005f;
static const float camera_accel = 0.00003f;
static const float drag = 0.95f;

static const int32_t ms_delay_per_frame = 8;

static const char* vertex_shader_code = "        			\n\
#version 330 core							\n\
									\n\
attribute vec3 position;                                		\n\
attribute vec2 input_uv;                               			\n\
									\n\
varying vec2 output_uv;							\n\
uniform mat4 matrix;							\n\
                                          				\n\
void main() {                                				\n\
	gl_Position = matrix * vec4(position, 1.0);              	\n\
	output_uv = input_uv;                           		\n\
}                                                       		\n";

static const char* fragment_shader_code = "				\n\
#version 330 core							\n\
									\n\
varying vec2 output_uv;							\n\
uniform sampler2D atlas_texture;					\n\
									\n\
void main() {								\n\
	gl_FragColor.rbg = texture2D(atlas_texture, output_uv).rbg;	\n\
	gl_FragColor.a = 1.0;						\n\
}									\n";

// 
// sampler2D uTexture
// void main() {
// 	vec4 textureColor = texture(uTexture, aTexCoords);
//
//}

struct vec3 {float x,y,z;};

typedef float* mat4;

static inline void perspective(mat4 result, float fov, float asp, float zNear, float zFar) {
	const float t = tanf(fov / 2.0f);
	result[4 * 0 + 0] = 1.0f / (asp * t);
	result[4 * 1 + 1] = 1.0f / t;
	result[4 * 2 + 2] = -(zFar + zNear) / (zFar - zNear);
	result[4 * 2 + 3] = -1.0f;
	result[4 * 3 + 2] = -(2.0f * zFar * zNear) / (zFar - zNear);
}

static inline float inversesqrt(float y) {
	float x2 = y * 0.5f;
	int32_t i = *(int32_t *)&y;
	i = 0x5f3759df - (i >> 1); 	// glm uses a86 for last three digits.
	y = *(float*) &i;
	return y * (1.5f - x2 * y * y);
}

static inline struct vec3 normalize(struct vec3 v) {
   float s = inversesqrt(v.x * v.x + v.y * v.y + v.z * v.z);
   return (struct vec3) {v.x * s, v.y * s, v.z * s};
}

static inline struct vec3 cross(struct vec3 x, struct vec3 y) {
	return (struct vec3) {
		x.y * y.z - y.y * x.z,
		x.z * y.x - y.z * x.x,
		x.x * y.y - y.x * x.y
	};
}

static inline float dot(struct vec3 a, struct vec3 b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline void look_at(mat4 result, struct vec3 eye, struct vec3 f, struct vec3 up) {
	struct vec3 s = normalize(cross(f, up));
	struct vec3 u = cross(s, f);

	result[4 * 0 + 0] =  s.x;
	result[4 * 1 + 0] =  s.y;
	result[4 * 2 + 0] =  s.z;
	result[4 * 0 + 1] =  u.x;
	result[4 * 1 + 1] =  u.y;
	result[4 * 2 + 1] =  u.z;
	result[4 * 0 + 2] = -f.x;
	result[4 * 1 + 2] = -f.y;
	result[4 * 2 + 2] = -f.z;
	result[4 * 3 + 0] = -dot(s, eye);
	result[4 * 3 + 1] = -dot(u, eye);
	result[4 * 3 + 2] =  dot(f, eye);
	result[4 * 3 + 3] =  1;
}

static inline void multiply_matrix(mat4 out, mat4 A, mat4 B) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            out[4 * i + j] = 
		A[4 * i + 0] * B[4 * 0 + j] + 
		A[4 * i + 1] * B[4 * 1 + j] + 
		A[4 * i + 2] * B[4 * 2 + j] + 
		A[4 * i + 3] * B[4 * 3 + j];
        }
    }
}

static int window_width = 1600;
static int window_height = 1000;
static float aspect = 1.6f;

static bool debug = false;
static bool quit = false;
static bool tab = false;
static bool should_move_camera = true;
static bool is_fullscreen = false;
static int counter = 0;
static float delta = 0.0;

static float pitch = 0.0f, yaw = 0.0f;
static struct vec3 position = {10, 5, 10};
static struct vec3 velocity = {0, 0, 0};

static struct vec3 forward = 	{0, 0, -1};
static struct vec3 straight = 	{0, 0, 1};
static struct vec3 up = 	{0, 1, 0};
static struct vec3 right = 	{-1, 0, 0};

static inline void move_camera(void) {
	const float pi_over_2 = 1.57079632679f;
	if (pitch > pi_over_2) pitch = pi_over_2 - 0.0001f;
	else if (pitch < -pi_over_2) pitch = -pi_over_2 + 0.0001f;

	forward.x = -sinf(yaw) * cosf(pitch);
	forward.y = -sinf(pitch);
	forward.z = -cosf(yaw) * cosf(pitch);
	forward = normalize(forward);

	right.x = -cosf(yaw);
	right.y = 0.0;
	right.z = sinf(yaw);
	right = normalize(right);
	straight = cross(right, up);
}



static const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";
static const char *fragmentShader1Source = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\n\0";



static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

int main(void) { // real main. 

	srand((unsigned)time(NULL));

	if (not glfwInit()) exit(1);

#ifdef __APPLE__
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);


	GLFWwindow* window = glfwCreateWindow( 1280, 720, "block game", NULL, NULL );
	if (not window) { glfwTerminate(); exit(1); }

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	if (not gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) exit(1);

	glEnable(GL_DEPTH_TEST);
	cc;
	glFrontFace(GL_CCW);
	cc;
	//glCullFace(GL_BACK);
	cc;
	//glEnable(GL_CULL_FACE);
	cc;
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //GL_FRONT
	cc;

	printf("glGetString(GL_VERSION) = %s\n", glGetString(GL_VERSION)); // debug


	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fragmentShaderOrange = glCreateShader(GL_FRAGMENT_SHADER); cc;
	unsigned int shaderProgramOrange = glCreateProgram(); cc;
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);cc;
	glCompileShader(vertexShader);cc;
	glShaderSource(fragmentShaderOrange, 1, &fragmentShader1Source, NULL);cc;
	glCompileShader(fragmentShaderOrange);cc;
	glAttachShader(shaderProgramOrange, vertexShader);cc;
	glAttachShader(shaderProgramOrange, fragmentShaderOrange);cc;
	glLinkProgram(shaderProgramOrange);cc;
	
	

	float firstTriangle[] = {
		-0.9f, -0.5f, 0.0f, 
		-0.0f, -0.5f, 0.0f, 
		-0.45f, 0.5f, 0.0f, 
	};
	float secondTriangle[] = {
		0.0f, -0.5f, 0.0f,  
		0.9f, -0.5f, 0.0f,  
		0.45f, 0.5f, 0.0f   
	};

	unsigned int VBOs[2], VAOs[2];
	glGenVertexArrays(2, VAOs); cc;
	glGenBuffers(2, VBOs);cc;

	glBindVertexArray(VAOs[0]);cc;
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);cc;
	glBufferData(GL_ARRAY_BUFFER, sizeof(firstTriangle), firstTriangle, GL_STATIC_DRAW);cc;
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);cc;
	glEnableVertexAttribArray(0);cc;

	glBindVertexArray(VAOs[1]);cc;
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);cc;
	glBufferData(GL_ARRAY_BUFFER, sizeof(secondTriangle), secondTriangle, GL_STATIC_DRAW);cc;
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);cc;
	glEnableVertexAttribArray(0);cc;

	while (not glfwWindowShouldClose(window)) {

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
		if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); 
		if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 

		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) puts("activate_airship();");
		if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) puts("destroy_all();");
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) puts("bubbles();");

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);cc;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);cc;
		glUseProgram(shaderProgramOrange);cc;
		glBindVertexArray(VAOs[0]);cc;
		glDrawArrays(GL_TRIANGLES, 0, 3);cc;
		glUseProgram(shaderProgramOrange);cc;
		glBindVertexArray(VAOs[1]);cc;
		glDrawArrays(GL_TRIANGLES, 0, 3);cc;
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glDeleteVertexArrays(2, VAOs);
	glDeleteBuffers(2, VBOs);
	glDeleteProgram(shaderProgramOrange);
	glfwTerminate();
}





















/*

	while (not quit) {

		//printf("player position = x=%lf y=%lf z=%lf             ", (double) position.x, (double) position.y, (double) position.z);
		//printf("player velocity = v.x=%lf v.y=%lf v.z=%lf\n", (double) velocity.x, (double) velocity.y, (double) velocity.z);

		cc;
		uint32_t start = SDL_GetTicks();

		SDL_Event event;
		while (SDL_PollEvent(&event)) {

			const Uint8* key = SDL_GetKeyboardState(0);

			if (event.type == SDL_QUIT) quit = true;

			else if (event.type == SDL_WINDOWEVENT_RESIZED) {
				printf("window was resized!!\n");
				int w=0,h=0;
				SDL_GetWindowSize(window, &w, &h);
				printf("width = %d, height = %d", w,h);
				window_width = w;
				window_height = h;
				aspect = (float) window_width / (float) window_height;
				perspective(perspective_matrix, fovy, aspect, znear, zfar);
			}

			else if (event.type == SDL_MOUSEMOTION and should_move_camera) {
				
    				float dx = (float) event.motion.xrel;
    				float dy = (float) event.motion.yrel;

				yaw -= camera_sensitivity * dx;
				pitch += camera_sensitivity * dy;
	
				move_camera();
			}

			else if (event.type == SDL_KEYDOWN) {
				if (key[SDL_SCANCODE_ESCAPE]) quit = true;
				if (tab and key[SDL_SCANCODE_Q]) quit = true; 
				if (tab and key[SDL_SCANCODE_0]) debug = !debug;	
				if (tab and key[SDL_SCANCODE_1]) { // pause game.
					should_move_camera = not should_move_camera;
					SDL_SetRelativeMouseMode(should_move_camera);
				}
				if (tab and key[SDL_SCANCODE_2]) {
					is_fullscreen = not is_fullscreen;
					SDL_SetWindowFullscreen(window, is_fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
					int w=0,h =0;
					SDL_GetWindowSize(window, &w, &h);

					printf("width = %d, height = %d", w,h);
					window_width = w;
					window_height = h;
					aspect = (float) window_width / (float) window_height;
					perspective(perspective_matrix, fovy, aspect, znear, zfar);
				}
			}
		}

		const Uint8* key = SDL_GetKeyboardState(0);
		
		tab = !!key[SDL_SCANCODE_TAB];

		if (key[SDL_SCANCODE_SPACE]) {
			
			velocity.x += delta * camera_accel * up.x;
			velocity.y += delta * camera_accel * up.y;
			velocity.z += delta * camera_accel * up.z;
		}

		if (key[SDL_SCANCODE_A]) { 
			
			velocity.x -= delta * camera_accel * up.x;
			velocity.y -= delta * camera_accel * up.y;
			velocity.z -= delta * camera_accel * up.z;
		}

		if (key[SDL_SCANCODE_E]) { 
			
			velocity.x += delta * camera_accel * straight.x;
			velocity.y += delta * camera_accel * straight.y;
			velocity.z += delta * camera_accel * straight.z;
		}
		if (key[SDL_SCANCODE_D]) { 
			
			velocity.x -= delta * camera_accel * straight.x;
			velocity.y -= delta * camera_accel * straight.y;
			velocity.z -= delta * camera_accel * straight.z;
		}

		if (key[SDL_SCANCODE_S]) {
			
			velocity.x += delta * camera_accel * right.x;
			velocity.y += delta * camera_accel * right.y;
			velocity.z += delta * camera_accel * right.z;
		}
		
		if (key[SDL_SCANCODE_F]) { 
			
			velocity.x -= delta * camera_accel * right.x;
			velocity.y -= delta * camera_accel * right.y;
			velocity.z -= delta * camera_accel * right.z;
		}

		if (key[SDL_SCANCODE_L]) { 
			yaw -= 0.08f;
			move_camera();
		}
		if (key[SDL_SCANCODE_J]) { 
			yaw += 0.08f;
			move_camera();
		}
		if (key[SDL_SCANCODE_I]) { 
			pitch -= 0.08f;
			move_camera();
		}
		if (key[SDL_SCANCODE_K]) { 
			pitch += 0.08f;
			move_camera();
		}

		glClearColor(0.0f, 0.5f, 0.0f, 1.0f);
		cc;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		cc;
		// generate new verticies (ie, a new mesh), then say this:

		glBufferData(
			GL_ARRAY_BUFFER, 
			(GLsizeiptr)((size_t)vertex_count * 5 * sizeof(float)), 
			verticies, 
			GL_STATIC_DRAW
		);
		cc;

	//	glBufferData(
	//		GL_ELEMENT_ARRAY_BUFFER, 
	//		(GLsizeiptr)((size_t)index_count * sizeof(unsigned)), 
	//		indicies, 
	//		GL_STATIC_DRAW
	//	);



		look_at(view_matrix, position, forward, up);

		memset(matrix, 0, 64);
		matrix[4 * 0 + 0] = 1.0;
		matrix[4 * 1 + 1] = 1.0;
		matrix[4 * 2 + 2] = 1.0;
		matrix[4 * 3 + 3] = 1.0;

		memcpy(copy, matrix, 64);
		multiply_matrix(matrix, copy, view_matrix);
		memcpy(copy, matrix, 64);
		multiply_matrix(matrix, copy, perspective_matrix);

		glUniformMatrix4fv(matrix_uniform, 1, GL_FALSE, matrix);
		cc;

		glDrawArrays(key[SDL_SCANCODE_GRAVE] ? GL_LINES : GL_TRIANGLES, 0, vertex_count);
		cc;


		//glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);


		SDL_GL_SwapWindow(window);

		velocity.x *= drag;
		velocity.y *= drag;
		velocity.z *= drag;

		position.x += delta * velocity.x;
		position.y += delta * velocity.y;
		position.z += delta * velocity.z;

		const int32_t sleep = ms_delay_per_frame - ((int32_t) SDL_GetTicks() - (int32_t) start);
		if (sleep > 0) SDL_Delay((uint32_t) sleep);
		delta = (float) ((int32_t) SDL_GetTicks() - (int32_t) start);

		if (counter == 200) counter = 0;
		else counter++;

		if (counter == 0) {
			double fps = 1 / ((double) (SDL_GetTicks() - start) / 1000.0);
			// printf("fps = %10.10lf\n", fps);
		}

		if (debug) {
			printf("DEBUG: [%s]\n", tab ? "tab" : "   ");
			printf("position = {%3.3lf, %3.3lf, %3.3lf}\n", (double)position.x,(double)position.y,(double)position.z);
			printf("velocity = {%3.3lf, %3.3lf, %3.3lf}\n", (double)velocity.x,(double)velocity.y,(double)velocity.z);
			printf("yaw = %3.3lf, pitch = %3.3lf\n", (double)yaw, (double)pitch);
			printf("forward = {%3.3lf, %3.3lf, %3.3lf}\n", (double)forward.x,(double)forward.y,(double)forward.z);
			printf("right = {%3.3lf, %3.3lf, %3.3lf}\n", (double)right.x,(double)right.y,(double)right.z);
			printf("up = {%3.3lf, %3.3lf, %3.3lf}\n", (double)up.x,(double)up.y,(double)up.z);
		}
	}

	//glDeleteVertexArrays(1, &vertex_array);

	glDeleteBuffers(1, &vertex_array_buffer);
	cc;
	glDetachShader(program, vertex_shader);
	cc;
	glDeleteShader(vertex_shader);
	cc;
	glDetachShader(program, fragment_shader);
	cc;
	glDeleteShader(fragment_shader);
	cc;
	glDeleteProgram(program);
	cc;

	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();


*/




















































/*
	glEnable(GL_TEXTURE_2D);
	cc;

	uint32_t texture_id;
	glGenTextures(1, &texture_id);
	cc;

	glActiveTexture(GL_TEXTURE0); 
	cc;

	glBindTexture(GL_TEXTURE_2D, texture_id);
	cc;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	cc;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	cc;

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	cc;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	cc;

	uint8_t pixel_bytes[64 * 64 * 4] = {0};
	for (unsigned i = 0; i < 64 * 64; i++) {
		pixel_bytes[i * 4 + 0] = 0xff & (atlas[i] >> 16);
		pixel_bytes[i * 4 + 1] = 0xff & (atlas[i] >> 8);
		pixel_bytes[i * 4 + 2] = 0xff & (atlas[i] >> 0);
		pixel_bytes[i * 4 + 3] = 0xff;
	}

	GLint internal_format = GL_RGBA32F;
	GLsizei width = 64;
	GLsizei height = 64;
	uint32_t format = GL_RGBA;
	uint32_t type = GL_UNSIGNED_BYTE;
	
	glTexImage2D(
		GL_TEXTURE_2D, 0, internal_format, 
		width, height, 0, format, type, pixel_bytes
	); 
	cc;
	glGenerateMipmap(GL_TEXTURE_2D);
	cc;
	//int p_dx = 64 / 8; // pixels of each tile in x
	//int p_dy = 64 / 8; // pixels of each tile in y
	// int tiles = 8 * 8; // number of tiles total
	//glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA32F, p_dx, p_dy, tiles); 
	//cc;
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 64); // width
	cc;
	glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 64); // height
	cc;
	for (int x = 0; x < 8; x++) {
		for (int y = 0; y < 8; y++) {
			
			//glTexSubImage2D(
			//	GL_TEXTURE_2D_ARRAY, 0, 0, 0, 
			//	x * 8 + y, p_dx, p_dy, 1, 
			//	GL_RGBA, GL_UNSIGNED_BYTE, 
			//	pixel_bytes + (x * p_dy * 8 + y * p_dx) * 4
			//);
			//cc;
			

			glTexSubImage2D(GL_TEXTURE_2D, 0, 
					x, y, 
					8, 8,
					GL_RGBA,
					GL_UNSIGNED_BYTE,
					pixel_bytes
			);
			cc;
		}
	}
	

	////
	for (int x = 0; x < 8; x++) {
		for (int y = 0; y < 8; y++) {

			// glTexSubImage3D(...);
			// cc;

			// example usage:

			 glTexSubImage3D(
				GL_TEXTURE_2D_ARRAY, 
				0, 0, 0, 
				x * 8 + y, 
				8, 8, 
				1, 
				GL_RBGA, 
				GL_UNSIGNED_BYTE, 
				mat->pixels + (x * 64 + y * 8) * 4
			);
		}
	}
	///


	
	GLint success = 0;
	GLchar error[1024] = {0};
	GLuint program = glCreateProgram();cc;
	
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER); cc;
	const GLchar* vs_sources[1] = {vertex_shader_code};
	GLint vs_lengths[1] = {(GLint)strlen(vertex_shader_code)};
	glShaderSource(vertex_shader, 1, vs_sources, vs_lengths);cc;
	glCompileShader(vertex_shader);cc;
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);cc;
	glGetShaderInfoLog(vertex_shader, sizeof(error), NULL, error);cc;
	if (not success) {
		printf("vertex shader compile error: %s\n", error);
		exit(1);
	}
	glAttachShader(program, vertex_shader);
	cc;

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);cc;
	const GLchar* fs_sources[1] = {fragment_shader_code};
	GLint fs_lengths[1] = {(GLint)strlen(fragment_shader_code)};
	glShaderSource(fragment_shader, 1, fs_sources, fs_lengths);cc;
	glCompileShader(fragment_shader);cc;
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);cc;
	glGetShaderInfoLog(fragment_shader, sizeof(error), NULL, error);cc;
	if (not success) {
		printf("fragment shader compile error: %s\n", error);
		exit(1);
	}
	glAttachShader(program, fragment_shader);
	cc;
	
	glLinkProgram(program);cc;
	glGetProgramiv(program, GL_LINK_STATUS, &success);cc;
	glGetProgramInfoLog(program, sizeof(error), NULL, error);cc;
	if (not success) {
		printf("program link error: %s\n", error);
		exit(1);
	}
	glValidateProgram(program);cc;
	glGetProgramiv(program, GL_VALIDATE_STATUS, &success);cc;
	glGetProgramInfoLog(program, sizeof(error), NULL, error);cc;
	if (not success) {
		printf("program validate error: %s\n", error);
		exit(1);
	}
	glUseProgram(program);
	cc;

	GLint matrix_uniform = glGetUniformLocation(program, "matrix");
	cc;
	//GLint texture_uniform = glGetUniformLocation(program, "atlas_texture");
	//cc;

	//glUniform1i(texture_uniform, 0);
	//cc;





	GLuint vertex_array_buffer;
	glGenBuffers(1, &vertex_array_buffer);
	cc;
	glBindBuffer(GL_ARRAY_BUFFER, vertex_array_buffer);
	cc;

	enum {attribute_position, attribute_uv};

	glVertexAttribPointer(attribute_position, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	cc;
	glEnableVertexAttribArray(attribute_position);
	cc;

	glVertexAttribPointer(attribute_uv, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	cc;
	glEnableVertexAttribArray(attribute_uv);
	cc;
	
	float* view_matrix = calloc(16, 4);
	float* perspective_matrix = calloc(16, 4);
	float* matrix = calloc(16, 4);
	float* copy = calloc(16, 4);

	straight = cross(right, up);
	perspective(perspective_matrix, fovy, aspect, znear, zfar);


	//GLuint element_buffer;
	//glGenBuffers(1, &element_buffer);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer);
	





*/



























/*

	const int s = 20;
	const int space_count = s * s * s;
	int8_t* space = calloc(space_count, 1);


	// set a flat world:
	for (int x = 1; x < s; x++) {
		for (int z = 1; z < s; z++) {
			const int y = 0;
			space[s * s * x + s * y + z] = 1;
		}
	}

	// // make a 2x2 box:
	space[s * s * 1 + s * 1 + 1] = 1;
	space[s * s * 1 + s * 1 + 2] = 1;
	space[s * s * 1 + s * 2 + 1] = 1;
	space[s * s * 1 + s * 2 + 2] = 1;
	space[s * s * 2 + s * 1 + 1] = 1;
	space[s * s * 2 + s * 1 + 2] = 1;
	space[s * s * 2 + s * 2 + 1] = 1;
	space[s * s * 2 + s * 2 + 2] = 1;


	
	// and a random block:
	space[s * s * 4 + s * 4 + 4] = 1;

	

	//space[s * s * 0 + s * 0 + 0] = 1;



#define push_vertex(xo, yo, zo, u, v) 			\
	verticies[raw_count++] = (float)x + xo;		\
	verticies[raw_count++] = (float)y + yo;		\
	verticies[raw_count++] = (float)z + zo;		\
	verticies[raw_count++] = (float) u;		\
	verticies[raw_count++] = (float) v;		\
	vertex_count++;


	GLsizei vertex_count = 0, raw_count = 0;//, index_count = 0;

	//unsigned* indicies = malloc(sizeof(unsigned) * space_count * 144);	
	float* verticies = malloc(sizeof(float) * space_count * 144);


	//float top_x[256] 	= {	0	};
	//float top_y[256] 	= {	0	};

	//float bottom_x[256] 	= {	0	};
	//float bottom_y[256] 	= {	0	};
	//float sides_x[256] 	= {	1	};
	//float sides_y[256] 	= {	0	};


	for (int x = 0; x < s; x++) {
		for (int y = 0; y < s; y++) {
			for (int z = 0; z < s; z++) {
				int8_t block = space[s * s * x + s * y + z];
				if (not block) continue;
				block--;

				//const float ut = 0;//(float) top_x[block] / 64.0f;
				//const float vt = 0;//(float) top_y[block] / 64.0f;

				//const float ub = (float) bottom_x[block] / 64.0f;
				//const float vb = (float) bottom_y[block] / 64.0f;
				//const float us = (float) sides_x[block] / 64.0f;
				//const float vs = (float) sides_y[block] / 64.0f;
				
				//const float e = 1;//8.0f / 64.0f;
				//const float _ = 0;
				
				if (not z or not space[s * s * (x) + s * (y) + (z - 1)]) {
					push_vertex(0,0,0, ut+_,vt+_);
					push_vertex(0,1,0, ut+_,vt+e);
					push_vertex(1,0,0, ut+e,vt+_);
					push_vertex(1,1,0, ut+e,vt+e);
					push_vertex(1,0,0, ut+e,vt+_);
					push_vertex(0,1,0, ut+_,vt+e);
				}

				if (z >= s - 1 or not space[s * s * (x) + s * (y) + (z + 1)]) {
					push_vertex(0,0,1, ut+_,vt+_);
					push_vertex(1,0,1, ut+_,vt+e);
					push_vertex(0,1,1, ut+e,vt+_);
					push_vertex(1,1,1, ut+e,vt+e);
					push_vertex(0,1,1, ut+e,vt+_);
					push_vertex(1,0,1, ut+_,vt+e);
				}

				if (x >= s - 1 or not space[s * s * (x + 1) + s * (y) + (z)]) {
					push_vertex(1,1,1, ut+_,vt+_);
					push_vertex(1,0,0, ut+_,vt+e);
					push_vertex(1,1,0, ut+e,vt+_);
					push_vertex(1,1,1, ut+e,vt+e);
					push_vertex(1,0,1, ut+e,vt+_);
					push_vertex(1,0,0, ut+_,vt+e);
				}

				if (not x or not space[s * s * (x - 1) + s * (y) + (z)]) {
					push_vertex(0,1,1, ut+_,vt+_);
					push_vertex(0,1,0, ut+_,vt+e);
					push_vertex(0,0,0, ut+e,vt+_);
					push_vertex(0,1,1, ut+e,vt+e);
					push_vertex(0,0,0, ut+e,vt+_);
					push_vertex(0,0,1, ut+_,vt+e);
				}

				if (not y or not space[s * s * (x) + s * (y - 1) + (z)]) {
					push_vertex(1,0,1, ut+_,vt+_);
					push_vertex(0,0,1, ut+_,vt+e);
					push_vertex(1,0,0, ut+e,vt+_);
					push_vertex(0,0,0, ut+e,vt+e);
					push_vertex(1,0,0, ut+e,vt+_);
					push_vertex(0,0,1, ut+_,vt+e);
				}

				if (y >= s - 1 or not space[s * s * (x) + s * (y + 1) + (z)]) {
					push_vertex(1,1,1, ut+_,vt+_);
					push_vertex(1,1,0, ut+_,vt+e);
					push_vertex(0,1,1, ut+e,vt+_);
					push_vertex(0,1,0, ut+e,vt+e);
					push_vertex(0,1,1, ut+e,vt+_);
					push_vertex(1,1,0, ut+_,vt+e);
				}
				
			}
		}
	}

	const float e = 1;// 8.0f / 64.0f;
	const float b = 0;

	int x = 0, y = 0, z = 0;

	push_vertex(0,0,0, b,b);
	push_vertex(0,1,0, e,b);
	push_vertex(1,0,0, b,e);
	push_vertex(1,1,0, e,e);
	push_vertex(1,0,0, b,e);
	push_vertex(0,1,0, e,b);


	push_vertex(0,0,1, b,b);
	push_vertex(1,0,1, e,b);
	push_vertex(0,1,1, b,e);
	push_vertex(1,1,1, e,e);
	push_vertex(0,1,1, b,e);
	push_vertex(1,0,1, e,b);

	push_vertex(1,1,1, b,b);
	push_vertex(1,0,1, e,b);
	push_vertex(1,1,0, b,e);
	push_vertex(1,0,0, e,e);
	push_vertex(1,1,0, b,b);
	push_vertex(1,0,1, e,b);







	x = 1; y = 1; z = 1;

	push_vertex(0,0,0, b,b);
	push_vertex(0,1,0, e,b);
	push_vertex(1,0,0, b,e);
	push_vertex(1,1,0, e,e);
	push_vertex(1,0,0, b,e);
	push_vertex(0,1,0, e,b);





	push_vertex(0,0,1, b,b);
	push_vertex(1,0,1, e,b);
	push_vertex(0,1,1, b,e);
	push_vertex(1,1,1, e,e);
	push_vertex(0,1,1, b,e);
	push_vertex(1,0,1, e,b);

	push_vertex(1,1,1, b,b);
	push_vertex(1,0,1, e,b);
	push_vertex(1,1,0, b,e);
	push_vertex(1,0,0, e,e);
	push_vertex(1,1,0, b,b);
	push_vertex(1,0,1, e,b);


*/





