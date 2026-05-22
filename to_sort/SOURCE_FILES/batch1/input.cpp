//
//  input.cpp
//  block-game
//
//  Created by Daniel Rehman on 1906086.
//                                                       
//

#include "input.hpp"
#include "camera.hpp"
#include "shaders.hpp"
#include "parameters.hpp"
#include "utilities.hpp"

#include "SDL2.framework/Headers/SDL.h"

#include <iostream>

bool escape = false;
bool tab = false;
bool move_camera = true;

static void window_changed(camera& camera, SDL_Window *window) {
    int w = 0, h = 0;
    SDL_GetWindowSize(window, &w, &h);
    window_width = w;
    window_height = h;
    camera.resized_window();
}

static void enable_mouse_rotation() {
    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetRelativeMouseMode(SDL_TRUE);                    
    move_camera = true;
}

static void disable_mouse_rotation() {
    SDL_ShowCursor(SDL_ENABLE);
    SDL_SetRelativeMouseMode(SDL_FALSE);
    move_camera = false;
}



void handle_input(bool& quit, SDL_Window* window, struct transform_data& transform, camera& camera, unsigned& gamemode, unsigned& metagamemode) {
    
    const Uint8* key = SDL_GetKeyboardState(NULL);
    
    ////////////// special keys: ////////////////////
    
    if (key[SDL_SCANCODE_RETURN]) {
        std::cout << "state: PS: RETURN...\n";
    }    
    if (key[SDL_SCANCODE_RSHIFT]) {                
        std::cout << "state: PS: RSHIFT down\n";                
    }
    if (key[SDL_SCANCODE_CAPSLOCK]) {
        std::cout << "state: PS: caps lock.\n";        
    }
        
    ///////////////////// WASD-LSH-SPC MOVEMENT ///////////////////      [finished]
    
    if (key[SDL_SCANCODE_SPACE]) camera.position += camera_speed * camera.upward;
    if (key[SDL_SCANCODE_LSHIFT]) camera.position -= camera_speed * camera.upward;
    if (key[SDL_SCANCODE_W]) camera.position += camera_speed * camera.forward;
    if (key[SDL_SCANCODE_S]) camera.position -= camera_speed * camera.forward;
    if (key[SDL_SCANCODE_A]) camera.position -= glm::normalize(glm::cross(camera.forward, camera.upward)) * camera_speed;    
    if (key[SDL_SCANCODE_D]) camera.position += glm::normalize(glm::cross(camera.forward, camera.upward)) * camera_speed;    
            
    ///////////////////// OLK; KEYPAD: ///////////////////     [currently unused]
    
    if (key[SDL_SCANCODE_O]) {
        std::cout << "state: PS: ^ O\n";                
    }
    if (key[SDL_SCANCODE_L]) {
        std::cout << "state: PS: v L\n";
        
    }
    if (key[SDL_SCANCODE_K]) {
        std::cout << "state: PS: <- K\n";
        
    }
    if (key[SDL_SCANCODE_SEMICOLON]) {
        std::cout << "state: PS: -> ;\n";        
    } 

    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        
        if (e.window.type == SDL_WINDOWEVENT_RESIZED) {
            window_changed(camera, window);
        }
        
        const Uint8* key = SDL_GetKeyboardState(NULL);
        
        if (e.type == SDL_QUIT) { // closed the window.
            std::cout << "they quitted!\n";
            quit = true;             
        } 
        
        if (e.type == SDL_MOUSEMOTION) {            
            if (move_camera) camera.rotate_camera({e.motion.xrel, e.motion.yrel});
        }
        
        if (e.type == SDL_KEYDOWN) {  
            
            if (key[SDL_SCANCODE_Q]) {
                std::cout << "Q\n";
                if (escape) {
                    quit = true;
                    escape = false;
                } else if (tab) {
                    // save the game here.
                    quit = true;
                    tab = false;
                } 
            } else if (key[SDL_SCANCODE_GRAVE]) {                                
                if (escape) {
                    disable_mouse_rotation();
                    escape = false;
                } else if (tab) {                    
                    gamemode = 0;
                    sleep(5);
                    tab = false;
                }           
            } else if (key[SDL_SCANCODE_1]) {                                
                if (escape) {
                    enable_mouse_rotation();
                    metagamemode = 1;
                    escape = false;
                } else if (tab) {                    
                    gamemode = 1;                    
                    tab = false;
                }
            } else if (key[SDL_SCANCODE_2]) {                                
                if (escape) {
                    enable_mouse_rotation();
                    metagamemode = 2;
                    escape = false;
                } else if (tab) {                    
                    gamemode = 2;                    
                    tab = false;
                }
            } else if (key[SDL_SCANCODE_3]) {                                
                if (escape) {
                    enable_mouse_rotation();
                    metagamemode = 3;
                    escape = false;
                } else if (tab) {                    
                    gamemode = 3;                    
                    tab = false;
                }
            }
            
            escape = false;
            tab = false;
            
            ///////////////// weird keys: //////////////////
            
            if (key[SDL_SCANCODE_RETURN]) {
                SDL_SetWindowFullscreen(window, 1);
                
                std::cout << "RETURN...\n";
            }
            
            if (key[SDL_SCANCODE_RSHIFT]) {                
                std::cout << "RSHIFT down\n"; 
                
            }
            if (key[SDL_SCANCODE_CAPSLOCK]) {
                std::cout << "caps lock.\n";                
            }           
            
                
            /////////////////// action keys: /////////////////
            
            if (key[SDL_SCANCODE_F]) {
                std::cout << "kd: F\n";                
            }
            
            if (key[SDL_SCANCODE_Q]) {
                std::cout << "kd: Q\n";
            }
            
            if (key[SDL_SCANCODE_E]) {
                std::cout << "kd: E\n";
                
            }
            if (key[SDL_SCANCODE_R]) {
                std::cout << "kd: R\n";                
            }
            
            if (key[SDL_SCANCODE_X]) {
                std::cout << "kd: X\n";                
            }
                        
            ////////////// modifiers ////////////////            
            if (key[SDL_SCANCODE_TAB]) tab = true;
            if (key[SDL_SCANCODE_ESCAPE]) escape = true;            
        }        
    }    
}
