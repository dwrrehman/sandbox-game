








/*


#include <stdlib.h>

#include <SDL2/SDL.h>

#define WINDOW_WIDTH 600

int main(void) {
    SDL_Event event;
    SDL_Renderer *renderer;
    SDL_Window *window;
    int i;

    SDL_Init(SDL_INIT_VIDEO);
    SDL_CreateWindowAndRenderer(WINDOW_WIDTH, WINDOW_WIDTH, 0, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    for (i = 0; i < WINDOW_WIDTH; ++i)
        SDL_RenderDrawPoint(renderer, i, i);
    SDL_RenderPresent(renderer);
    while (1) {
        if (SDL_PollEvent(&event) && event.type == SDL_QUIT)
            break;
    }
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}


*/










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


