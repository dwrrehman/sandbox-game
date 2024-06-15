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


static const float fovy = 1.22173f /*radians*/;
static const float znear = 0.01f;
static const float zfar = 1000.0f;
static const float camera_sensitivity = 0.004f;

enum blocks {
	air_block,
	grass_block,
	dirt_block,
	stone_block,
	granite_block,
	wood_block,
	leaves_block,
	water_block,
	moss_block,
	iron_ore_block,
	off_cell_block,
	on_cell_block,
	off_path_block,
	on_path_block,
	glass_block,
	block_count,
};

static const float speed = 0.15f;

struct vec3 { float x, y, z; };
struct nat3 { nat x, y, z; };
typedef float* mat4;
typedef uint64_t nat;

static float view_matrix[16] = {0};
static float perspective_matrix[16] = {0};
static float matrix[16] = {0};
static float copy[16] = {0};
static double lastx = 0, lasty = 0;
static bool first = true;

static int window_width = 1280;
static int window_height = 720;
static float aspect = 1280.0f / 720.0f;

static float delta = 0.0; 
static float pitch = 0.0f, yaw = 0.0f;

static struct vec3 position = {0, 0, 0};

static struct vec3 forward = 	{0, 0, -1};
static struct vec3 straight = 	{0, 0, 1};
static struct vec3 up = 	{0, 1, 0};
static struct vec3 right = 	{-1, 0, 0};

// temporary:

static uint32_t err = 0;
#define cc	do { err = glGetError(); if (err != GL_NO_ERROR) { printf("%s: line%u: err%u\n", __FILE__, __LINE__, err);  exit(1); } } while(0);




static const char* vertex_shader_code = "        			\n\
#version 330 core							\n\
									\n\
layout(location = 0) in vec3 pos;					\n\
layout(location = 1) in vec2 tex;					\n\
out vec2 UV;                      					\n\
uniform mat4 matrix;                      				\n\
                                          				\n\
void main() {                                				\n\
	gl_Position = matrix * vec4(pos, 1.0);           		\n\
	UV = tex; 							\n\
}                                                       		\n";

static const char* fragment_shader_code = "				\n\
#version 330 core							\n\
									\n\
in vec2 UV;								\n\
out vec3 color;								\n\
uniform sampler2D atlas_texture;					\n\
									\n\
void main() {								\n\
	 color = texture(atlas_texture, UV).rgb;			\n\
}									\n";

static void printGLInfo(void) {
	printf("OpenGL: %s %s %s\n",
			glGetString(GL_VENDOR),
			glGetString(GL_RENDERER),
			glGetString(GL_VERSION));
	printf("OpenGL Shading language: %s\n",
			glGetString(GL_SHADING_LANGUAGE_VERSION));
}

static void listGLExtensions(void) {
	GLint num = 0;
	glGetIntegerv(GL_NUM_EXTENSIONS, &num);
	printf("GL extensions supported: %d\n", num);
	if (num < 1) return;

	for (int i = 0; i < num; i++) {
		const GLubyte* ext = glGetStringi(GL_EXTENSIONS, (GLuint) i);
		if (ext) printf("  %s\n",ext);
	}
}

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

static inline void look_at(mat4 result, struct vec3 eye, struct vec3 f, struct vec3 up1) {
	struct vec3 s = normalize(cross(f, up1));
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

static void framebuffer_size_callback(__attribute__((unused)) GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	printf("framebuffer_size_callback: window was resized!!\n");
	printf("width = %d, height = %d", width, height);
	window_width = width;
	window_height = height;
	aspect = (float) window_width / (float) window_height;
	perspective(perspective_matrix, fovy, aspect, znear, zfar);
}

static void mouse_callback(__attribute__((unused)) GLFWwindow* window, double x, double y) {
	if (first) { lastx = x; lasty = y; first = false; }
	yaw += camera_sensitivity * -((float)x - (float)lastx);
	pitch += camera_sensitivity * -((float)lasty - (float)y);
	move_camera();
	lastx = x; lasty = y;
}

static void mouse_button_callback(__attribute__((unused)) GLFWwindow* window, int button, int action, __attribute__((unused)) int mods) {
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
		puts("right mouse button clicked!!");

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		puts("left mouse button clicked!!");
}


static void key_callback(__attribute__((unused)) GLFWwindow* window, 
			int key, __attribute__((unused)) int scancode, 
			int action, __attribute__((unused)) int mods
) {
	if (key == GLFW_KEY_E && action == GLFW_PRESS) {
		puts("something happened!");
	}
}





int main(void) {
	srand((unsigned)time(NULL));
	if (not glfwInit()) exit(1);

#ifdef __APPLE__
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	GLFWwindow* window = glfwCreateWindow( window_width, window_height, "block game", NULL, NULL );
	if (not window) { puts("error: window creation"); glfwTerminate(); exit(1); }
	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	if (not monitor) { puts("error: could not get monitor"); glfwTerminate(); exit(1); }

	glfwMakeContextCurrent(window);
	glfwSwapInterval(0);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 
	if (glfwRawMouseMotionSupported())
    		glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);

	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetKeyCallback(window, key_callback);

	if (not gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) exit(1);

	GLuint vertex_array;
	glGenVertexArrays(1, &vertex_array);cc;
	glBindVertexArray(vertex_array);cc;

	glEnable(GL_DEPTH_TEST);cc;
	glFrontFace(GL_CCW);cc;
	glCullFace(GL_BACK);cc;
	glEnable(GL_CULL_FACE);cc;
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);cc;

	printf("glGetString(GL_VERSION) = %s\n", glGetString(GL_VERSION)); // debug
	printGLInfo(); puts("");
	listGLExtensions(); puts("");

	straight = cross(right, up);
	perspective(perspective_matrix, fovy, aspect, znear, zfar);

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
	glAttachShader(program, vertex_shader);cc;

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
	glAttachShader(program, fragment_shader);cc;
	
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
	glUseProgram(program);cc;
	GLint matrix_uniform = glGetUniformLocation(program, "matrix");cc;
	GLint texture_uniform = glGetUniformLocation(program, "atlas_texture");cc;
	glUniform1i(texture_uniform, 0);cc;

	glBindAttribLocation(program, 0, "pos");cc;
	glBindAttribLocation(program, 1, "tex");cc;
	glBindFragDataLocation(program, 0, "color");cc;

	uint32_t texture_id;
	glGenTextures(1, &texture_id);cc;
	glActiveTexture(GL_TEXTURE0);cc;
	glBindTexture(GL_TEXTURE_2D, texture_id);cc;
	glGenerateMipmap(GL_TEXTURE_2D);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);cc;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);cc;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);cc;
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);cc;

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
	);cc;

	const int s = 200;
	const int space_count = s * s * s;
	int8_t* space = calloc(space_count, 1);

	for (int x = 0; x < s; x++) {
		for (int z = 0; z < s; z++) {

			const float f = perlin2d(x, z, 0.01f, 20);
			const int H = (int) (f * 50);
			//printf("H = %u, n = %f\n", H, f);

			const int divide = H / 2;
			for (int y = 0; y < H; y++) {
				if (y >= divide) space[s * s * x + s * y + z] = dirt_block;
				if (y < divide) space[s * s * x + s * y + z] = stone_block + (rand() % 2) * (rand() % 2);

			}
			space[s * s * x + s * H + z] = grass_block;
		}
	}

	space[s * s * 50 + s * 50 + 4] = air_block;
	space[s * s * 50 + s * 50 + 6] = grass_block;
	space[s * s * 50 + s * 50 + 8] = dirt_block;
	space[s * s * 50 + s * 50 + 10] = stone_block;
	space[s * s * 50 + s * 50 + 12] = granite_block;
	space[s * s * 50 + s * 50 + 14] = wood_block;
	space[s * s * 50 + s * 50 + 16] = leaves_block;
	space[s * s * 50 + s * 50 + 18] = water_block;
	space[s * s * 50 + s * 50 + 20] = moss_block;
	space[s * s * 50 + s * 50 + 22] = iron_ore_block;
	space[s * s * 50 + s * 50 + 24] = off_cell_block;
	space[s * s * 50 + s * 50 + 26] = on_cell_block;
	space[s * s * 50 + s * 50 + 28] = off_path_block;
	space[s * s * 50 + s * 50 + 30] = on_path_block;
	space[s * s * 50 + s * 50 + 32] = glass_block;


	position.x = 20;
	position.y = 25;
	position.z = 20;
	space[s * s * 20 + s * 25 + 20] = on_cell_block;


