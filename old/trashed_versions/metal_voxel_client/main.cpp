// the cpp client for the metal-based voxel engine  game that we are writing. 

/*



202407033.020014:

	how to do this at all:



		1. we need to cast a ray    for each pixel on the screen,   (ie, the raycasting algorithm happens in the fragment shader)

			the ray is cast into the world,    and if its intersects something, a color is drawn, based on what kind of block we intersect with. each block type has a color. in fact, is defined by its color lol.

					we might edit this, to make each block not an entity that exists in its own right, but rather only the collection of blocks do? no.. nvm. we are just going to assign each block its own color, and have that information lookupable. 


		2. we arent going to use the vertex shader, because we have no verticies. in our entire game. 

		3. we are going to have to somehow give the entire world full of uint8_t's    to the fragment shader somehow. 

				additioally, we want to make the raycasting algorithm as efficient as possible... soooo yeah. idk how to do that yet 


		4. most of this game's display code will just be in the fragment shader, so we need to understand this part better. 









*/


#include <stdint.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>

#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <Metal/shared_ptr.hpp>
#include <QuartzCore/QuartzCore.hpp>
#include <simd/simd.h>
#include <SDL.h>

#include "triangle_metallib.h"

struct vertex {
	vector_float3 position;
	vector_float4 color;	
};

static int window_width = 1600;
static int window_height = 1000;
static float aspect = 1.6f;
static const float fovy = 1.22173f /*radians*/;
static const float znear = 0.01f;
static const float zfar = 1000.0f;
static const float camera_sensitivity = 0.005f;
static const float camera_accel = 0.00003f;
static const float drag = 0.95f;
static const int32_t ms_delay_per_frame = 8;
static bool debug = false;
static bool quit = false;
static bool tab = false;
static bool should_move_camera = true;
static bool is_fullscreen = false;
static int counter = 0;
static float delta = 0.0;
static float pitch = 0.0f, yaw = 0.0f;

struct vec3 { float x,y,z; };

static struct vec3 position = {10, 5, 10};
static struct vec3 velocity = {0, 0, 0};
static struct vec3 forward = 	{0, 0, -1};
static struct vec3 straight = 	{0, 0, 1};
static struct vec3 up = 	{0, 1, 0};
static struct vec3 right = 	{-1, 0, 0};


