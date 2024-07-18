/*  202407055.232919:   dwrr   

 opencl-based voxel sandbox 3d game.

    still a work in progress!

*/
#include <time.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <iso646.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <assert.h>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#include <SDL.h>

typedef uint64_t nat;
struct vec3 { float x, y, z; };


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




int main(void) {


srand((unsigned)time(NULL));

	const int s = 100;
	const int space_count = s * s * s;
	int8_t* space = calloc(space_count, 1);

	for (int i = 0; i < space_count; i++) {
		space[i] = (rand() % 2) * (rand() % 2) * (rand() % 2) * (rand() % 2) * (rand() % 2) * (rand() % 40);
	}

	for (int x = 1; x < 10; x++) {
		for (int z = 1; z < 10; z++) {
			const int y = 0;
			space[s * s * x + s * y + z] = 1;
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

	struct vec3 right =    {1, 0, 0};
	struct vec3 up =       {0, 1, 0};
	struct vec3 straight = {0, 0, 1};
	struct vec3 forward =  {0, 0, 1};
	struct vec3 position = {10, 5, 10};
	struct vec3 velocity = {0, 0, 0};
	float yaw = 0.0f, pitch = 0.0f;

	const float camera_sensitivity = 0.005;
	const float pi_over_2 = 1.57079632679f;
	const float camera_accel = 0.05f;


	if (SDL_Init(SDL_INIT_EVERYTHING)) exit(printf("init: %s\n", SDL_GetError()));

	int window_width = 1600, window_height = 1000;

	SDL_Window *window = SDL_CreateWindow(
		"voxel game", 
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
		window_width, window_height,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
	);

	SDL_Surface* surface = SDL_GetWindowSurface(window);
	SDL_SetRelativeMouseMode(1);
	
	bool quit = false;
	bool resized = true;




		//if (block == 1) { color = (uint32_t) ~0; }
		//if (block == 2) { color = 255; }
		//if (block == 3) { color = 255 << 8; }
		//if (block == 4) { color = 255 << 16; }



	uint32_t colors[256] = {0};

	for (int i = 1; i < 256; i++) {

		const uint32_t R = (rand() % 256U) << 0U;
		const uint32_t G = (rand() % 256U) << 8U;
		const uint32_t B = (rand() % 256U) << 16U;
		const uint32_t A = 255U << 24U;
		colors[i] = R | G | B | A;
	}





	while (not quit) {

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			const Uint8* key = SDL_GetKeyboardState(0);
			if (event.type == SDL_QUIT) {
				quit = true;
			} else if (event.type == SDL_WINDOWEVENT) {

				if (	event.window.event == SDL_WINDOWEVENT_RESIZED or 
					event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) resized = true;

			} else if (event.type == SDL_MOUSEMOTION) {

				// printf("motion %u %u : ", event.motion.x, event.motion.y);

				float dx = (float) event.motion.xrel;
    				float dy = (float) event.motion.yrel;

				yaw += camera_sensitivity * dx;
				pitch -= camera_sensitivity * dy;

				//printf("mouse:    yaw = %lf, pitch = %lf\n", yaw, pitch);
	
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

			} else if (event.type == SDL_KEYDOWN) {
				if (key[SDL_SCANCODE_ESCAPE]) quit = true;
				if (key[SDL_SCANCODE_Q]) quit = true; 
				if (key[SDL_SCANCODE_Z]) SDL_SetRelativeMouseMode(0);
				if (key[SDL_SCANCODE_G]) SDL_SetRelativeMouseMode(1);
			}
		}

		const Uint8* key = SDL_GetKeyboardState(0);

		if (key[SDL_SCANCODE_SPACE]) {
			velocity.x -= camera_accel * up.x;
			velocity.y -= camera_accel * up.y;
			velocity.z -= camera_accel * up.z;
		}

		if (key[SDL_SCANCODE_A]) { 
			velocity.x += camera_accel * up.x;
			velocity.y += camera_accel * up.y;
			velocity.z += camera_accel * up.z;
		}

		if (key[SDL_SCANCODE_E]) { 
			velocity.x += camera_accel * straight.x;
			velocity.y += camera_accel * straight.y;
			velocity.z += camera_accel * straight.z;
		}
		if (key[SDL_SCANCODE_D]) { 
			velocity.x -= camera_accel * straight.x;
			velocity.y -= camera_accel * straight.y;
			velocity.z -= camera_accel * straight.z;
		}

		if (key[SDL_SCANCODE_S]) {
			velocity.x -= camera_accel * right.x;
			velocity.y -= camera_accel * right.y;
			velocity.z -= camera_accel * right.z;
		}
		
		if (key[SDL_SCANCODE_F]) {
			velocity.x += camera_accel * right.x;
			velocity.y += camera_accel * right.y;
			velocity.z += camera_accel * right.z;
		}

		if (resized) {
			resized = 0;
			SDL_GetWindowSize(window, &window_width, &window_height);
			printf("Resizing, now: window_width = %d,  window_height = %d\n", 
				window_width, window_height
			);
			surface = SDL_GetWindowSurface(window);
			//SDL_UpdateWindowSurface(window);
		}

		SDL_LockSurface(surface);

		const int screen_w = surface->w;
		const int screen_h = surface->h;

		const float fov = 0.05f;

		for (int y = 0; y < screen_h; y += 25) {
			for (int x = 0; x < screen_w; x += 25) {

				uint32_t color = 0;

				const float xr = (float) x / (float) screen_w;
				const float yr = (float) y / (float) screen_h;

				const struct vec3 top = cross(forward, right);

				struct vec3 step = forward;
				step.x /= 20;
				step.y /= 20;
				step.z /= 20;

				const float st_x = -fov + 2 * fov * xr;
				const float st_y = -fov + 2 * fov * yr;
				
				step.x += top.x * st_y;
				step.y += top.y * st_y;
				step.z += top.z * st_y;

				step.x += right.x * st_x;
				step.y += right.y * st_x;
				step.z += right.z * st_x;

				struct vec3 ray = position;

				for (int n = 0; n < 400; n++) {      


					// NEXT STEP:    DO   the DDA ALGORITHM  don't step 0.1 per 
					//             ray step, step a whole block, always!!! nice. 


					// thennnn think about doing gpu stuff. 
					// honestly we could even just make this more efficient by other means first, though. 
					// doing it on the gpu is probably the last step. yay. 





					int px = (int) ray.x;
					int py = (int) ray.y;
					int pz = (int) ray.z;
					
					const int8_t block = space[s * s * pz + s * py + px];
					
					if (block) {
						color = colors[block];
						break;
					}

					ray.x = fmodf(ray.x + step.x + (float)s, (float)s);
					ray.y = fmodf(ray.y + step.y + (float)s, (float)s);
					ray.z = fmodf(ray.z + step.z + (float)s, (float)s);
				}

				for (int i = 0; i < 5; i++) {
					if (y + i >= screen_h) continue;
					for (int j = 0; j < 5; j++) {
						if (x + j >= screen_w) break;
						((uint32_t*)surface->pixels)[(y + i) * surface->w + (x + j)] = color;
					}
				}
			}
		}

		SDL_UnlockSurface(surface);
		SDL_UpdateWindowSurface(window);
		nanosleep((const struct timespec[]){{0, 16000000L}}, NULL);

		velocity.x *= 0.96f;
		velocity.y *= 0.96f;
		velocity.z *= 0.96f;
		position.x += velocity.x;
		position.y += velocity.y;
		position.z += velocity.z;

		position.x = fmodf(position.x + (float)s, (float)s);
		position.y = fmodf(position.y + (float)s, (float)s);
		position.z = fmodf(position.z + (float)s, (float)s);
	}
	SDL_DestroyWindow(window);
	SDL_Quit();
}
















			//} else if (event.type == SDL_WINDOWEVENT_FULLSCREEN) {

			//	SDL_Surface* surface = SDL_GetWindowSurface(window);
			//	printf("window was resized!!\n");





				//	printf("window was resized!!\n");

				// todo: handle all of even.window.event equal to  
				///  SDL_WINDOWEVENT_RESIZED or 
				//	SDL_WINDOWEVENT_SIZE_CHANGED or 
				//	SDL_WINDOWEVENT_MAXIMIZED or 
				//	SDL_WINDOWEVENT_RESTORED

				//SDL_Surface* surface = SDL_GetWindowSurface(window);

				/*	SDL_GetWindowSize(window, &window_width, &window_height);
					printf("Resizing, now: window_width = %d,  window_height = %d\n", 
						window_width, window_height
					);

					// SDL_SetWindowSize(window, width, height);
					SDL_FreeSurface(surface);
					surface = SDL_GetWindowSurface(window);
					SDL_BlitSurface(image,NULL,surface,NULL);
					SDL_UpdateWindowSurface(window);
					}
				*/