#define push_vertex(xo, yo, zo, u, v) 			\
	verticies[raw_count++] = (float)x + xo;		\
	verticies[raw_count++] = (float)y + yo;		\
	verticies[raw_count++] = (float)z + zo;		\
	verticies[raw_count++] = (float) u;		\
	verticies[raw_count++] = (float) v;		\
	vertex_count++;

	GLsizei vertex_count = 0, raw_count = 0;//, index_count = 0;

	float* verticies = malloc(sizeof(float) * space_count * 144);

	unsigned short front_x[256] 	= {1,0,3,4,5,6,7,2,3,4,5,6,7,1};
	unsigned short front_y[256] 	= {0,0,0,0,0,0,0,1,1,1,1,1,1,1};

	unsigned short back_x[256] 	= {1,0,3,4,5,6,7,2,3,4,5,6,7,1};
	unsigned short back_y[256] 	= {0,0,0,0,0,0,0,1,1,1,1,1,1,1};

	unsigned short up_x[256] 	= {2,0,3,4,0,6,7,2,3,4,5,6,7,1};
	unsigned short up_y[256] 	= {0,0,0,0,1,0,0,1,1,1,1,1,1,1};

	unsigned short down_x[256] 	= {0,0,3,4,0,6,7,2,3,4,5,6,7,1};
	unsigned short down_y[256] 	= {0,0,0,0,1,0,0,1,1,1,1,1,1,1};

	unsigned short left_x[256] 	= {1,0,3,4,5,6,7,2,3,4,5,6,7,1};
	unsigned short left_y[256] 	= {0,0,0,0,0,0,0,1,1,1,1,1,1,1};

	unsigned short right_x[256] 	= {1,0,3,4,5,6,7,2,3,4,5,6,7,1};
	unsigned short right_y[256] 	= {0,0,0,0,0,0,0,1,1,1,1,1,1,1};

	for (int x = 0; x < s; x++) {
		for (int y = 0; y < s; y++) {
			for (int z = 0; z < s; z++) {

				int8_t block = space[s * s * x + s * y + z];
				if (not block) continue;

				block--;
				
				const float e = 1.0 / 8.0;
				const float _ = 0;

				if (not z or not space[s * s * (x) + s * (y) + (z - 1)]) { 
					// LEFT
					const float ut = e * left_x[block];
					const float vt = e * left_y[block];
					push_vertex(0,0,0, ut+e,vt+e);
					push_vertex(0,1,0, ut+e,vt+_);
					push_vertex(1,0,0, ut+_,vt+e);
					push_vertex(1,1,0, ut+_,vt+_);
					push_vertex(1,0,0, ut+_,vt+e);
					push_vertex(0,1,0, ut+e,vt+_);
				}

				if (z >= s - 1 or not space[s * s * (x) + s * (y) + (z + 1)]) {
					//RIGHT
					const float ut = e * right_x[block];
					const float vt = e * right_y[block];
					push_vertex(0,0,1, ut+e,vt+e);
					push_vertex(1,0,1, ut+_,vt+e);
					push_vertex(0,1,1, ut+e,vt+_);
					push_vertex(1,1,1, ut+_,vt+_);
					push_vertex(0,1,1, ut+e,vt+_);
					push_vertex(1,0,1, ut+_,vt+e);
				} 

				if (x >= s - 1 or not space[s * s * (x + 1) + s * (y) + (z)]) {
					// BACK
					const float ut = e * back_x[block];
					const float vt = e * back_y[block];
					push_vertex(1,1,1, ut+e,vt+_); // back top right
					push_vertex(1,0,0, ut+_,vt+e); // back bottom left
					push_vertex(1,1,0, ut+_,vt+_); // back bottom right
					push_vertex(1,1,1, ut+e,vt+_); // back top right
					push_vertex(1,0,1, ut+e,vt+e); // back top left
					push_vertex(1,0,0, ut+_,vt+e); // back bottom left
				}

				if (not x or not space[s * s * (x - 1) + s * (y) + (z)]) {
					// FRONT
					const float ut = e * front_x[block];
					const float vt = e * front_y[block];
					push_vertex(0,1,1, ut+_,vt+_); //top right
					push_vertex(0,1,0, ut+e,vt+_); //bottom right
					push_vertex(0,0,0, ut+e,vt+e); //bottom left
					push_vertex(0,1,1, ut+_,vt+_); //top right
					push_vertex(0,0,0, ut+e,vt+e); //bottom left
					push_vertex(0,0,1, ut+_,vt+e); //top left
				}

				if (not y or not space[s * s * (x) + s * (y - 1) + (z)]) {
					// DOWN
					const float ut = e * down_x[block];
					const float vt = e * down_y[block];
					push_vertex(1,0,1, ut+_,vt+_);
					push_vertex(0,0,1, ut+_,vt+e);
					push_vertex(1,0,0, ut+e,vt+_);
					push_vertex(0,0,0, ut+e,vt+e);
					push_vertex(1,0,0, ut+e,vt+_);
					push_vertex(0,0,1, ut+_,vt+e);
				}

				if (y >= s - 1 or not space[s * s * (x) + s * (y + 1) + (z)]) {
					// UP
					const float ut = e * up_x[block];
					const float vt = e * up_y[block];
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

	GLuint vertex_array_buffer;
	glGenBuffers(1, &vertex_array_buffer);cc;
	glBindBuffer(GL_ARRAY_BUFFER, vertex_array_buffer);cc;
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);cc;
	glEnableVertexAttribArray(0);cc;
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));cc;
	glEnableVertexAttribArray(1);cc;

	while (not glfwWindowShouldClose(window)) {
		glfwPollEvents();

		const clock_t begin_time = clock();

		const nat position_x = (nat) position.x;
		const nat position_y = (nat) position.y;
		const nat position_z = (nat) position.z;

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);

		if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); 

		if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); 

		if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
			const GLFWvidmode* videomode = glfwGetVideoMode(monitor);
			glfwSetWindowMonitor(window, monitor, 0, 0, videomode->width, videomode->height, videomode->refreshRate);
		}

		if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
			glfwSetWindowMonitor(window, NULL, 50, 50, 1280, 720, 0);
		}

		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
			position.x += speed * up.x;
			position.y += speed * up.y;
			position.z += speed * up.z;
		}

		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
			position.x -= speed * up.x;
			position.y -= speed * up.y;
			position.z -= speed * up.z;
		}

		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
			
			const nat px = (nat) position.x + speed * right.x;
			const nat py = (nat) position.y + speed * right.y;
			const nat pz = (nat) position.z + speed * right.z;
			if (space[s * s * px + s * py + pz] == air_block) {
				space[s * s * position_x + s * position_y + position_z] = air_block;
				space[s * s * px + s * py + pz] = on_cell_block;
				position.x += speed * right.x;
				position.y += speed * right.y;
				position.z += speed * right.z;
			}
			
			
		}

		if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
			position.x -= speed * right.x;
			position.y -= speed * right.y;
			position.z -= speed * right.z;
		}

		if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
			position.x += speed * straight.x;
			position.y += speed * straight.y;
			position.z += speed * straight.z;
		}

		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
			position.x -= speed * straight.x;
			position.y -= speed * straight.y;
			position.z -= speed * straight.z;
		}



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

		glClearColor(0.5f, 0.6f, 1.0f, 1.0f);cc;
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);cc;

		glUniformMatrix4fv(matrix_uniform, 1, GL_FALSE, matrix);cc;

		glBufferData(
			GL_ARRAY_BUFFER, 
			(GLsizeiptr)((size_t) vertex_count * 5 * sizeof(float)), 
			verticies, 
			GL_STATIC_DRAW
		);cc;

		const bool render_lines = glfwGetKey(window, GLFW_KEY_BACKSLASH) == GLFW_PRESS;
		glDrawArrays(render_lines ? GL_LINES : GL_TRIANGLES, 0, vertex_count); cc;
		glfwSwapBuffers(window);
	

		if (position.x < 0) position.x = 0;
		if (position.y < 0) position.y = 0;
		if (position.z < 0) position.z = 0;
		if (position.x >= s) position.x = (float)s - 0.01f;
		if (position.y >= s) position.y = (float)s - 0.01f;
		if (position.z >= s) position.z = (float)s - 0.01f;

		const clock_t end_time = clock();
		delta = (float) (end_time - begin_time);
		usleep(16666); // todo: convert elapsed to seconds, and use it. 


		printf("position = {%3.3lf, %3.3lf, %3.3lf}\n", (double)position.x,(double)position.y,(double)position.z);

	}
	glfwTerminate();
}











		// printf("velocity = {%3.3lf, %3.3lf, %3.3lf}\n", (double)velocity.x,(double)velocity.y,(double)velocity.z);

		/*printf("\033[%um[x0z0]\033[0m \033[%um[x1z0]\033[0m \033[%um[x0z1]\033[0m \033[%um[x1z1]\033[0m\n", 
			space[s*s*(int)(position.x - 0.4f) + s * (int)(position.y - 1.9f) + (int)(position.z - 0.4f)] ? 32 : 1,
			space[s*s*(int)(position.x + 0.4f) + s * (int)(position.y - 1.9f) + (int)(position.z - 0.4f)] ? 32 : 1,
			space[s*s*(int)(position.x - 0.4f) + s * (int)(position.y - 1.9f) + (int)(position.z + 0.4f)] ? 32 : 1,
			space[s*s*(int)(position.x + 0.4f) + s * (int)(position.y - 1.9f) + (int)(position.z + 0.4f)] ? 32 : 1
		);*/

		/*printf("x0z0: collision: overlap: [xo=%lf, yo=%lf, zo=%lf]\n",
			
			space[s*s*(int)(position.x - 0.4f) + s * (int)(position.y - 0.9f) + (int)(position.z - 0.4f)] ?
				(double) (1.0f - (position.x - 0.4f) + floorf(position.x - 0.4f)) : 0.0,

			space[s*s*(int)(position.x - 0.4f) + s * (int)(position.y - 0.9f) + (int)(position.z - 0.4f)] ?
				(double) (1.0f - (position.y - 0.9f) + floorf(position.y - 0.9f)) : 0.0,

			space[s*s*(int)(position.x - 0.4f) + s * (int)(position.y - 0.9f) + (int)(position.z - 0.4f)] ?
				(double) (1.0f - (position.z - 0.4f) + floorf(position.z - 0.4f)) : 0.0

		);*/

		/*printf("can jump? %s\n", 
		not in_air and (
		space[s*s*(int)(position.x - 0.4f) + s * (int)(position.y - 1.91f) + (int)(position.z - 0.4f)] or
		space[s*s*(int)(position.x - 0.4f) + s * (int)(position.y - 1.91f) + (int)(position.z + 0.4f)] or
		space[s*s*(int)(position.x + 0.4f) + s * (int)(position.y - 1.91f) + (int)(position.z - 0.4f)] or
		space[s*s*(int)(position.x + 0.4f) + s * (int)(position.y - 1.91f) + (int)(position.z + 0.4f)])
			? 
			"\033[32myes\033[0m" : "\033[31mno\033[0m"

		);*/



