// client for a 3d block game using opengl and sdl2.
#include <iso646.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include <SDL2/SDL.h>
#include <OpenGL/gl3.h>


static const char* vertex_shader_code = "        	\n\
#version 120                              		\n\
                                                        \n\
attribute vec3 position;                                \n\
attribute float block;                                  \n\
                               				\n\
varying float block_type;                              	\n\
uniform mat4 transform;					\n\
                                          		\n\
void main() {                                		\n\
	gl_Position = transform * vec4(position, 1.0);              \n\
	block_type = block;                             \n\
}                                                       \n";


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
	else gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);				\n\
}                               	 					\n";








// float* result = malloc(4 * 4 * sizeof(float));  calll using this sized mat4.

float* perspective(float* result, float fovy, float aspect, float zNear, float zFar) {
   
	const float t = tan(fovy / 2.0f);
	result[0 * 4 + 0] = 1.0f / (aspect * t);
	result[4 * 1 + 1] = 1.0f / t;
	result[4 * 2 + 2] = -(zFar + zNear) / (zFar - zNear);
	result[4 * 2 + 3] = -1.0f;
	result[4 * 3 + 2] = -(2.0f * zFar * zNear) / (zFar - zNear);
	return result;
}

mat4x4 lookAt(vec3 const & eye, vec3  const & center, vec3  const & up)
{
    vec3  f = normalize(center - eye);
    vec3  u = normalize(up);
    vec3  s = normalize(cross(f, u));
    u = cross(s, f);


// Maths::Matrix4 mat;
// Maths::Vector3 z = Maths::normalize(target - position);
// Maths::Vector3 x = Maths::normalize(Maths::crossProduct(z, up));
// Maths::Vector3 y = Maths::crossProduct(x, z);




    mat4x4 Result(1);
    Result[0][0] = s.x;
    Result[1][0] = s.y;
    Result[2][0] = s.z;
    Result[0][1] = u.x;
    Result[1][1] = u.y;
    Result[2][1] = u.z;
    Result[0][2] =-f.x;
    Result[1][2] =-f.y;
    Result[2][2] =-f.z;
    Result[3][0] =-dot(s, eye);
    Result[3][1] =-dot(u, eye);
    Result[3][2] = dot(f, eye);
    return Result;
}

vec3 normalize(const vec3 &v)
{
   float length_of_v = sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
   return vec3(v.x / length_of_v, v.y / length_of_v, v.z / length_of_v);
}



int mult(int A[N][N], int B[N][N]) {
    int C[N][N];

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            int num = 0;
            for (int k = 0; k < N; k++) {
                num += A[i][k] * B[k][j];
            }
            C[i][j] = num;
            std::cout << num << " ";
        }
        std::cout << std::endl;
    }

    return 0;
}





int main() {

	if (SDL_Init(SDL_INIT_VIDEO)) exit(printf("SDL_Init failed: %s\n", SDL_GetError()));

	SDL_Window *window = SDL_CreateWindow("block game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL);
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

	glEnable(GL_DEPTH_TEST);
	

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

	// ---------------------------------------------------

	int vertex_count = 3;

	float positions[] = {
		-0.5, -0.5, 0.0,
		0.0, 0.5, 0.0,
		0.5, -0.5, 0.0,
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
	
	GLuint vertex_array_block_buffer;
	glGenBuffers(1, &vertex_array_block_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_array_block_buffer);
	glEnableVertexAttribArray(attribute_block);
	glVertexAttribPointer(attribute_block, 1, GL_FLOAT, GL_FALSE, 0, 0);


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

	
	
	bool quit = false;

	int counter = 0;

	while (not quit) {
		uint32_t start = SDL_GetTicks();

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			const Uint8* key = SDL_GetKeyboardState(0);
			if (event.type == SDL_QUIT) quit = true;
			if (event.type == SDL_KEYDOWN) {

				if (key[SDL_SCANCODE_Q]) quit = true;
				if (key[SDL_SCANCODE_ESCAPE]) quit = true;

				if (key[SDL_SCANCODE_E]) {
					block_types[0]++;
					block_types[1]++;
					block_types[2]++;
				}

				if (key[SDL_SCANCODE_W]) {
					block_types[0]--;
					block_types[1]--;
					block_types[2]--;
				}

				if (key[SDL_SCANCODE_F]) {
					positions[0] += 0.001;
				}

				if (key[SDL_SCANCODE_D]) {
					positions[0] -= 0.001;
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

		glDrawArrays(GL_TRIANGLES, 0, vertex_count);

		SDL_GL_SwapWindow(window);



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
