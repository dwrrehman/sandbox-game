//   Client for my multiplayer universe sandbox game.
//        Written by Daniel Warren Riaz Rehman 
//               on 2104305.171454
#include <SDL2/SDL.h>
#include <iso646.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>

const char* window_title = "universe client";
int window_height = 800, window_width = 1200;

static inline void window_changed(SDL_Window* window, SDL_Renderer* renderer) {
	int w = 0, h = 0;
	SDL_GetWindowSize(window, &w, &h);
	SDL_RenderSetLogicalSize(renderer, w, h);
	window_width = w;
	window_height = h;
	printf("width and height: (%d, %d)\n", window_width, window_height);
}

static inline void toggle_fullscreen(SDL_Window* window, SDL_Renderer* renderer) {
	static bool full = false;
	full = !full;
	SDL_SetWindowFullscreen(window, full ? SDL_WINDOW_FULLSCREEN : 0);
	window_changed(window, renderer);
	
}

static inline void display_pixels(int count, int* array, SDL_Renderer* renderer) {
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    	SDL_RenderClear(renderer);
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	for (int i = 0; i < count; i += 2) 
		SDL_RenderDrawPoint(renderer, array[i], array[i + 1]);
    	SDL_RenderPresent(renderer);
}

int main(const int argc, const char** argv) {

	if (SDL_Init(SDL_INIT_EVERYTHING)) exit(printf("SDL_Init failed: %s\n", SDL_GetError()));

	SDL_Window *window = SDL_CreateWindow(window_title, 
			SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
			window_width, window_height, 
			SDL_WINDOW_RESIZABLE);

	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	SDL_ShowCursor(0);
	srand((unsigned)time(0));

	int count = 10000 * 2;
	int* array = malloc(count * sizeof(int));

	bool quit = false;
	SDL_Event event;

	while (not quit) {

		// get array data from server.
		for (int i = 0; i < count; i += 2) {
			array[i + 0] = rand() % window_width;
			array[i + 1] = rand() % window_height;
		}

		display_pixels(count, array, renderer);

		while (SDL_PollEvent(&event)) {
			const Uint8* key = SDL_GetKeyboardState(0);
			if (event.type == SDL_QUIT) quit = true;
			if (event.type == SDL_WINDOWEVENT) {
                		if (event.window.event == SDL_WINDOWEVENT_RESIZED) window_changed(window, renderer);
			}
			if (event.type == SDL_KEYDOWN) { if (key[SDL_SCANCODE_GRAVE]) toggle_fullscreen(window, renderer); }
			if (key[SDL_SCANCODE_ESCAPE]) quit = true;
			if (key[SDL_SCANCODE_Q]) quit = true;
			if (key[SDL_SCANCODE_W]) { SDL_Log("W\n"); }
			if (key[SDL_SCANCODE_S]) { SDL_Log("S\n"); }
			if (key[SDL_SCANCODE_A]) { SDL_Log("A\n"); }
			if (key[SDL_SCANCODE_D]) { SDL_Log("D\n"); }
		}
		usleep(10000);
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	free(array);
}

