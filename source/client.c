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
/*ddxddxdde
        TODO:
 -----------------------------

   	- research  simple voxel lighting without using normal vectors lol.

   	- research rendering optimizations.

	- research whether doing matrix mul is faster on gpu.

	- why is the camera acceleratio so small?

	- 






*/

static const int window_width = 1600;
static const int window_height = 1000;
static const float aspect = (float) window_width / (float) window_height;

static const float fovy = 1.22173f /*radians*/;
static const float znear = 0.01f;
static const float zfar = 100.0f;

static const float camera_sensitivity = 0.005f;
static const float camera_accel = 0.00003f;
static const float drag = 0.95f;

static const int32_t ms_delay_per_frame = 16;

static const char* vertex_shader_code = "        			\n\
#version 120                              				\n\
                                                        		\n\
attribute vec3 position;                                		\n\
attribute float block;                                  		\n\
                               						\n\
varying float block_type;                              			\n\
									\n\
uniform mat4 view;							\n\
uniform mat4 perspective;						\n\
                                          				\n\
void main() {                                				\n\
	gl_Position = perspective * view * vec4(position, 1.0);  	\n\
	block_type = block;                             		\n\
}                                                       		\n";

static const char* fragment_shader_code = "        				\n\
#version 120                                            			\n\
                               							\n\
varying float block_type;			       				\n\
                               							\n\
void main() {                                					\n\
	if (block_type == 0.0) gl_FragColor = vec4(0.5, 0.0, 0.8, 1.0);		\n\
	else if (block_type == 1.0) gl_FragColor = vec4(0.8, 0.5, 0.0, 1.0);	\n\
	else if (block_type == 2.0) gl_FragColor = vec4(0.0, 1.0, 1.0, 1.0);	\n\
	else if (block_type == 3.0) gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);	\n\
	else if (block_type == 4.0) gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);	\n\
	else if (block_type == 5.0) gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);	\n\
	else gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);				\n\
}                               	 					\n";

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
	 //TODO: is this neccessary?
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