// static const float forward_accel = 0.040f;
// static const float strafe_accel = 0.028f;
// static const float jump_accel = 0.23f;
// static const int32_t ms_delay_per_frame = 8;
// static struct vec3 velocity = {0, 0, 0};








//if (
				//not in_air and (
				//space[s*s*(int)(position.x - 0.4f) + s * (int)(position.y - 1.90001f) + (int)(position.z - 0.4f)] or
				//space[s*s*(int)(position.x - 0.4f) + s * (int)(position.y - 1.90001f) + (int)(position.z + 0.4f)] or
				//space[s*s*(int)(position.x + 0.4f) + s * (int)(position.y - 1.90001f) + (int)(position.z - 0.4f)] or
				//space[s*s*(int)(position.x + 0.4f) + s * (int)(position.y - 1.90001f) + (int)(position.z + 0.4f)])
			//) {
				
			//}

//if (
				//not in_air and (
				//space[s*s*(int)(position.x - 0.4f) + s * (int)(position.y - 1.90001f) + (int)(position.z - 0.4f)] or
				//space[s*s*(int)(position.x - 0.4f) + s * (int)(position.y - 1.90001f) + (int)(position.z + 0.4f)] or
				//space[s*s*(int)(position.x + 0.4f) + s * (int)(position.y - 1.90001f) + (int)(position.z - 0.4f)] or
				//space[s*s*(int)(position.x + 0.4f) + s * (int)(position.y - 1.90001f) + (int)(position.z + 0.4f)])
			//) {
				
			//}






		//velocity.y -= 0.017f;
		//velocity.x *= 0.75f;
		//velocity.z *= 0.75f;

		//position.x += velocity.x; 
		//position.y += velocity.y; 
		//position.z += velocity.z;


		/*
		if (space[s*s*(int)(position.x - 0.4f) + s * (int)(position.y - 1.9f) + (int)(position.z - 0.4f)] or 
		    space[s*s*(int)(position.x + 0.4f) + s * (int)(position.y - 1.9f) + (int)(position.z - 0.4f)] or
		    space[s*s*(int)(position.x - 0.4f) + s * (int)(position.y - 1.9f) + (int)(position.z + 0.4f)] or
		    space[s*s*(int)(position.x + 0.4f) + s * (int)(position.y - 1.9f) + (int)(position.z + 0.4f)]) {
			const float overlap = 1.0f - (position.y - 1.9f) + floorf(position.y - 1.9f);
			position.y += overlap;
			velocity.y = 0.0f;
			in_air = false;
		}

		if (space[s*s*(int)(position.x - 0.4f) + s * (int)(position.y - 1.9f) + (int)(position.z - 0.4f)] or 
		    space[s*s*(int)(position.x - 0.4f) + s * (int)(position.y - 1.9f) + (int)(position.z + 0.4f)]) {
			const float overlap = 1.0f - (position.x - 0.4f) + floorf(position.x - 0.4f);
			position.x += overlap;
			velocity.x = 0.0f;
		}


		if (space[s*s*(int)(position.x + 0.4f) + s * (int)(position.y - 1.9f) + (int)(position.z - 0.4f)] or
		    space[s*s*(int)(position.x - 0.4f) + s * (int)(position.y - 1.9f) + (int)(position.z - 0.4f)]) {
			const float overlap = 1.0f - (position.z - 0.4f) + floorf(position.z - 0.4f);
			position.z += overlap;
			velocity.z = 0.0f;
		}

		*/


































































