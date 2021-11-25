// client for a 3d block game using opengl and sdl2.
#include <iso646.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>


#include <SDL2/SDL.h>
#include <OpenGL/gl3.h>


static const char* vertex_shader_code = "        	\n\
#version 330 core                              		\n\
                                                        \n\
attribute vec3 position;                                \n\
attribute float block;                                  \n\
                               				\n\
varying float block_type;                              	\n\
							\n\
uniform mat4 transform;					\n\
                                          		\n\
void main() {                                		\n\
	gl_Position = transform * vec4(position, 1.0);  \n\
	block_type = block;                             \n\
}                                                       \n";


static const char* fragment_shader_code = "        				\n\
#version 330 core                                            			\n\
                               							\n\
varying float block_type;			       				\n\
                               							\n\
void main() {                                					\n\
	if (block_type == 0.0) gl_FragColor = vec4(0.5, 0.0, 0.8, 1.0);		\n\
	else if (block_type == 1.0) gl_FragColor = vec4(0.8, 0.5, 0.0, 1.0);	\n\
	else if (block_type == 2.0) gl_FragColor = vec4(0.0, 1.0, 1.0, 1.0);	\n\
	else if (block_type == 3.0) gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);	\n\
	else gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);				\n\
}                               	 					\n";

struct vec3 {float x,y,z;};

typedef float* mat4;




// float* result = malloc(4 * 4 * sizeof(float));  calll using this sized mat4.

static inline void perspective(mat4 result, float fovy, float aspect, float zNear, float zFar) {
   
	const float t = tanf(fovy / 2.0f);
	result[4 * 0 + 0] = 1.0f / (aspect * t);
	result[4 * 1 + 1] = 1.0f / t;
	result[4 * 2 + 2] = -(zFar + zNear) / (zFar - zNear);
	result[4 * 2 + 3] = -1.0f;
	result[4 * 3 + 2] = -(2.0f * zFar * zNear) / (zFar - zNear);
}

