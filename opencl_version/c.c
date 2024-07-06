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

int main(void) {


srand((unsigned)time(NULL));

	const int s = 100;
	const int space_count = s * s * s;
	int8_t* space = calloc(space_count, 1);

	for (int i = 0; i < space_count; i++) {
		space[i] = (rand() % 2) * (rand() % 2) * (rand() % 2) * (rand() % 2);
	}

	for (int x = 1; x < 10; x++) {
		for (int z = 1; z < 10; z++) {
			const int y = 0;
			if (rand() % 2) space[s * s * x + s * y + z] = rand() % 3;
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







	struct vec3 position = {10, 5, 10};
	struct vec3 velocity = {0, 0, 0};
	// struct vec3 forward = {0, 0, -1};
	struct vec3 straight = {0, 0, 1};
	struct vec3 up = {0, 1, 0};
	struct vec3 right = {-1, 0, 0};

	if (SDL_Init(SDL_INIT_EVERYTHING)) exit(printf("init: %s\n", SDL_GetError()));

	SDL_Window *window = SDL_CreateWindow(
		"voxel game", 
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
		1600, 1000,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
	);

	SDL_Surface* surface = SDL_GetWindowSurface(window);

	SDL_SetRelativeMouseMode(1);
	
	bool quit = false;
	bool tab = false;
	while (not quit) {

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			const Uint8* key = SDL_GetKeyboardState(0);
			if (event.type == SDL_QUIT) quit = true;
			else if (event.type == SDL_WINDOWEVENT_RESIZED) {
				printf("window was resized!!\n");

			} else if (event.type == SDL_MOUSEMOTION) {
				printf("motion %u %u\n", event.motion.x, event.motion.y);

			} else if (event.type == SDL_KEYDOWN) {
				if (key[SDL_SCANCODE_ESCAPE]) quit = true;
				if (tab and key[SDL_SCANCODE_Q]) quit = true; 
				if (tab and key[SDL_SCANCODE_0]) SDL_SetRelativeMouseMode(0);
				if (tab and key[SDL_SCANCODE_1]) SDL_SetRelativeMouseMode(1);
			}
		}

		const Uint8* key = SDL_GetKeyboardState(0);
		
		tab = !!key[SDL_SCANCODE_TAB];


		const double camera_accel = 0.05;

		if (key[SDL_SCANCODE_SPACE]) {
			velocity.x -=  camera_accel * up.x;
			velocity.y -=  camera_accel * up.y;
			velocity.z -=  camera_accel * up.z;
		}

		if (key[SDL_SCANCODE_A]) { 
			velocity.x +=  camera_accel * up.x;
			velocity.y +=  camera_accel * up.y;
			velocity.z +=  camera_accel * up.z;
		}

		if (key[SDL_SCANCODE_E]) { 
			velocity.x +=  camera_accel * straight.x;
			velocity.y +=  camera_accel * straight.y;
			velocity.z +=  camera_accel * straight.z;
		}
		if (key[SDL_SCANCODE_D]) { 
			velocity.x -=  camera_accel * straight.x;
			velocity.y -=  camera_accel * straight.y;
			velocity.z -=  camera_accel * straight.z;
		}

		if (key[SDL_SCANCODE_S]) {
			velocity.x +=  camera_accel * right.x;
			velocity.y +=  camera_accel * right.y;
			velocity.z +=  camera_accel * right.z;
		}
		
		if (key[SDL_SCANCODE_F]) {
			velocity.x -= camera_accel * right.x;
			velocity.y -= camera_accel * right.y;
			velocity.z -= camera_accel * right.z;
		}


		SDL_LockSurface(surface);

		const int screen_w = 3000; // surface->w;
		const int screen_h = 2000; // surface->h;

		for (int y = 0; y < screen_h; y += 30) {
			for (int x = 0; x < screen_w; x += 30) {

				uint32_t color = 0;

				const float xr = (float) x / (float) screen_w;
				const float yr = (float) y / (float) screen_h;

				struct vec3 step = {0.0, 0.0, 0.1f};

				step.x = -0.3f + 0.6f * xr;
				step.y = -0.3f + 0.6f * yr;

				struct vec3 ray = position;

				for (int n = 0; n < 50; n++) {

					int px = (int) ray.x;
					int py = (int) ray.y;
					int pz = (int) ray.z;
					px = (px + s * 3) % s;
					py = (py + s * 3) % s;
					pz = (pz + s * 3) % s;
					const int8_t block = space[s * s * pz + s * py + px];
					
					if (block) {
						if (block == 1) { color = (uint32_t) ~0; }
						if (block == 2) { color = 255; }
						if (block == 3) { color = 255 << 8; }
						if (block == 4) { color = 255 << 16; }
						break;
					}

					ray.x += step.x;
					ray.y += step.y;
					ray.z += step.z;
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
		nanosleep((const struct timespec[]){{0, 33000000L}}, NULL);

		velocity.x *= 0.95f;
		velocity.y *= 0.95f;
		velocity.z *= 0.95f;
		position.x += velocity.x;
		position.y += velocity.y;
		position.z += velocity.z;

		if (position.x >= s) position.x = 0;
		if (position.y >= s) position.y = 0;
		if (position.z >= s) position.z = 0;
	}
	SDL_DestroyWindow(window);
	SDL_Quit();
}