/*
		if (space[s*s*(int)(position.x + 0.4f) + s * (int)(position.y - 1.9f) + (int)(position.z)]) {
			const float overlap = 1.0f - (position.x + 0.4f) + floorf(position.x + 0.4f);
			position.x += overlap;
			velocity.x = 0.0f;
		}




		if (space[s*s*(int)(position.x) + s * (int)(position.y - 1.9f) + (int)(position.z + 0.4f)]) {
			const float overlap = 1.0f - (position.z + 0.4f) + floorf(position.z + 0.4f);
			position.z += overlap;
			velocity.z = 0.0f;
		}
*/



/*







		if (space[s*s*(int)(position.x - 0.4f) + s * (int)(position.y - 1.9f) + (int)(position.z - 0.4f)]) {
			const float overlapx = 1.0f - (position.x - 0.4f) + floorf(position.x - 0.4f);
			const float overlapz = 1.0f - (position.z - 0.4f) + floorf(position.z - 0.4f);
			if (overlapx < overlapz) {
				position.z += overlapz;
				velocity.z = 0.0f;
			} else {
				position.x += overlapx;
				velocity.x = 0.0f;
			}
			
		}



*/














		//printf("position = {%3.3lf, %3.3lf, %3.3lf}\n", (double)position.x,(double)position.y,(double)position.z);
		//printf("velocity = {%3.3lf, %3.3lf, %3.3lf}\n", (double)velocity.x,(double)velocity.y,(double)velocity.z);

		//printf("yaw = %3.3lf, pitch = %3.3lf\n", (double)yaw, (double)pitch);
		//printf("forward = {%3.3lf, %3.3lf, %3.3lf}\n", (double)forward.x,(double)forward.y,(double)forward.z);
		//printf("right = {%3.3lf, %3.3lf, %3.3lf}\n", (double)right.x,(double)right.y,(double)right.z);
		//printf("up = {%3.3lf, %3.3lf, %3.3lf}\n", (double)up.x,(double)up.y,(double)up.z);

		//printf("\033[%um[still]\033[0m | \033[%um[x]\033[0m \033[%um[y]\033[0m \033[%um[z]\033[0m\n", 
		//	still_collides ? 32 : 1, 
		//	x_collides ? 32 : 1, 
		//	y_collides ? 32 : 1, 
		//	z_collides ? 32 : 1
		//);













/*

	bool y_collides = 0, x_collides = 0, z_collides = 0, 

		still_collides = 0,

		still_collides_xp = 0,
		still_collides_xm = 0,

		still_collides_yp = 0,
		still_collides_ym = 0,

		still_collides_zp = 0,
		still_collides_zm = 0

		;
*/














//if (still_collides_ym) {


	//	if (RectA.X1 < RectB.X2 && RectA.X2 > RectB.X1 &&
    	//		RectA.Y1 < RectB.Y2 && RectA.Y2 > RectB.Y1) 





