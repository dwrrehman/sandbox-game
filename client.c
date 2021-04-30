#include <SDL2/SDL.h>

#include <iso646.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>

const int window_height = 600;
const int window_width = 900;

int main() {

	srand((unsigned)time(0));

	if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init failed: %s\n", SDL_GetError());
		exit(1);
	}

	SDL_Window *window = SDL_CreateWindow("Client for my game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, SDL_WINDOW_RESIZABLE);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE);

	bool quit = false;

	while (not quit) {

		

		// const uint8_t* key = SDL_GetKeyboardState(0);
		SDL_Event e;
		while (SDL_PollEvent(&e)) {
			const Uint8* key = SDL_GetKeyboardState(0);
			if (e.type == SDL_QUIT) quit = true;
	
			if (e.type == SDL_KEYDOWN) {
				if (key[SDL_SCANCODE_0]) {
					printf("pressed 0\n");
				}
			}

			if (key[SDL_SCANCODE_ESCAPE]) quit = true;
			if (key[SDL_SCANCODE_Q]) quit = true;
			if (key[SDL_SCANCODE_W]) { SDL_Log("W\n"); }
			if (key[SDL_SCANCODE_S]) { SDL_Log("S\n"); }
			if (key[SDL_SCANCODE_A]) { SDL_Log("A\n"); }
			if (key[SDL_SCANCODE_D]) { SDL_Log("D\n"); }
		}

		for (int i = 0; i < 100; i++) {
			SDL_RenderDrawPoint(renderer, rand() % window_width , rand() % window_height);
		}

		SDL_RenderPresent(renderer);

		usleep(10000);
	}

	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
}






















// static inline void window_changed(camera& camera, SDL_Window *window) {
//     int w = 0, h = 0;
//     SDL_GetWindowSize(window, &w, &h);
//     window_width = w;
//     window_height = h;
// }

// static inline void handle_input(SDL_Window* window, camera& camera, float delta) {

//     const Uint8* key = SDL_GetKeyboardState(nullptr);
//     const float power = (delta * (key[SDL_SCANCODE_LCTRL] ? 0.0016f : camera_acceleration));

//     bool rotation_mode = !!key[SDL_SCANCODE_C];
//     bool tab = !!key[SDL_SCANCODE_TAB];
//     bool escape = !!key[SDL_SCANCODE_ESCAPE];
    
//     if (key[SDL_SCANCODE_SPACE]) camera.velocity += power * camera.upward;
//     if (key[SDL_SCANCODE_LSHIFT]) camera.velocity -= power * camera.upward;
//     if (key[SDL_SCANCODE_W]) camera.velocity += power * camera.forward;
//     if (key[SDL_SCANCODE_W]) camera.velocity += power * camera.forward;
//     if (key[SDL_SCANCODE_S]) camera.velocity -= power * camera.forward;
//     if (key[SDL_SCANCODE_A]) camera.velocity -= power * glm::normalize(glm::cross(camera.forward, camera.upward));
//     if (key[SDL_SCANCODE_D]) camera.velocity += power * glm::normalize(glm::cross(camera.forward, camera.upward));
//     if (key[SDL_SCANCODE_X]) camera.upward = glm::vec3(0,1,0);

//     if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
//         SDL_Log("Mouse Button 1 (left) is pressed.");
//     }

//     if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
//         SDL_Log("Mouse Button 2 (right) is pressed.");
//     }

//     SDL_Event e;
//     while (SDL_PollEvent(&e)) {
// 	const Uint8* key = SDL_GetKeyboardState(nullptr);

//         if (e.window.type == SDL_WINDOWEVENT_RESIZED) window_changed(camera, window);
//         if (e.type == SDL_QUIT) gamemode = 0;

//         }
//         if (e.type == SDL_KEYDOWN) {

//             if (key[SDL_SCANCODE_GRAVE]) {
//                 if (escape) {} else if (tab) gamemode = 0;

                
//             } else if (key[SDL_SCANCODE_3]) {
//                 if (escape) debugmode = !debugmode; else if (tab) gamemode = 3;

//             } else if (key[SDL_SCANCODE_4]) {
//                 if (escape) {} else if (tab) gamemode = 4;
           
// 	    } else if (key[SDL_SCANCODE_5]) {
//                 if (escape) rendermode = GL_FILL; else if (tab) gamemode = 5;

//             } else if (key[SDL_SCANCODE_6]) {
//                 if (escape) rendermode = GL_LINES; else if (tab) gamemode = 6;
//             }
//         }
//     }
// }




