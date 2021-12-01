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

/*
        TODO:
 -----------------------------

   	- research  simple voxel lighting without using normal vectors lol.

   	- research rendering optimizations.

	- code up the ray casting algorithm, for placing and breaking blocks. really easy.

	- code up the world generation, using 2d-wrap-around (modulo behavior) perlin noise.









	DONE:
---------------------------------

x	- research whether doing matrix mul is faster on gpu.

x	- why is the camera acceleratio so small?







*/

static int window_width = 1600;
static int window_height = 1000;
static float aspect = 1.6f;

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
uniform mat4 matrix;							\n\
                                          				\n\
void main() {                                				\n\
	gl_Position = matrix * vec4(position, 1.0);              	\n\
	block_type = block;                             		\n\
}                                                       		\n";

static const char* fragment_shader_code = "        				\n\
#version 120                                            			\n\
                               							\n\
varying float block_type;			       				\n\
                               							\n\
void main() {                                					\n\
	if (block_type == 0.0) gl_FragColor = vec4(0.1, 0.1, 0.1, 1.0);		\n\
	else if (block_type == 1.0) gl_FragColor = vec4(0.2, 0.2, 0.0, 1.0);	\n\
	else if (block_type == 2.0) gl_FragColor = vec4(0.0, 0.2, 0.2, 1.0);	\n\
	else if (block_type == 3.0) gl_FragColor = vec4(0.2, 0.0, 0.2, 1.0);	\n\
	else if (block_type == 4.0) gl_FragColor = vec4(0.5, 0.5, 0.0, 1.0);	\n\
	else if (block_type == 5.0) gl_FragColor = vec4(0.5, 0.0, 0.5, 1.0);	\n\
	else if (block_type == 6.0) gl_FragColor = vec4(0.0, 0.5, 0.5, 1.0);	\n\
	else if (block_type == 7.0) gl_FragColor = vec4(0.7, 0.7, 0.0, 1.0);	\n\
	else if (block_type == 8.0) gl_FragColor = vec4(0.7, 0.0, 0.7, 1.0);	\n\
	else if (block_type == 9.0) gl_FragColor = vec4(0.0, 0.7, 0.7, 1.0);	\n\
	else if (block_type == 10.0) gl_FragColor = vec4(1.0, 0.0, 1.0, 1.0);	\n\
	else if (block_type == 11.0) gl_FragColor = vec4(0.0, 1.0, 1.0, 1.0);	\n\
	else if (block_type == 12.0) gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0);	\n\
	else if (block_type == 13.0) gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);	\n\
	else if (block_type == 14.0) gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);	\n\
	else if (block_type == 15.0) gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);	\n\
	else if (block_type == 16.0) gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);	\n\
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

	glFrontFace(GL_CCW);

	glCullFace(GL_BACK);

	glEnable(GL_CULL_FACE);

	glPolygonMode(GL_FRONT, GL_FILL);


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
	if (not success) printf("vs error: %s\n", error);
	glAttachShader(program, vertex_shader);

	GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	const GLchar* fs_sources[1] = {fragment_shader_code};
	GLint fs_lengths[1] = {(GLint)strlen(fragment_shader_code)};
	glShaderSource(fragment_shader, 1, fs_sources, fs_lengths);
	glCompileShader(fragment_shader);
	glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
	glGetShaderInfoLog(fragment_shader, sizeof(error), NULL, error);
	if (not success) printf("fs error: %s\n", error);
	glAttachShader(program, fragment_shader);
	
	enum {attribute_position, attribute_block};
	
	glBindAttribLocation(program, attribute_position, "position");
	glBindAttribLocation(program, attribute_block, "block");

	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &success);
	glGetProgramInfoLog(program, sizeof(error), NULL, error);
	if (not success) printf("link error: %s\n", error);
	glValidateProgram(program);
	glGetProgramiv(program, GL_VALIDATE_STATUS, &success);
	glGetProgramInfoLog(program, sizeof(error), NULL, error);
	if (not success) printf("validate error: %s\n", error);
	glUseProgram(program);

	GLint matrix_uniform = glGetUniformLocation(program, "matrix");

	const int s = 10;
	const int space_count = s * s * s;
	int8_t* space = malloc(space_count);

	for (int i = 0; i < space_count; i++) {
		space[i] = 0;
	}

	srand((unsigned)time(NULL));

	// set a flat world:
	for (int x = 0; x < s; x++) {
		for (int z = 0; z < s; z++) {
			space[s * s * x + s * 0/*y=0*/ + z] = rand() % 5 + 1;
		}
	}

	// make a 2x2 box:
	space[s * s * 1 + s * 1 + 1] = 1;
	space[s * s * 1 + s * 1 + 2] = 2;
	space[s * s * 1 + s * 2 + 1] = 3;
	space[s * s * 1 + s * 2 + 2] = 4;
	space[s * s * 2 + s * 1 + 1] = 5;
	space[s * s * 2 + s * 1 + 2] = 6;
	space[s * s * 2 + s * 2 + 1] = 7;
	space[s * s * 2 + s * 2 + 2] = 8;


	// float cube_verticies[] = {
	// 	0,0,0, 0,    0,1,0, 0,    1,0,0, 0,

	// 	1,1,0, 0,    1,0,0, 0,    0,1,0, 0,


	// 	0,0,1, 0,    1,0,1, 0,    0,1,1, 0,

	// 	1,1,1, 0,    0,1,1, 0,    1,0,1, 0,    


	// 	1,1,1, 0,    1,0,0, 0,    1,1,0, 0,    

	// 	1,1,1, 0,    1,0,1, 0,    1,0,0, 0,   


	// 	0,1,1, 0,    0,1,0, 0,    0,0,0, 0,    

	// 	0,1,1, 0,    0,0,0, 0,    0,0,1, 0,


	// 	1,0,1, 0,    0,0,1, 0,    1,0,0, 0,

	// 	0,0,0, 0,    1,0,0, 0,    0,0,1, 0,    


	// 	1,1,1, 0,    1,1,0, 0,    0,1,1, 0,    

	// 	0,1,0, 0,    0,1,1, 0,    1,1,0, 0,    

	// }; // sizeof verticies / (sizeof(float) * 4);  =  36 verticies.


	int vertex_count = 0, list_count = 0;
	float* verticies = malloc(sizeof(float) * space_count * 144);

	for (int x = 0; x < s; x++) {
		for (int y = 0; y < s; y++) {
			for (int z = 0; z < s; z++) {
				int8_t block = space[s * s * x + s * y + z];
				if (not block) continue;

			if (not z or space[s * s * (x) + s * (y) + (z - 1)] == 0) {

				verticies[list_count++] = (float)x + 0;
				verticies[list_count++] = (float)y + 0;
				verticies[list_count++] = (float)z + 0;
				verticies[list_count++] = (float) block;
				verticies[list_count++] = (float)x + 0;
				verticies[list_count++] = (float)y + 1;
				verticies[list_count++] = (float)z + 0;
				verticies[list_count++] = (float) block;
				verticies[list_count++] = (float)x + 1;
				verticies[list_count++] = (float)y + 0;
				verticies[list_count++] = (float)z + 0;
				verticies[list_count++] = (float) block;
				verticies[list_count++] = (float)x + 1;
				verticies[list_count++] = (float)y + 1;
				verticies[list_count++] = (float)z + 0;
				verticies[list_count++] = (float) block;
				verticies[list_count++] = (float)x + 1;
				verticies[list_count++] = (float)y + 0;
				verticies[list_count++] = (float)z + 0;
				verticies[list_count++] = (float) block;
				verticies[list_count++] = (float)x + 0;
				verticies[list_count++] = (float)y + 1;
				verticies[list_count++] = (float)z + 0;
				verticies[list_count++] = (float) block;

				vertex_count += 6;
			}

			if (z >= s - 1 or space[s * s * (x) + s * (y) + (z + 1)] == 0) {

				verticies[list_count++] = (float)x + 0;
				verticies[list_count++] = (float)y + 0;
				verticies[list_count++] = (float)z + 1;
				verticies[list_count++] = (float) block;
				verticies[list_count++] = (float)x + 1;
				verticies[list_count++] = (float)y + 0;
				verticies[list_count++] = (float)z + 1;
				verticies[list_count++] = (float) block;
				verticies[list_count++] = (float)x + 0;
				verticies[list_count++] = (float)y + 1;
				verticies[list_count++] = (float)z + 1;
				verticies[list_count++] = (float) block;
				verticies[list_count++] = (float)x + 1;
				verticies[list_count++] = (float)y + 1;
				verticies[list_count++] = (float)z + 1;
				verticies[list_count++] = (float) block;
				verticies[list_count++] = (float)x + 0;
				verticies[list_count++] = (float)y + 1;
				verticies[list_count++] = (float)z + 1;
				verticies[list_count++] = (float) block;
				verticies[list_count++] = (float)x + 1;
				verticies[list_count++] = (float)y + 0;
				verticies[list_count++] = (float)z + 1;
				verticies[list_count++] = (float) block;

				vertex_count += 6;
			}

			
			if (x >= s - 1 or space[s * s * (x + 1) + s * (y) + (z)] == 0) {

				verticies[list_count++] = (float)x + 1;
				verticies[list_count++] = (float)y + 1;
				verticies[list_count++] = (float)z + 1;
				verticies[list_count++] = (float) block;
				verticies[list_count++] = (float)x + 1;
				verticies[list_count++] = (float)y + 0;
				verticies[list_count++] = (float)z + 0;
				verticies[list_count++] = (float) block;
				verticies[list_count++] = (float)x + 1;
				verticies[list_count++] = (float)y + 1;
				verticies[list_count++] = (float)z + 0;
				verticies[list_count++] = (float) block;
				verticies[list_count++] = (float)x + 1;
				verticies[list_count++] = (float)y + 1;
				verticies[list_count++] = (float)z + 1;
				verticies[list_count++] = (float) block;
				verticies[list_count++] = (float)x + 1;
				verticies[list_count++] = (float)y + 0;
				verticies[list_count++] = (float)z + 1;
				verticies[list_count++] = (float) block;
				verticies[list_count++] = (float)x + 1;
				verticies[list_count++] = (float)y + 0;
				verticies[list_count++] = (float)z + 0;
				verticies[list_count++] = (float) block;

				vertex_count += 6;
			}

			if (not x or space[s * s * (x - 1) + s * (y) + (z)] == 0) {

				verticies[list_count++] = (float)x + 0;
				verticies[list_count++] = (float)y + 1;
				verticies[list_count++] = (float)z + 1;
				verticies[list_count++] = (float) block;
				verticies[list_count++] = (float)x + 0;
				verticies[list_count++] = (float)y + 1;
				verticies[list_count++] = (float)z + 0;
				verticies[list_count++] = (float) block;
				verticies[list_count++] = (float)x + 0;
				verticies[list_count++] = (float)y + 0;
				verticies[list_count++] = (float)z + 0;
				verticies[list_count++] = (float) block;
				verticies[list_count++] = (float)x + 0;
				verticies[list_count++] = (float)y + 1;
				verticies[list_count++] = (float)z + 1;
				verticies[list_count++] = (float) block;
				verticies[list_count++] = (float)x + 0;
				verticies[list_count++] = (float)y + 0;
				verticies[list_count++] = (float)z + 0;
				verticies[list_count++] = (float) block;
				verticies[list_count++] = (float)x + 0;
				verticies[list_count++] = (float)y + 0;
				verticies[list_count++] = (float)z + 1;
				verticies[list_count++] = (float) block;

				vertex_count += 6;
			}

			if (not y or space[s * s * (x) + s * (y - 1) + (z)] == 0) {

				verticies[list_count++] = (float)x + 1;
				verticies[list_count++] = (float)y + 0;
				verticies[list_count++] = (float)z + 1;
				verticies[list_count++] = (float) block;
				verticies[list_count++] = (float)x + 0;
				verticies[list_count++] = (float)y + 0;
				verticies[list_count++] = (float)z + 1;
				verticies[list_count++] = (float) block;
				verticies[list_count++] = (float)x + 1;
				verticies[list_count++] = (float)y + 0;
				verticies[list_count++] = (float)z + 0;
				verticies[list_count++] = (float) block;
				verticies[list_count++] = (float)x + 0;
				verticies[list_count++] = (float)y + 0;
				verticies[list_count++] = (float)z + 0;
				verticies[list_count++] = (float) block;
				verticies[list_count++] = (float)x + 1;
				verticies[list_count++] = (float)y + 0;
				verticies[list_count++] = (float)z + 0;
				verticies[list_count++] = (float) block;
				verticies[list_count++] = (float)x + 0;
				verticies[list_count++] = (float)y + 0;
				verticies[list_count++] = (float)z + 1;
				verticies[list_count++] = (float) block;
	
				vertex_count += 6;
			}

			if (y >= s - 1 or space[s * s * (x) + s * (y + 1) + (z)] == 0) {

				verticies[list_count++] = (float)x + 1;
				verticies[list_count++] = (float)y + 1;
				verticies[list_count++] = (float)z + 1;
				verticies[list_count++] = (float) block;
				verticies[list_count++] = (float)x + 1;
				verticies[list_count++] = (float)y + 1;
				verticies[list_count++] = (float)z + 0;
				verticies[list_count++] = (float) block;
				verticies[list_count++] = (float)x + 0;
				verticies[list_count++] = (float)y + 1;
				verticies[list_count++] = (float)z + 1;
				verticies[list_count++] = (float) block;
				verticies[list_count++] = (float)x + 0;
				verticies[list_count++] = (float)y + 1;
				verticies[list_count++] = (float)z + 0;
				verticies[list_count++] = (float) block;
				verticies[list_count++] = (float)x + 0;
				verticies[list_count++] = (float)y + 1;
				verticies[list_count++] = (float)z + 1;
				verticies[list_count++] = (float) block;
				verticies[list_count++] = (float)x + 1;
				verticies[list_count++] = (float)y + 1;
				verticies[list_count++] = (float)z + 0;
				verticies[list_count++] = (float) block;

				vertex_count += 6;
			}

	
				// for (int i = 0; i < 36 * 4; i += 4) { // for each block vertex;
				// 	verticies[list_count++] = (float)x + cube_verticies[i + 0];
				// 	verticies[list_count++] = (float)y + cube_verticies[i + 1];
				// 	verticies[list_count++] = (float)z + cube_verticies[i + 2];
				// 	verticies[list_count++] = (float) block;
				// 	vertex_count++;
				// }
			}
		}
	}

	/*

		todo: rendereing:

			- we have to not draw the faces which are next to other transparent blocks, i think...

			- we have to do the greedy algorithm, which treats multiple faces as one.. 

			- we have to cull back faces, and only draw front faces.

	*/

	

	



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
	struct vec3 position = {5, 3, 5};
	struct vec3 velocity = {0, 0, 0};

	struct vec3 forward = 	{0, 0, -1};
	struct vec3 straight = 	{0, 0, 1};
	struct vec3 up = 	{0, 1, 0};
	struct vec3 right = 	{-1, 0, 0};

	straight = cross(right, up);

	float* view_matrix = calloc(16, 4);
	float* perspective_matrix = calloc(16, 4);
	float* matrix = calloc(16, 4);
	float* copy = calloc(16, 4);

	perspective(perspective_matrix, fovy, aspect, znear, zfar);

	while (not quit) {
		uint32_t start = SDL_GetTicks();

		SDL_Event event;
		while (SDL_PollEvent(&event)) {

			const Uint8* key = SDL_GetKeyboardState(0);

			if (event.type == SDL_QUIT) quit = true;

			if (event.type == SDL_WINDOWEVENT_RESIZED) {
				printf("window was resized!!\n");
	int w=0,h =0;
					SDL_GetWindowSize(window, &w, &h);

					printf("width = %d, height = %d", w,h);
					window_width = w;
					window_height = h;
					aspect = (float) window_width / (float) window_height;
					perspective(perspective_matrix, fovy, aspect, znear, zfar);
			}

			if (event.type == SDL_MOUSEMOTION) {

    				float dx = (float) event.motion.xrel;
    				float dy = (float) event.motion.yrel;

				yaw -= camera_sensitivity * dx;
				pitch += camera_sensitivity * dy;
	
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

			if (event.type == SDL_KEYDOWN) {
				if (tab and key[SDL_SCANCODE_Q]) quit = true; 
				if (key[SDL_SCANCODE_ESCAPE]) quit = true;
				if (key[SDL_SCANCODE_0]) debug = !debug;

				if (key[SDL_SCANCODE_9]) {
					SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
					int w=0,h =0;
					SDL_GetWindowSize(window, &w, &h);

					printf("width = %d, height = %d", w,h);
					window_width = w;
					window_height = h;
					aspect = (float) window_width / (float) window_height;

					perspective(perspective_matrix, fovy, aspect, znear, zfar);
					// todo: reset the perspective, based on the aspect ratio, baased on the size of the window.
				}
			}

			if (event.type == SDL_MOUSEBUTTONDOWN) {
				if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
	        			SDL_Log("Mouse Button 1 (left) is pressed.");
	    			}

	    			if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
	        			SDL_Log("Mouse Button 2 (right) is pressed.");
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

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// generate new verticies (ie, a new mesh), then say this:
		glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)((size_t)vertex_count * 4 * sizeof(float)), verticies, GL_STATIC_DRAW);


		look_at(view_matrix, position, forward, up);
	
		multiply_matrix(matrix, view_matrix, matrix);
		multiply_matrix(matrix, perspective_matrix, matrix);

	
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

		glDrawArrays(GL_TRIANGLES, 0, vertex_count);
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




/*




















	float x = floorf(origin.x);
	float y = floorf(origin.y);
	float z = floorf(origin.z);

	float dx = direction.x;
	float dy = direction.y;
	float dz = direction.z;

	float step_x = signum(dx);
	float step_y = signum(dy);
	float step_z = signum(dz);

	float t_max_x = intbound(origin.x, dx);
	float t_max_y = intbound(origin.y, dy);
	float t_max_z = intbound(origin.z, dz);
  
	float t_delta_x = step_x / dx;
	float t_delta_y = step_y / dy;
	float t_delta_z = step_z / dz;
  
	struct vec3 face = {0, 0, 0};

	if (not dx and not dy and not dz) printf("error: ray cast with zero direction.\n");

	const float radius = 12.0;

  while ((step_x > 0 ? x < wx : x >= 0) and
         (step_y > 0 ? y < wy : y >= 0) and
         (step_z > 0 ? z < wz : z >= 0)) {

    
    if (not (x < 0 or y < 0 or z < 0 or x >= wx or y >= wy or z >= wz))
      if (callback(x, y, z, blocks[x * wy * wz + y * wz + z], face)) break;


	if (x < y and x < z) {}
	else if (y < x and y < z) {}
	else if (z < x and z < y) {}
	else 


    if (t_max_x < t_max_y) {
      if (t_max_x < tMaxZ) {
        if (t_max_x > radius) break;
        x += stepX;
        t_max_x += tDeltaX;
        face = {-stepX,0,0};
      } else {
        if (tMaxZ > radius) break;
        z += stepZ;
        tMaxZ += tDeltaZ;
        face = {0,0,-stepZ};
      }
    } else {
      if (t_max_y < tMaxZ) {
        if (t_max_y > radius) break;
        y += stepY;
        t_max_y += tDeltaY;
        face = {0,-stepY,0};
      } else {
        if (tMaxZ > radius) break;
        z += stepZ;
        tMaxZ += tDeltaZ;
        face = {0,0,-stepZ};
      }
    }
  }




*/