/*

//} 
			//else  {

			//	printf("could not jump becuase @(%u,%u,%u), ie below you, was air..\n", 
					(int)(position.x), (int)(position.y - 1), (int)(position.z));
			//}








		still_collides_xp =
			!!space[s * s * (int)(position.x + 0.5) + 
			            s * (int)(position.y) + 
			                (int)(position.z)];
		still_collides_xm =
			!!space[s * s * (int)(position.x - 0.5) + 
			            s * (int)(position.y) + 
			                (int)(position.z)];

		still_collides_yp =
			!!space[s * s * (int)(position.x) + 
			            s * (int)(position.y + 1.0) + 
			                (int)(position.z)];
		still_collides_ym =
			!!space[s * s * (int)(position.x) + 
			            s * (int)(position.y - 1.0) + 
			                (int)(position.z)];


		still_collides_zp =
			!!space[s * s * (int)(position.x) + 
			            s * (int)(position.y) + 
			                (int)(position.z + 0.5)];
		still_collides_zm =
			!!space[s * s * (int)(position.x) + 
			            s * (int)(position.y) + 
			                (int)(position.z - 0.5)];


		still_collides = 	still_collides_xp or still_collides_xm or  
					still_collides_yp or still_collides_ym or  
					still_collides_zp or still_collides_zm ;
		
		





		y_collides =
			!!space[s * s * (int)(position.x - 0.5) + 
			            s * (int)(position.y - 1.0 + velocity.y) + 
			                (int)(position.z - 0.5)]

		||
			!!space[s * s * (int)(position.x + 0.5) + 
			            s * (int)(position.y - 1.0 + velocity.y) + 
			                (int)(position.z - 0.5)]

		||
			!!space[s * s * (int)(position.x - 0.5) + 
			            s * (int)(position.y - 1.0 + velocity.y) + 
			                (int)(position.z + 0.5)]

		||
			!!space[s * s * (int)(position.x + 0.5) + 
			            s * (int)(position.y - 1.0 + velocity.y) + 
			                (int)(position.z + 0.5)];






		x_collides =
			!!space[s * s * (int)(position.x - 0.5 + velocity.x) + 
			            s * (int)(position.y - 1.0) + 
			                (int)(position.z - 0.5)]


		||
			!!space[s * s * (int)(position.x + 0.5 + velocity.x) + 
			            s * (int)(position.y - 1.0) + 
			                (int)(position.z - 0.5)]


		||
			!!space[s * s * (int)(position.x - 0.5 + velocity.x) + 
			            s * (int)(position.y - 1.0) + 
			                (int)(position.z + 0.5)]


		||
			!!space[s * s * (int)(position.x + 0.5 + velocity.x) + 
			            s * (int)(position.y - 1.0) + 
			                (int)(position.z + 0.5)];

			





		z_collides =
			!!space[s * s * (int)(position.x - 0.5) + 
			            s * (int)(position.y - 1.0) + 
			                (int)(position.z - 0.5 + velocity.z)]

		||
			!!space[s * s * (int)(position.x + 0.5) + 
			            s * (int)(position.y - 1.0) + 
			                (int)(position.z - 0.5 + velocity.z)]


		||
			!!space[s * s * (int)(position.x - 0.5) + 
			            s * (int)(position.y - 1.0) + 
			                (int)(position.z + 0.5 + velocity.z)]

		||
			!!space[s * s * (int)(position.x + 0.5) + 
			            s * (int)(position.y - 1.0) + 
			                (int)(position.z + 0.5 + velocity.z)];
	




	//	if (not still_collides) { 
			//if (not x_collides) 			
			//else velocity.x *= -0.5;
			//if (not y_collides) 
			//else velocity.y *= -0.5;
			//if (not z_collides) 
			//else velocity.z *= -0.5;
	//	} else {
			//if (not still_collides and x_collides) velocity.x = 0;
			//if (not still_collides and y_collides) velocity.y = 0;
			//if (not still_collides and z_collides) velocity.z = 0;
		}

	//	if (still_collides_xp) position.x -= 0.001;
	//	if (still_collides_xm) position.x += 0.001;
		
	//	if (still_collides_yp) position.y -= 0.001;
	//	if (still_collides_ym) position.y += 0.001;

	//	if (still_collides_zp) position.z -= 0.001;
	//	if (still_collides_zm) position.z += 0.001;

		*/









































































































/*



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



	unsigned int VBOs[1], VAOs[1];
	glGenVertexArrays(1, VAOs); cc;
	glGenBuffers(1, VBOs);cc;
	glBindVertexArray(VAOs[0]);cc;
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);cc;
	glBufferData(GL_ARRAY_BUFFER, sizeof(firstTriangle), firstTriangle, GL_STATIC_DRAW);cc;
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);cc;
	glEnableVertexAttribArray(0);cc;
*/



		/*
		glUseProgram(shaderProgramOrange);cc;
		glBindVertexArray(VAOs[0]);cc;
		glDrawArrays(GL_TRIANGLES, 0, 3);cc;
		*/



//glDeleteVertexArrays(2, VAOs);
	//glDeleteBuffers(2, VBOs);
	//glDeleteProgram(shaderProgramOrange);



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



