int main() {

	if (SDL_Init(SDL_INIT_VIDEO)) exit(printf("SDL_Init failed: %s\n", SDL_GetError()));

	SDL_Window *window = SDL_CreateWindow("block game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
				window_width, window_height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL);
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
	glEnable(GL_DEPTH_TEST);
	// glEnable(GL_CULL_BACKFACES);

	printf("%s\n", glGetString(GL_VERSION)); // debug

	SDL_GL_SetSwapInterval(0); // no vync.
	SDL_SetRelativeMouseMode(SDL_TRUE);
	
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

	GLint view_uniform = glGetUniformLocation(program, "view");
	GLint perspective_uniform = glGetUniformLocation(program, "perspective");


	const int s = 100;
	const int space_count = s * s * s;
	int8_t* space = malloc(space_count);

	for (int i = 0; i < space_count; i++) {
		space[i] = 0;
	}

	srand((unsigned)time(NULL));

	// set a flat world:
	for (int x = 0; x < s; x++) {
		// for (int y = 0; y < s; y++) {
			for (int z = 0; z < s; z++) {
				space[s * s * x + s * 0/*y=0*/ + z] = rand() % 5 + 1;
			}
			// break;
		// }
	}

	space[s * s * 3 + s * 1 + 3] = 1;
	space[s * s * 3 + s * 2 + 3] = 1;
	space[s * s * 3 + s * 3 + 3] = 2;

	

	
	

// #define color0  1.0
// #define color1  2.0
// #define color2  3.0
// #define color3  4.0
// #define color4  5.0
// #define color5  6.0



	float cube_verticies[] = {
		0,0,0, 0,    1,0,0, 0,    0,1,0, 0,

		1,1,0, 0,    1,0,0, 0,    0,1,0, 0,


		0,0,1, 0,    1,0,1, 0,    0,1,1, 0,

		1,1,1, 0,    1,0,1, 0,    0,1,1, 0,


		1,1,1, 0,    1,0,0, 0,    1,1,0, 0,    

		1,1,1, 0,    1,0,1, 0,    1,0,0, 0,   


		0,1,1, 0,    0,0,0, 0,    0,1,0, 0,

		0,1,1, 0,    0,0,1, 0,    0,0,0, 0,   


		1,0,1, 0,    0,0,1, 0,    1,0,0, 0,

		0,0,0, 0,    0,0,1, 0,    1,0,0, 0,


		1,1,1, 0,    0,1,1, 0,    1,1,0, 0,

		0,1,0, 0,    0,1,1, 0,    1,1,0, 0,

	}; // sizeof verticies / (sizeof(float) * 4);  =  36 verticies.

	int vertex_count = 0;
	float* verticies = malloc(sizeof(float) * space_count * 144);

	for (int x = 0; x < s; x++) {
		for (int y = 0; y < s; y++) {
			for (int z = 0; z < s; z++) {
				int8_t block = space[s * s * x + s * y + z];
				if (not block) continue;

				for (int i = 0; i < 36 * 4; i += 4) { // for each block vertex;
					verticies[vertex_count++] = (float)x + cube_verticies[i + 0];
					verticies[vertex_count++] = (float)y + cube_verticies[i + 1];
					verticies[vertex_count++] = (float)z + cube_verticies[i + 2];
					verticies[vertex_count++] = (float) block;
				}
			}
		}
	}



	

	



	GLuint vertex_array;
	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);

	GLuint vertex_array_buffer;
	glGenBuffers(1, &vertex_array_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_array_buffer);

	glEnableVertexAttribArray(attribute_position);
	glVertexAttribPointer(attribute_position, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

	glEnableVertexAttribArray(attribute_block);
	glVertexAttribPointer(attribute_block, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));

	// variables:
	bool debug = false;
	bool quit = false;
	bool tab = false;
	int counter = 0;
	float delta = 0.0;

	float pitch = 0.0f, yaw = 0.0f;
	struct vec3 position = {0, 0, -3};
	struct vec3 velocity = {0, 0, 0};

	struct vec3 forward = 	{0, 0, -1};
	struct vec3 straight = 	{0, 0, 1};
	struct vec3 up = 	{0, 1, 0};
	struct vec3 right = 	{-1, 0, 0};

	straight = cross(right, up);

	float* view_matrix = calloc(16, 4);
	float* perspective_matrix = calloc(16, 4);

	perspective(perspective_matrix, fovy, aspect, znear, zfar);
	glUniformMatrix4fv(perspective_uniform, 1, GL_FALSE, perspective_matrix);

	while (not quit) {
		uint32_t start = SDL_GetTicks();

		SDL_Event event;
		while (SDL_PollEvent(&event)) {

			const Uint8* key = SDL_GetKeyboardState(0);

			if (event.type == SDL_QUIT) quit = true;

			if (event.type == SDL_MOUSEMOTION) {

    				float dx = (float) event.motion.xrel;
    				float dy = (float) event.motion.yrel;

				yaw -= camera_sensitivity * dx;
				pitch += camera_sensitivity * dy;

				if (pitch > 1.57079632679f) pitch = 1.57079632679f - 0.0001f;
				else if (pitch < -1.57079632679f) pitch = -1.57079632679f + 0.0001f;

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

			if (event.type == SDL_KEYDOWN) {
				if (key[SDL_SCANCODE_Q]) quit = true;        // tempoerary. 
				if (key[SDL_SCANCODE_ESCAPE]) quit = true;
				if (key[SDL_SCANCODE_1]) debug = !debug;
				// if (key[SDL_SCANCODE_TAB]) SDL_SetWindowFullscreen ( window, ( fullscreen = !fullscreen) ? SDL_WINDOW_FULLSCREEN : 0);
			}
		}

			    // if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
			    //     SDL_Log("Mouse Button 1 (left) is pressed.");
			    // }

			    // if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
			    //     SDL_Log("Mouse Button 2 (right) is pressed.");
			    // }

		const Uint8* key = SDL_GetKeyboardState(0);
		
		tab = !!key[SDL_SCANCODE_TAB];

		if (key[SDL_SCANCODE_SPACE]) {
			velocity.x += delta * camera_accel * up.x;
			velocity.y += delta * camera_accel * up.y;
			velocity.z += delta * camera_accel * up.z;
		}

		if (key[SDL_SCANCODE_LSHIFT]) { 
			velocity.x -= delta * camera_accel * up.x;
			velocity.y -= delta * camera_accel * up.y;
			velocity.z -= delta * camera_accel * up.z;
		}

		if (key[SDL_SCANCODE_W]) { 
			velocity.x += delta * camera_accel * straight.x;
			velocity.y += delta * camera_accel * straight.y;
			velocity.z += delta * camera_accel * straight.z;
		}
		if (key[SDL_SCANCODE_S]) { 
			velocity.x -= delta * camera_accel * straight.x;
			velocity.y -= delta * camera_accel * straight.y;
			velocity.z -= delta * camera_accel * straight.z;
		}

		if (key[SDL_SCANCODE_A]) {
			velocity.x += delta * camera_accel * right.x;
			velocity.y += delta * camera_accel * right.y;
			velocity.z += delta * camera_accel * right.z;
		}
		
		if (key[SDL_SCANCODE_D]) { 
			velocity.x -= delta * camera_accel * right.x;
			velocity.y -= delta * camera_accel * right.y;
			velocity.z -= delta * camera_accel * right.z;
		}

		glClearColor(0.0f, 0.15f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// generate new verticies (ie, a new mesh), then say this:
		glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)((size_t)vertex_count * 4 * sizeof(float)), verticies, GL_STATIC_DRAW);

		look_at(view_matrix, position, forward, up);
		glUniformMatrix4fv(view_uniform, 1, GL_FALSE, view_matrix);

		glDrawArrays(GL_TRIANGLES, 0, vertex_count);
		SDL_GL_SwapWindow(window);

		// simulate basic movement physics:
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
			printf("fps = %10.10lf\n", fps);
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
