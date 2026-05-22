/*  202410233.231453: dwrr
  a shell / editor application 
  that doesnt use the terminal at all.
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
#include <string.h>
#include <assert.h>

#include <SDL.h>

typedef uint64_t nat;

int main(void) {
	srand(42);

	int window_width = 1600, window_height = 1000;

	if (SDL_Init(SDL_INIT_EVERYTHING)) abort();	
	SDL_Window *window = SDL_CreateWindow(
		"editor", 
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
		window_width, window_height,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
	);
	SDL_Surface* surface = SDL_GetWindowSurface(window);
	
	bool quit = false;
	bool resized = false;

	bool task_running = false;

	SDL_GetWindowSize(window, &window_width, &window_height);
	surface = SDL_GetWindowSurface(window);

	size_t pixel_count = (size_t) surface->w * (size_t) surface->h;

	nat row = 0, column = 0, lpixel_size = 300;
	nat row_count = surface->h / lpixel_size - 1;
	nat column_count = surface->w / lpixel_size - 1;
	
	while (not quit) {
		SDL_Event event;
	input_loop: 
		if (task_running ? not SDL_PollEvent(&event) : not SDL_WaitEvent(&event)) goto input_done;
		const Uint8* key = SDL_GetKeyboardState(0);
		if (event.type == SDL_QUIT) { quit = true; }
		else if (event.type == SDL_WINDOWEVENT) {
			if (	event.window.event == SDL_WINDOWEVENT_RESIZED or 
				event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) resized = true;
		} else if (event.type == SDL_MOUSEMOTION) {
		} else if (event.type == SDL_KEYDOWN) {
			if (key[SDL_SCANCODE_Q]) quit = true; 
			if (key[SDL_SCANCODE_T]) task_running = not task_running;
			if (key[SDL_SCANCODE_S]) { 

				SDL_LockSurface(surface);
				memset((uint32_t*) surface->pixels, 0, sizeof(uint32_t) * pixel_count);
				SDL_UnlockSurface(surface);

			}
			if (key[SDL_SCANCODE_A]) { 
				SDL_LockSurface(surface);
				const nat width = surface->w;
				for (nat r = 0; r < lpixel_size; r++) {
					for (nat c = 0; c < lpixel_size; c++) {
						const nat at = width * (lpixel_size * row + r) + (lpixel_size * column + c);
						((uint32_t*) surface->pixels)[at] = (uint32_t) -1;
					}
				}
				if (column >= column_count) { 
					column = 0; 
					if (row >= row_count) row = 0; 
					else row++;
				} else column++;
				SDL_UnlockSurface(surface);
			} 
		}
		if (task_running) goto input_loop;
	input_done:;
		//const Uint8* key = SDL_GetKeyboardState(0);
		//if (key[SDL_SCANCODE_A]) {
		//	// ... 
		//}

		if (resized) {
			resized = 0;
			SDL_GetWindowSize(window, &window_width, &window_height);
			surface = SDL_GetWindowSurface(window);
			pixel_count = (size_t)surface->w * (size_t)surface->h;

			row = 0; column = 0; lpixel_size = 300;
			row_count = surface->h / lpixel_size - 1;
			column_count = surface->w / lpixel_size - 1;
		}

		SDL_UpdateWindowSurface(window);
		nanosleep((const struct timespec[]){{0, (20000000)}}, NULL);
	}
	SDL_DestroyWindow(window);
	SDL_Quit();
}

























/*




	
		} else {
			if (SDL_WaitEvent(&event)) {
				const Uint8* key = SDL_GetKeyboardState(0);
				if (event.type == SDL_QUIT) { quit = true; }
				else if (event.type == SDL_WINDOWEVENT) {
					if (	event.window.event == SDL_WINDOWEVENT_RESIZED or 
						event.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) resized = true;
				} else if (event.type == SDL_MOUSEMOTION) {
				} else if (event.type == SDL_KEYDOWN) {
					if (key[SDL_SCANCODE_Q]) quit = true; 
					if (key[SDL_SCANCODE_T]) task_running = not task_running;
					if (key[SDL_SCANCODE_A]) { 
						SDL_LockSurface(surface);
						const nat width = surface->w;
						for (nat r = 0; r < lpixel_size; r++) {
							for (nat c = 0; c < lpixel_size; c++) {
								const nat at = width * (lpixel_size * row + r) + (lpixel_size * column + c);
								((uint32_t*) surface->pixels)[at] = (uint32_t) -1;
							}
						}
						if (column >= column_count) { 
							column = 0; 
							if (row >= row_count) row = 0; 
							else row++;
						} else column++;
						SDL_UnlockSurface(surface);
					} 
				}
			}
			
		}		




*/