/*

static void initCube(Cube *cube)
{

	glGenVertexArrays(1,&cube->vao);
	glBindVertexArray(cube->vao);
	glGenBuffers(2,cube->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, cube->vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cubeGeometry), cubeGeometry, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cube->vbo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeConnectivity), cubeConnectivity, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 0);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_TRUE, sizeof(Vertex), 3 * sizeof(float));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	cube->model = glm::mat4(1.0f);
}



static bool initShaders(CubeApp *app, const char *vs, const char *fs)
{
	
	app->program=programCreateFromFiles(vs, fs);
	if (app->program == 0)
		return false;

	app->locProjection=glGetUniformLocation(app->program, "projection");
	app->locModelView=glGetUniformLocation(app->program, "modelView");
	app->locTime=glGetUniformLocation(app->program, "time");

	return true;
}












static GLuint programCreate(GLuint vertex_shader, GLuint fragment_shader)
{
	GLuint program=0;
	GLint status;

	program=glCreateProgram();
	info("created program %u",program);

	if (vertex_shader)
		glAttachShader(program, vertex_shader);
	if (fragment_shader)
		glAttachShader(program, fragment_shader);


	glBindAttribLocation(program, 0, "pos");
	glBindAttribLocation(program, 1, "nrm");
	glBindAttribLocation(program, 2, "clr");
	glBindAttribLocation(program, 3, "tex");


	glBindFragDataLocation(program, 0, "color");


	info("linking program %u",program);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		warn("Failed to link program!");
		printInfoLog(program,true);
		glDeleteProgram(program);
		return 0;
	}
	return program;
}





















































typedef struct {
	GLuint vbo[2];
	GLuint vao;
	glm::mat4 model;
} Cube;

typedef enum {
	DEBUG_OUTPUT_DISABLED=0,
	DEBUG_OUTPUT_ERRORS_ONLY,
	DEBUG_OUTPUT_ALL
} DebugOutputLevel;

struct AppConfig {
	int posx;
	int posy;
	int width;
	int height;
	bool decorated;
	bool fullscreen;
	unsigned int frameCount;
	DebugOutputLevel debugOutputLevel;
	bool debugOutputSynchronous;

	AppConfig() :
		posx(100),
		posy(100),
		width(800),
		height(600),
		decorated(true),
		fullscreen(false),
		frameCount(0),
		debugOutputLevel(DEBUG_OUTPUT_DISABLED),
		debugOutputSynchronous(false)
	{}
};

typedef struct {

	GLFWwindow *win;
	int width, height;
	unsigned int flags;

	double timeCur, timeDelta;
	double avg_frametime;
	double avg_fps;
	unsigned int frame;

	bool pressedKeys[GLFW_KEY_LAST+1];
	bool releasedKeys[GLFW_KEY_LAST+1];

	Cube cube;

	GLuint program;	
	GLint locProjection;
	GLint locModelView;
	GLint locTime;
	glm::mat4 projection;
	glm::mat4 view;
} CubeApp;

struct Vertex {
	GLfloat pos[3]; 
	GLfloat clr[4];
};

static GLenum getGLError(const char *action, bool ignore=false, const char *file=NULL, const int line=0)
{
	GLenum e,err=GL_NO_ERROR;

	do {
		e=glGetError();
		if ( (e != GL_NO_ERROR) && (!ignore) ) {
			err=e;
			if (file)
				fprintf(stderr,"%s:",file);
			if (line)
				fprintf(stderr,"%d:",line);
			warn("GL error 0x%x at %s",(unsigned)err,action);
		}
	} while (e != GL_NO_ERROR);
	return err;
}

#define GL_ERROR_DBG(action) getGLError(action, false, __FILE__, __LINE__)


extern void APIENTRY debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity,
			  GLsizei length, const GLchar *message, const GLvoid* userParam) {
	const AppConfig *cfg=(const AppConfig*)userParam;

	switch(type) {
		case GL_DEBUG_TYPE_ERROR:
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
			if (cfg->debugOutputLevel >= DEBUG_OUTPUT_ERRORS_ONLY) {
				warn("GLDEBUG: %s %s %s [0x%x]: %s",
					translateDebugSourceEnum(source),
					translateDebugTypeEnum(type),
					translateDebugSeverityEnum(severity),
					id, message);
			}
			break;
		default:
			if (cfg->debugOutputLevel >= DEBUG_OUTPUT_ALL) {
				warn("GLDEBUG: %s %s %s [0x%x]: %s",
					translateDebugSourceEnum(source),
					translateDebugTypeEnum(type),
					translateDebugSeverityEnum(severity),
					id, message);
			}
	}
}


static void initGLState(const AppConfig&cfg)
{
	printGLInfo();
	listGLExtensions();
}

static  GLuint shaderCreateAndCompile(GLenum type, const GLchar *source) {
	GLuint shader=0;
	GLint status;

	shader=glCreateShader(type);
	info("created shader object %u",shader);
	glShaderSource(shader, 1, (const GLchar**)&source, NULL);
	info("compiling shader object %u",shader);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		warn("Failed to compile shader");
		printInfoLog(shader,false);
		glDeleteShader(shader);
		shader=0;
	}

	return shader;
}

static GLuint programCreate(GLuint vertex_shader, GLuint fragment_shader) {
	GLuint program=0;
	GLint status;

	program=glCreateProgram();

	if (vertex_shader)
		glAttachShader(program, vertex_shader);
	if (fragment_shader)
		glAttachShader(program, fragment_shader);

	glBindAttribLocation(program, 0, "pos");
	glBindAttribLocation(program, 1, "nrm");
	glBindAttribLocation(program, 2, "clr");
	glBindAttribLocation(program, 3, "tex");

	glBindFragDataLocation(program, 0, "color");

	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		return 0;
	}
	return program;
}

static GLenum programCreateFromFiles(const char *vs, const char *fs)
{
	GLuint id_vs=shaderCreateFromFileAndCompile(GL_VERTEX_SHADER, vs);
	GLuint id_fs=shaderCreateFromFileAndCompile(GL_FRAGMENT_SHADER, fs);
	GLuint program=programCreate(id_vs,id_fs);

	return program;
}

static void destroyShaders(CubeApp *app) {
	if (app->program) {
		info("deleting program %u",app->program);
		glDeleteProgram(app->program);
		app->program=0;
	}
}

static bool initShaders(CubeApp *app, const char *vs, const char *fs) {
	destroyShaders(app);
	app->program=programCreateFromFiles(vs, fs);
	if (app->program == 0)
		return false;

	app->locProjection=glGetUniformLocation(app->program, "projection");
	app->locModelView=glGetUniformLocation(app->program, "modelView");
	app->locTime=glGetUniformLocation(app->program, "time");
	return true;
}

bool initCubeApplication(CubeApp *app, const AppConfig& cfg) {
	int i;
	int w, h, x, y;
	bool debugCtx=(cfg.debugOutputLevel > DEBUG_OUTPUT_DISABLED);

	app->win=NULL;
	app->flags=0;
	app->avg_frametime=-1.0;
	app->avg_fps=-1.0;
	app->frame = 0;

	for (i=0; i<=GLFW_KEY_LAST; i++)
		app->pressedKeys[i]=app->releasedKeys[i]=false;

	app->cube.vbo[0]=app->cube.vbo[1]=app->cube.vao=0;
	app->program=0;

	info("initializing GLFW");
	if (!glfwInit()) {
		warn("Failed to initialze GLFW");
		return false;
	}

	app->flags |= APP_HAVE_GLFW;

	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, (debugCtx)?GL_TRUE:GL_FALSE);

	GLFWmonitor *monitor = NULL;
	x = cfg.posx;
	y = cfg.posy;
	w = cfg.width;
	h = cfg.height;

	if (cfg.fullscreen) {
		monitor = glfwGetPrimaryMonitor();
	}
	if (monitor) {
		glfwGetMonitorPos(monitor, &x, &y);
		const GLFWvidmode *v = glfwGetVideoMode(monitor);
		if (v) {
			w = v->width;
			h = v->height;
			info("Primary monitor: %dx%d @(%d,%d)", w, h, x, y);
		}
		else {
			warn("Failed to query current video mode!");
		}
	}

	if (!cfg.decorated) {
		glfwWindowHint(GLFW_DECORATED, GL_FALSE);
	}

	info("creating window and OpenGL context");
	app->win=glfwCreateWindow(w, h, APP_TITLE, monitor, NULL);
	if (!app->win) {
		warn("failed to get window with OpenGL 3.2 core context");
		return false;
	}

	app->width = w;
	app->height = h;

	if (!monitor) {
		glfwSetWindowPos(app->win, x, y);
	}

	glfwSetWindowUserPointer(app->win, app);
	glfwSetFramebufferSizeCallback(app->win, callback_Resize);
	glfwSetKeyCallback(app->win, callback_Keyboard);
	glfwMakeContextCurrent(app->win);
	glfwSwapInterval(1);

	info("initializing glad");
	if (!gladLoadGL(glfwGetProcAddress)) {
		warn("failed to intialize glad GL extension loader");
		return false;
	}

	if (!GLAD_GL_VERSION_3_2) {
		warn("failed to load at least GL 3.2 functions via GLAD");
		return false;
	}

	app->flags |= APP_HAVE_GL;


	initGLState(cfg);
	initCube(&app->cube);
	if (!initShaders(app,"shaders/color.vs.glsl","shaders/color.fs.glsl")) {
		warn("something wrong with our shaders...");
		return false;
	}

	
	app->timeCur=glfwGetTime();

	return true;
}

static void setProjectionAndView(CubeApp *app) {
	app->projection = glm::perspective(glm::radians(75.0f), (float)app->width / (float)app->height, 0.1f, 10.0f);
	app->view = glm::translate(glm::vec3(0.0f, 0.0f, -4.0f));
}

static void displayFunc(CubeApp *app, const AppConfig& cfg) {
	
	app->cube.model = glm::rotate(app->cube.model, (float)(glm::half_pi<double>() * app->timeDelta), glm::vec3(0.8f, 0.6f, 0.1f));
	glViewport(0, 0, app->width, app->height);
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	setProjectionAndView(app);
	glm::mat4 modelView = app->view * app->cube.model;
	glUseProgram(app->program);
	glUniformMatrix4fv(app->locProjection, 1, GL_FALSE, glm::value_ptr(app->projection));
	glUniformMatrix4fv(app->locModelView, 1, GL_FALSE, glm::value_ptr(modelView));
	glUniform1f(app->locTime, (GLfloat)app->timeCur);
	glBindVertexArray(app->cube.vao);
	glDrawElements(GL_TRIANGLES, 6 * 6, GL_UNSIGNED_SHORT, BUFFER_OFFSET(0));
	glBindVertexArray(0);
	glfwSwapBuffers(app->win);
	GL_ERROR_DBG("display function");
}

static void mainLoop(CubeApp *app, const AppConfig& cfg) {
	unsigned int frame = 0;
	double start_time = glfwGetTime();
	double last_time = start_time;

	info("entering main loop");
	while (!glfwWindowShouldClose(app->win)) {
		
		double now=glfwGetTime();
		app->timeDelta = now - app->timeCur;
		app->timeCur = now;

		double elapsed = app->timeCur - last_time;
		if (elapsed >= 1.0) {
			char WinTitle[80];
			app->avg_frametime=1000.0 * elapsed/(double)frame;
			app->avg_fps=(double)frame/elapsed;
			last_time=app->timeCur;
			frame=0;
			
			mysnprintf(WinTitle, sizeof(WinTitle), APP_TITLE "   /// AVG: %4.2fms/frame (%.1ffps)", app->avg_frametime, app->avg_fps);
			glfwSetWindowTitle(app->win, WinTitle);
			info("frame time: %4.2fms/frame (%.1ffps)",app->avg_frametime, app->avg_fps);
		}

		
		displayFunc(app, cfg);
		app->frame++;
		frame++;
		if (cfg.frameCount && app->frame >= cfg.frameCount) {
			break;
		}
		glfwPollEvents();
	}
	info("left main loop\n%u frames rendered in %.1fs seconds == %.1ffps",
		app->frame,(app->timeCur-start_time),
		(double)app->frame/(app->timeCur-start_time) );
}



int main (int argc, char **argv) {
	parseCommandlineArgs(cfg, argc, argv);

	initCubeApplication(&app, cfg);
	mainLoop(&app, cfg);

	glBindVertexArray(0);
	if (cube->vao) {
		info("Cube: deleting VAO %u", cube->vao);
		glDeleteVertexArrays(1,&cube->vao);
		cube->vao=0;
	}
	if (cube->vbo[0] || cube->vbo[1]) {
		info("Cube: deleting VBOs %u %u", cube->vbo[0], cube->vbo[1]);
		glDeleteBuffers(2,cube->vbo);
		cube->vbo[0]=0;
		cube->vbo[1]=0;
	}
	destroyShaders(app);
	glfwDestroyWindow(app->win);
	glfwTerminate();
}












vs:


#version 150 core

uniform mat4 modelView;
uniform mat4 projection;

in vec3 pos;
in vec4 clr;

out vec4 v_clr;

void main()
{
	v_clr = clr;
	gl_Position = projection * modelView * vec4(pos, 1.0);
}




fs:



#version 150 core

in vec4 v_clr;

out vec4 color;

void main()
{
	color = v_clr;
}





































#version 330 core

// Interpolated values from the vertex shaders
in vec2 UV;

// Ouput data
out vec3 color;

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;

void main(){

    // Output color = color of the texture at the specified UV
    color = texture( myTextureSampler, UV ).rgb;
}








#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;

// Output data ; will be interpolated for each fragment.
out vec2 UV;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;

void main(){

    // Output position of the vertex, in clip space : MVP * position
    gl_Position =  MVP * vec4(vertexPosition_modelspace,1);

    // UV of the vertex. No special space for this one.
    UV = vertexUV;
}


















// Two UV coordinatesfor each vertex. They were created with Blender. You'll learn shortly how to do this yourself.
static const GLfloat g_uv_buffer_data[] = {
    0.000059f, 1.0f-0.000004f,
    0.000103f, 1.0f-0.336048f,
    0.335973f, 1.0f-0.335903f,
    1.000023f, 1.0f-0.000013f,
    0.667979f, 1.0f-0.335851f,
    0.999958f, 1.0f-0.336064f,
    0.667979f, 1.0f-0.335851f,
    0.336024f, 1.0f-0.671877f,
    0.667969f, 1.0f-0.671889f,
    1.000023f, 1.0f-0.000013f,
    0.668104f, 1.0f-0.000013f,
    0.667979f, 1.0f-0.335851f,
    0.000059f, 1.0f-0.000004f,
    0.335973f, 1.0f-0.335903f,
    0.336098f, 1.0f-0.000071f,
    0.667979f, 1.0f-0.335851f,
    0.335973f, 1.0f-0.335903f,
    0.336024f, 1.0f-0.671877f,
    1.000004f, 1.0f-0.671847f,
    0.999958f, 1.0f-0.336064f,
    0.667979f, 1.0f-0.335851f,
    0.668104f, 1.0f-0.000013f,
    0.335973f, 1.0f-0.335903f,
    0.667979f, 1.0f-0.335851f,
    0.335973f, 1.0f-0.335903f,
    0.668104f, 1.0f-0.000013f,
    0.336098f, 1.0f-0.000071f,
    0.000103f, 1.0f-0.336048f,
    0.000004f, 1.0f-0.671870f,
    0.336024f, 1.0f-0.671877f,
    0.000103f, 1.0f-0.336048f,
    0.336024f, 1.0f-0.671877f,
    0.335973f, 1.0f-0.335903f,
    0.667969f, 1.0f-0.671889f,
    1.000004f, 1.0f-0.671847f,
    0.667979f, 1.0f-0.335851f
};





// Create one OpenGL texture
GLuint textureID;
glGenTextures(1, &textureID);

// "Bind" the newly created texture : all future texture functions will modify this texture
glBindTexture(GL_TEXTURE_2D, textureID);

// Give the image to OpenGL
glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);



*/