int main(void) { 
	srand((unsigned)time(NULL));

	const int s = 20;
	const int space_count = s * s * s;
	int8_t* space = (int8_t*) calloc(space_count, 1);

	for (int x = 1; x < s; x++) {
		for (int z = 1; z < s; z++) {
			const int y = 0;
			space[s * s * x + s * y + z] = rand() % 2;
		}
	}
	space[s * s * 1 + s * 1 + 1] = 1;
	space[s * s * 1 + s * 1 + 2] = 1;
	space[s * s * 1 + s * 2 + 1] = 1;
	space[s * s * 1 + s * 2 + 2] = 1;
	space[s * s * 2 + s * 1 + 1] = 1;
	space[s * s * 2 + s * 1 + 2] = 1;
	space[s * s * 2 + s * 2 + 1] = 1;
	space[s * s * 2 + s * 2 + 2] = 1;
	space[s * s * 4 + s * 4 + 4] = 1;


	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");
	if (SDL_Init(SDL_INIT_EVERYTHING)) exit(printf("SDL_Init failed: %s\n", SDL_GetError()));
	SDL_Window *window = SDL_CreateWindow("block game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
				window_width, window_height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	SDL_SetRelativeMouseMode(SDL_TRUE);

	auto swapchain = (CA::MetalLayer*)SDL_RenderGetMetalLayer(renderer);
	auto device = swapchain->device();
	auto name = device->name();
	printf("using device: \"%s\"...\n", name->utf8String());

	auto library_data = dispatch_data_create(&triangle_metallib[0], triangle_metallib_len, dispatch_get_main_queue(), ^{});
	NS::Error* err;

	auto library = MTL::make_owned(device->newLibrary(library_data, &err));
	if (not library) { printf("Failed to create library"); exit(1); } 

	auto vertex_function_name = NS::String::string("vertexShader", NS::ASCIIStringEncoding);
	auto vertex_function = MTL::make_owned(library->newFunction(vertex_function_name));

	auto fragment_function_name = NS::String::string("fragmentShader", NS::ASCIIStringEncoding);
	auto fragment_function = MTL::make_owned(library->newFunction(fragment_function_name));

	auto pipeline_descriptor = MTL::make_owned(MTL::RenderPipelineDescriptor::alloc()->init());
	pipeline_descriptor->setVertexFunction(vertex_function.get());
	pipeline_descriptor->setFragmentFunction(fragment_function.get());

	auto color_attachment_descriptor = pipeline_descriptor->colorAttachments()->object(0);
	color_attachment_descriptor->setPixelFormat(swapchain->pixelFormat());

	auto pipeline = MTL::make_owned(device->newRenderPipelineState(pipeline_descriptor.get(), &err));
	if (not pipeline) { printf("Failed to create pipeline"); exit(1); } 

	auto queue = MTL::make_owned(device->newCommandQueue());
	

	/*  // unsigned int my_vertex_count = 6;
	const struct vertex triangleVertices[] = {
		{ {  0.5,  0.5, 0 } },
		{ {  0.5, -0.5, 0 } },
		{ { -0.5,  0.5, 0 } },
		{ { -0.5, -0.5, 0 } },
		{ { -0.5,  0.5, 0 } },
		{ {  0.5, -0.5, 0 } }
	};*/


#define e 700

	unsigned int my_vertex_count = 6;
	const struct vertex triangleVertices[] = {
		{ {  e,  e, 0 }, 	{ 1, 0, 0, 1 } },
		{ {  e, -e, 0 }, 	{ 0, 1, 0, 1 } },
		{ { -e,  e, 0 }, 	{ 0, 0, 1, 1 } },

		{ { -e, -e, 0 }, 	{ 1, 1, 1, 1 } },
		{ { -e,  e, 0 }, 	{ 0, 0, 1, 1 } },
		{ {  e, -e, 0 }, 	{ 0, 1, 0, 1 } },
	};
	
	
	bool quit = false;
	while (not quit) {

		uint32_t start = SDL_GetTicks();

		SDL_Event event;
		while (SDL_PollEvent(&event)) {

			const Uint8* key = SDL_GetKeyboardState(0);

			if (event.type == SDL_QUIT) quit = true;

			else if (event.type == SDL_WINDOWEVENT_RESIZED) {     // <--------- why is this not getting called...!?!?!
				printf("window was resized!!\n");
				int w=0,h=0;
				SDL_GetWindowSize(window, &w, &h);
				printf("width = %d, height = %d", w,h);
				window_width = w;
				window_height = h;
				aspect = (float) window_width / (float) window_height;

				// perspective(perspective_matrix, fovy, aspect, znear, zfar);
			}

			else if (event.type == SDL_MOUSEMOTION and should_move_camera) {
				
    				float dx = (float) event.motion.xrel;
    				float dy = (float) event.motion.yrel;

				yaw -= camera_sensitivity * dx;
				pitch += camera_sensitivity * dy;
	
				// move_camera();
			}

			else if (event.type == SDL_KEYDOWN) {
				if (key[SDL_SCANCODE_ESCAPE]) quit = true; 		// temporary, for debugging. 
				if (tab and key[SDL_SCANCODE_Q]) quit = true; 
				if (tab and key[SDL_SCANCODE_0]) debug = !debug;
				if (tab and key[SDL_SCANCODE_1]) { // pause game.
					should_move_camera = not should_move_camera;
					SDL_SetRelativeMouseMode((SDL_bool) should_move_camera);
				}

				if (tab and key[SDL_SCANCODE_2]) {
					is_fullscreen = not is_fullscreen;
					SDL_SetWindowFullscreen(window, is_fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
					int w = 0, h = 0;
					SDL_GetWindowSize(window, &w, &h);

					printf("width = %d, height = %d", w,h);
					window_width = w;
					window_height = h;
					aspect = (float) window_width / (float) window_height;
					// perspective(perspective_matrix, fovy, aspect, znear, zfar);
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
			// move_camera();
		}
		if (key[SDL_SCANCODE_J]) { 
			yaw += 0.08f;
			// move_camera();
		}
		if (key[SDL_SCANCODE_I]) { 
			pitch -= 0.08f;
			// move_camera();
		}
		if (key[SDL_SCANCODE_K]) { 
			pitch += 0.08f;
			// move_camera();
		}


		// matrix stuff?

		auto drawable = swapchain->nextDrawable();
		auto pass = MTL::make_owned(MTL::RenderPassDescriptor::renderPassDescriptor());

		auto color_attachment = pass->colorAttachments()->object(0);
		color_attachment->setLoadAction(MTL::LoadAction::LoadActionClear);
		color_attachment->setStoreAction(MTL::StoreAction::StoreActionStore);
		color_attachment->setTexture(drawable->texture());

		const vector_uint2 viewport = { (unsigned int) window_width, (unsigned int) window_height };

		printf("view port = width(x) = %u, height(y) = %u\n", window_width, window_height);

		auto buffer = MTL::make_owned(queue->commandBuffer());
		auto encoder = MTL::make_owned(buffer->renderCommandEncoder(pass.get()));
		encoder->setViewport(MTL::Viewport { 
			(double) window_width / 2.0, (double) window_height / 2.0, 
			(double) window_width, (double) window_height, 
			-0.001, 10000.0
		});
		encoder->setRenderPipelineState(pipeline.get());
		//encoder->setVertexBytes(triangleVertices, sizeof(triangleVertices), 0);
		//encoder->setVertexBytes(&viewport, sizeof(viewport), 1);
		encoder->drawPrimitives(MTL::PrimitiveType::PrimitiveTypeTriangle, NS::UInteger(0), 4);
		encoder->endEncoding();
		buffer->presentDrawable(drawable);
		buffer->commit();
		drawable->release();











	/*


	things to googl:

		- how to write a compute shader in metal

		- how to write directly to the next swapchain buffer   in metal 

		- best way to make a voxel raycasting engine 

		- 




			lets modify how we are doing this: 

				we are instead going to use      [[kernel]]   ie metal kernels   to do compute shaders,  and then we are just going to write directly to the frame buffer probably. 
	

	*/











		velocity.x *= drag;
		velocity.y *= drag;
		velocity.z *= drag;

		position.x += delta * velocity.x;
		position.y += delta * velocity.y;
		position.z += delta * velocity.z;


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
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}




















/*


		//encoder->setVertexBytes(&matrix, sizeof(matrix), 2); // set the matrix here!!! 

#include "atlas.h"

struct vec3 { float x,y,z; };
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

static inline void look_at(mat4 result, struct vec3 eye, struct vec3 f, struct vec3 given_up) {
	
	struct vec3 s = normalize(cross(f, given_up));
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

// todo: make this a function. 
#define push_vertex(xo, yo, zo, u, v) 			\
	do {						\
	verticies[raw_count++] = (float)x + xo;		\
	verticies[raw_count++] = (float)y + yo;		\
	verticies[raw_count++] = (float)z + zo;		\
	verticies[raw_count++] = (float) u;		\
	verticies[raw_count++] = (float) v;		\
	vertex_count++;					\
	} while(0);








	uint8_t pixel_bytes[64 * 64 * 4] = {0};
	for (unsigned i = 0; i < 64 * 64; i++) {
		pixel_bytes[i * 4 + 0] = 0xff & (atlas[i] >> 16);
		pixel_bytes[i * 4 + 1] = 0xff & (atlas[i] >> 8);
		pixel_bytes[i * 4 + 2] = 0xff & (atlas[i] >> 0);
		pixel_bytes[i * 4 + 3] = 0xff;
	}












	size_t vertex_count = 0, raw_count = 0;//, index_count = 0;
	//unsigned* indicies = malloc(sizeof(unsigned) * space_count * );
	float* verticies = (float*) malloc(sizeof(float) * space_count * 6 * 6 * 5);

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
				printf("generating block at <%d,%d,%d>...\n", x,y,z);

				//block--;

				const float ut = 0;//(float) top_x[block] / 64.0f;
				const float vt = 0;//(float) top_y[block] / 64.0f;
				//const float ub = (float) bottom_x[block] / 64.0f;
				//const float vb = (float) bottom_y[block] / 64.0f;
				//const float us = (float) sides_x[block] / 64.0f;
				//const float vs = (float) sides_y[block] / 64.0f;
				
				const float e = 1;//8.0f / 64.0f;
				const float _ = 0;
				
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

	puts("debug: our verticies data is: ");
	for (size_t i = 0; i < vertex_count * 5; i++) {
		if (i % 5 == 0) puts("");
		printf("%.2f, ", (double) verticies[i]);
	}
	puts("");

	float* view_matrix = (float*) calloc(16, 4);
	float* perspective_matrix = (float*) calloc(16, 4);
	float* matrix = (float*) calloc(16, 4);
	float* copy = (float*) calloc(16, 4);

	straight = cross(right, up);
	perspective(perspective_matrix, fovy, aspect, znear, zfar);


	// for testing:             












		//const int32_t sleep = ms_delay_per_frame - ((int32_t) SDL_GetTicks() - (int32_t) start);
		//if (sleep > 0) SDL_Delay((uint32_t) sleep);



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



















	202404114.154655:

			we are currently in the middle of trying to get  3D rendering  to work:


				- we need to pass the perspective/transform matrix to the vertex shader, 

				- we need to switch the triangleVerticies variable to use our actual vertex data, 

				- we need to figure out how to write our vertex and fragment shader .metal  code...

				- we need to eventuallyyy do an indexing on our verticies, for efficiency.

			x	- we need to figure out how to enable back-face-culling. 

			x	- we need to figure out how to render a texture using Metal. 



	https://donaldpinckney.com/metal/2018/07/05/metal-intro-1.html


	https://www.kodeco.com/books/metal-by-tutorials/v2.0/chapters/3-the-rendering-pipeline




	THIS ONE!!

	https://www.haroldserrano.com/blog/rendering-3d-objects-in-metal





	
		halpppp

		


	*/
















