static inline float inversesqrt(float number) {
	float x2 = number * 0.5f;
	float y = number;
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

static inline void look_at(mat4 result, struct vec3 eye, struct vec3 forward, struct vec3 up) {
	struct vec3 f = normalize(forward);
	struct vec3 s = normalize(cross(f, up));
	struct vec3 u = cross(s, f);

	result[4 * 0 + 0] = s.x;
	result[4 * 1 + 0] = s.y;
	result[4 * 2 + 0] = s.z;
	result[4 * 0 + 1] = u.x;
	result[4 * 1 + 1] = u.y;
	result[4 * 2 + 1] = u.z;
	result[4 * 0 + 2] =-f.x;
	result[4 * 1 + 2] =-f.y;
	result[4 * 2 + 2] =-f.z;
	result[4 * 3 + 0] =-dot(s, eye);
	result[4 * 3 + 1] =-dot(u, eye);
	result[4 * 3 + 2] = dot(f, eye);
	result[4 * 3 + 3] = 1;
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


#define window_width 800
#define window_height 600

static const float fovy = 1.22173f /*radians*/;
static const float aspect = (float) window_width / (float) window_height;
static const float znear = 0.001f;
static const float zfar = 1000.0f;



int main() {

	if (SDL_Init(SDL_INIT_VIDEO)) exit(printf("SDL_Init failed: %s\n", SDL_GetError()));

	SDL_Window *window = SDL_CreateWindow("block game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(window);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);	

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);

	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

	printf("%s\n", glGetString(GL_VERSION)); // debug

	SDL_GL_SetSwapInterval(0); // no vync.

	SDL_SetRelativeMouseMode(SDL_TRUE);

	// glEnable(GL_DEPTH_TEST);
	
	GLint success = 0;
	GLchar error[1024] = {0};
	GLuint program = glCreateProgram();
	
	GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	const GLchar* vs_sources[1] = {vertex_shader_code};
	GLint vs_lengths[1] = {(GLint)strlen(vertex_shader_code)};
	glShaderSource(vertex_shader, 1, vs_sources, vs_lengths);
	glCompileShader(vertex_shader);
	glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
	glGetShaderInfoLog(vertex_shader, sizeof(error), NULL, error);
	printf("vs error: %s\n", error);
	glAttachShader(program, vertex_shader);

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	const GLchar* fs_sources[1] = {fragment_shader_code};
	GLint fs_lengths[1] = {(GLint)strlen(fragment_shader_code)};
	glShaderSource(fragment_shader, 1, fs_sources, fs_lengths);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	glGetShaderInfoLog(fragment_shader, sizeof(error), NULL, error);
	printf("fs error: %s\n", error);
	glAttachShader(program, fragment_shader);
	
	enum {attribute_position, attribute_block};
	
	glBindAttribLocation(program, attribute_position, "position");
	glBindAttribLocation(program, attribute_block, "block");

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	glGetProgramInfoLog(program, sizeof(error), NULL, error);
	printf("link error: %s\n", error);
	glValidateProgram(program);
	glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
	glGetProgramInfoLog(program, sizeof(error), NULL, error);
	printf("validate error: %s\n", error);
	glUseProgram(program);

	GLint transform = glGetUniformLocation(program, "transform");

	

	// ---------------------------------------------------

	int vertex_count = 3;

	float positions[] = {
		
	};

	
	
	float block_types[] = {
		1.0,
		1.0,
		1.0
	};

	GLuint vertex_array;
	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);
	
	GLuint vertex_array_position_buffer;
	glGenBuffers(1, &vertex_array_position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_array_position_buffer);
	glEnableVertexAttribArray(attribute_position);
	glVertexAttribPointer(attribute_position, 3, GL_FLOAT, GL_FALSE, 0, 0);

	GLuint vertex_array_index_buffer;
	glGenBuffers(1, &vertex_array_index_buffer);
	
	GLuint vertex_array_block_buffer;
	glGenBuffers(1, &vertex_array_block_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_array_block_buffer);
	glEnableVertexAttribArray(attribute_block);
	glVertexAttribPointer(attribute_block, 1, GL_FLOAT, GL_FALSE, 0, 0);


	// parameters:
	
	const float camera_sensitivity = 0.01f;
	const float camera_accel = 0.01f;
	const float drag = 0.95f;

	// variables:

	struct vec3 position = {0, 0, -3};
	struct vec3 velocity = {0, 0, 0};
	float pitch = 0.0f, yaw = 0.0f;

	struct vec3 right = 	{1, 0, 0};
	struct vec3 up = 	{0, 1, 0};
	struct vec3 forward = 	{0, 0, 1};


	float* perspective_matrix = calloc(16, 4);
	perspective(perspective_matrix, fovy, aspect, znear, zfar);
	
	
	bool quit = false;

	int counter = 0;

	while (not quit) {
		uint32_t start = SDL_GetTicks();

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			const Uint8* key = SDL_GetKeyboardState(0);
			if (event.type == SDL_QUIT) quit = true;

			if (event.type == SDL_MOUSEMOTION) {

    				float dx = (float) event.motion.xrel;
    				float dy = (float) event.motion.yrel;

				yaw += camera_sensitivity * dx;
				pitch -= camera_sensitivity * dy;

				if (pitch > 1.57079632679f) pitch = 1.57079632679f - 0.0001f;
				else if (pitch < -1.57079632679f) pitch = -1.57079632679f + 0.0001f;

				forward.x = -sinf(yaw) * cosf(pitch);
				forward.y = -sinf(pitch);
				forward.z = -cosf(yaw) * cosf(pitch);

				right.x = -cosf(yaw);
				right.y = 0.0;
				right.z = sinf(yaw);

				up = cross(forward, right);

				forward = normalize(forward);
				right = normalize(right);
				up = normalize(up);
			}


				if (key[SDL_SCANCODE_SPACE]) { 
					velocity.x += camera_accel * up.x;
					velocity.y += camera_accel * up.y;
					velocity.z += camera_accel * up.z;
				}
				if (key[SDL_SCANCODE_LSHIFT]) { 
					velocity.x -= camera_accel * up.x;
					velocity.y -= camera_accel * up.y;
					velocity.z -= camera_accel * up.z;
				}

				if (key[SDL_SCANCODE_W]) { 
					velocity.x += camera_accel * forward.x;
					velocity.y += camera_accel * forward.y;
					velocity.z += camera_accel * forward.z;
				}
				if (key[SDL_SCANCODE_S]) { 
					velocity.x -= camera_accel * forward.x;
					velocity.y -= camera_accel * forward.y;
					velocity.z -= camera_accel * forward.z;
				}

				if (key[SDL_SCANCODE_D]) {
					velocity.x += camera_accel * right.x;
					velocity.y += camera_accel * right.y;
					velocity.z += camera_accel * right.z;
				}
				
				if (key[SDL_SCANCODE_A]) { 
					velocity.x -= camera_accel * right.x;
					velocity.y -= camera_accel * right.y;
					velocity.z -= camera_accel * right.z;
				}








				if (key[SDL_SCANCODE_LSHIFT]) velocity.y -= camera_accel;

				if (key[SDL_SCANCODE_W]) velocity += camera_accel;
				if (key[SDL_SCANCODE_S]) velocity -= camera_accel;

				if (key[SDL_SCANCODE_A]) velocity -= camera_accel;
				if (key[SDL_SCANCODE_D]) velocity += camera_accel;



    // const Uint8* key = SDL_GetKeyboardState(nullptr);
    // const float power = (delta * (key[SDL_SCANCODE_LCTRL] ? 0.0016f : camera_acceleration));

    // bool rotation_mode = !!key[SDL_SCANCODE_C];
    // bool tab = !!key[SDL_SCANCODE_TAB];
    // bool escape = !!key[SDL_SCANCODE_ESCAPE];
    
    // if (key[SDL_SCANCODE_SPACE]) camera.velocity += power * camera.upward;
    // if (key[SDL_SCANCODE_LSHIFT]) camera.velocity -= power * camera.upward;
    // if (key[SDL_SCANCODE_W]) camera.velocity += power * forward;
    // if (key[SDL_SCANCODE_S]) camera.velocity -= power * forward;
    // if (key[SDL_SCANCODE_A]) camera.velocity -= power * right
    // if (key[SDL_SCANCODE_D]) camera.velocity += power * right


			    if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
			        SDL_Log("Mouse Button 1 (left) is pressed.");
			    }

			    if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
			        SDL_Log("Mouse Button 2 (right) is pressed.");
			    }
















			if (event.type == SDL_KEYDOWN) {

				if (key[SDL_SCANCODE_Q]) quit = true;
				if (key[SDL_SCANCODE_ESCAPE]) quit = true;

				if (key[SDL_SCANCODE_2]) {
					block_types[0]++;
					block_types[1]++;
					block_types[2]++;
				}

				if (key[SDL_SCANCODE_1]) {
					block_types[0]--;
					block_types[1]--;
					block_types[2]--;
				}

				if (key[SDL_SCANCODE_TAB]) {
					
				}
				// if (key[SDL_SCANCODE_TAB]) SDL_SetWindowFullscreen ( window, ( fullscreen = !fullscreen) ? SDL_WINDOW_FULLSCREEN : 0);
			}
		}


		glClearColor(0.0f, 0.15f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		glBindBuffer(GL_ARRAY_BUFFER, vertex_array_position_buffer);
		glBufferData(GL_ARRAY_BUFFER, vertex_count * 3 * sizeof(float), positions, GL_STATIC_DRAW);

		glBindBuffer(GL_ARRAY_BUFFER, vertex_array_block_buffer);
		glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(float), block_types, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_array_index_buffer);
        	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indicies_count * sizeof(unsigned int), indicies, GL_STATIC_DRAW);






		float* view_matrix = calloc(16, 4);
		look_at(view_matrix, position, forward, up);
	 
		float* matrix = calloc(16, 4);
		float* copy = calloc(16, 4);

		matrix[4 * 0 + 0] = 1.0;
		matrix[4 * 1 + 1] = 1.0;
		matrix[4 * 2 + 2] = 1.0;
		matrix[4 * 3 + 3] = 1.0;

		memcpy(copy, matrix, 64);
		multiply_matrix(matrix, copy, view_matrix);

		memcpy(copy, matrix, 64);
		multiply_matrix(matrix, copy, perspective_matrix);

		glUniformMatrix4fv(transform, 1, GL_FALSE, matrix);

		glDrawArrays(GL_TRIANGLES, 0, vertex_count);
		SDL_GL_SwapWindow(window);


		

		velocity.x *= drag;
		velocity.y *= drag;
		velocity.z *= drag;



		int32_t time = (int32_t) SDL_GetTicks() - (int32_t) start;
		if (time < 0) continue;
		int32_t sleep = 16 - (int32_t) time;  // 16 for 60fps.
		if (sleep > 0) SDL_Delay((uint32_t) sleep);

		if (counter == 100) counter = 0;
		else counter++;

		if (counter == 0) {
			double fps = 1 / ((double) (SDL_GetTicks() - start) / 1000.0);
			printf("fps = %10.10lf\n", fps);
		}

		printf("DEBUG: \n");
		printf("yaw = %3.3lf, pitch = %3.3lf\n", (double)yaw, (double)pitch);
		printf("position = {%3.3lf, %3.3lf, %3.3lf}\n", (double)position.x,(double)position.y,(double)position.z);
		printf("forward = {%3.3lf, %3.3lf, %3.3lf}\n", (double)forward.x,(double)forward.y,(double)forward.z);
		printf("right = {%3.3lf, %3.3lf, %3.3lf}\n", (double)right.x,(double)right.y,(double)right.z);
		printf("up = {%3.3lf, %3.3lf, %3.3lf}\n", (double)up.x,(double)up.y,(double)up.z);
	}	

	// glDeleteTextures(1, &texture);

	glBindVertexArray(0); // unbind vao            ...?

	glDeleteVertexArrays(1, &vertex_array);

	// delete buffers?

	glDetachShader(program, vertex_shader);
	glDeleteShader(vertex_shader);
	glDetachShader(program, fragment_shader);
	glDeleteShader(fragment_shader);
	glDeleteProgram(program);

		
	
	SDL_GL_DeleteContext(context);
	SDL_DestroyWindow(window);
	SDL_Quit();
}




	// uint8_t* data = ...;  /// load image data from txture, and get the widt and height. should be in rgba format i think.

	// GLuint texture;
	// glGenTextures(1, &texture);
	// glActiveTexture(GL_TEXTURE0);
	// glBindTexture(GL_TEXTURE_2D, texture);
	
	// glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	// glTextureParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 

	// glTextureParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
	// glTextureParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data)