// 


// texture( atlas_texture, UV ).rgb




// sampler2D uTexture
// void main() {
// 	vec4 textureColor = texture(uTexture, aTexCoords);
//
//}

/*
#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 vertexPosition_modelspace;
layout(location = 1) in vec2 vertexUV;

// Output data ; will be interpolated for each fragment.
out vec2 UV;

// Values that stay constant for the whole mesh.
uniform mat4 MVP;

void main(){

    // Output position of the vertex, in clip space : MVP * position
    gl_Position =  MVP * vec4(vertexPosition_modelspace,1);

    // UV of the vertex. No special space for this one.
    UV = vertexUV;
}




#version 330 core

// Interpolated values from the vertex shaders
in vec2 UV;

// Ouput data
out vec3 color;

// Values that stay constant for the whole mesh.
uniform sampler2D myTextureSampler;

void main(){

    // Output color = color of the texture at the specified UV
    color = texture( myTextureSampler, UV ).rgb;
}









	static const float cubeGeometry[]={

		-1.0, -1.0,  1.0,    255,   0,   0, 255,
		 1.0, -1.0,  1.0,    192,   0,   0, 255,
		-1.0,  1.0,  1.0,    192,   0,   0, 255,
		 1.0,  1.0,  1.0,    128,   0,   0, 255,

		 1.0, -1.0, -1.0,      0, 255, 255, 255,
		-1.0, -1.0, -1.0,      0, 192, 192, 255,
		 1.0,  1.0, -1.0,      0, 192, 192, 255,
		-1.0,  1.0, -1.0,      0, 128, 128, 255,

		-1.0, -1.0, -1.0,      0, 255,   0, 255,
		-1.0, -1.0,  1.0,      0, 192,   0, 255,
		-1.0,  1.0, -1.0,      0, 192,   0, 255,
		-1.0,  1.0,  1.0,      0, 128,   0, 255,

		 1.0, -1.0,  1.0,    255,   0, 255, 255,
		 1.0, -1.0, -1.0,    192,   0, 192, 255,
		 1.0,  1.0,  1.0,    192,   0, 192, 255,
		 1.0,  1.0, -1.0,    128,   0, 128, 255,

		-1.0,  1.0,  1.0,      0,   0, 255, 255,
		 1.0,  1.0,  1.0,      0,   0, 192, 255,
		-1.0,  1.0, -1.0,      0,   0, 192, 255,
		 1.0,  1.0, -1.0,      0,   0, 128, 255,

		 1.0, -1.0,  1.0,    255, 255,   0, 255,
		-1.0, -1.0,  1.0,    192, 192,   0, 255,
		 1.0, -1.0, -1.0,    192, 192,   0, 255,
		-1.0, -1.0, -1.0,    128, 128,   0, 255,
	};




	static const unsigned cubeConnectivity[]={
		 0, 1, 2,  2, 1, 3,	
		 4, 5, 6,  6, 5, 7,	
		 8, 9,10, 10, 9,11,	
		12,13,14, 14,13,15,	
		16,17,18, 18,17,19,	
		20,21,22, 22,21,23
	};


*/







// static bool debug = false; // delete this
//static bool quit = false; // delete this
// static bool tab = false; // delete this
//static bool should_move_camera = true; // delete this
//static bool is_fullscreen = false; // delete this








/*
	const float e = 1.0 / 8;
	const float c = 4 * e;
	const float r = 3 * e;

	const float verticies[] = {
	   0, 0, 0,    c + 0, r + 0,
	   1, 0, 0,    c + e, r + 0,
	   1, 1, 0,    c + e, r + e,
	   0, 0, 0,    c + 0, r + 0,
	   1, 1, 0,    c + e, r + e,
	   0, 1, 0,    c + 0, r + e,
	};

	unsigned int vertex_count = 3 * 2;


							confirmed working vertex data!

*/













/*



int
main(int argc, char *argv[])
{
    int x, y;
    for(y=0; y<4000; y++)
        for(x=0; x<4000; x++)
            perlin2d(x, y, 0.1, 4);

    return 0;
}

*/









/*






static float Noise2D(float x, float y) {
	unsigned int n = (int) ((x + y) * 57);
	n = (n << 13) ^ n;
	return (1.0 - ( (n * ((n * n * 15731) + 789221) +  1376312589) & 0x7fffffff) / 1073741824.0f);
}


float PerlinNoise2D(float x, float y, int seed) {
	float floor_x = floor(x), floor_y = floor(y);
	auto s = Noise2D(floor_x, floor_y);
	auto t = Noise2D(floor_x + 1, floor_y);
	auto u = Noise2D(floor_x, floor_y + 1);
	auto v = Noise2D(floor_x +  1, floor_y + 1);

	auto frac_x = x- floor_x;
	auto frac_y = y - floor_y;
	auto value = s * (1 - frac_x) + t * frac_x;
	value += (u - s) * frac_y * (1 - frac_x);
	value += (v - t) * frac_x * frac_y;
	return  value;
}





static float perlin_2d(int x, int y, float gain, int octaves, int hgrid) {

	float total = 0.0f;
	float frequency = 1.0f / (float) hgrid;
	float amplitude = gain;
	float lacunarity = 2.0;

	for (int i = 0; i < octaves; ++i) {
		total += noise((float)x * frequency, (float)y * frequency) * amplitude;         
		frequency *= lacunarity;
		amplitude *= gain;
	} 

	return (total);
}


static float perlin_2d(float x, float y, float gain, int octaves) {

	float total = 0.0f;
	float frequency = 0.005;
	float amplitude = 1.0;
	float lacunarity = 2.0;

	for (int i = 0; i < octaves; ++i) {
		total += noise((float)x * frequency, (float)y * frequency) * amplitude;         
		frequency *= lacunarity;
		amplitude *= 0.5;
	} 

	return total;
}


function FractalBrownianMotion(x, y, numOctaves) {
	let result = 0.0;
	let amplitude = 1.0;
	let frequency = 0.005;

	for (let octave = 0; octave < numOctaves; octave++) {
		const n = amplitude * Noise2D(x * frequency, y * frequency);
		result += n;
		
		amplitude *= 0.5;
		frequency *= 2.0;
	}

	return result;
}*/












/*
	// set a flat world:
	for (int x = 0; x < s; x++) {
		for (int z = 0; z < s; z++) {
			for (int y = 0; y < 5; y++) {
				space[s * s * x + s * y + z] = stone_block;
			}
		}
	}

	// grass stair case:
	for (int x = 0; x < s; x++) {
		for (int z = 0; z < s; z++) {
			for (int y = 0; y < (x) % 10; y++) {
				if (not space[s * s * x + s * y + z]) space[s * s * x + s * y + z] = dirt_block;
			}
			if (not space[s * s * x + s * ((x) % 10) + z]) space[s * s * x + s * ((x) % 10) + z] = grass_block;
		}
	}

	// // make a 2x2 box:
	space[s * s * 1 + s * 10 + 1] = rand() % block_count;
	space[s * s * 1 + s * 10 + 2] = rand() % block_count;
	space[s * s * 1 + s * 11 + 1] = rand() % block_count;
	space[s * s * 1 + s * 11 + 2] = rand() % block_count;
	space[s * s * 2 + s * 10 + 1] = rand() % block_count;
	space[s * s * 2 + s * 10 + 2] = rand() % block_count;
	space[s * s * 2 + s * 11 + 1] = rand() % block_count;
	space[s * s * 2 + s * 11 + 2] = rand() % block_count;


	// // make a 2x2 box:
	space[s * s * 1 + s * 20 + 1] = rand() % block_count;
	space[s * s * 1 + s * 20 + 2] = rand() % block_count;
	space[s * s * 1 + s * 21 + 1] = rand() % block_count;
	space[s * s * 1 + s * 21 + 2] = rand() % block_count;
	space[s * s * 2 + s * 20 + 1] = rand() % block_count;
	space[s * s * 2 + s * 20 + 2] = rand() % block_count;
	space[s * s * 2 + s * 21 + 1] = rand() % block_count;
	space[s * s * 2 + s * 21 + 2] = rand() % block_count;

*/




