//
//  window.cpp
//  block-game
//
//  Created by Daniel Rehman on 1906086.
//                                                       
//

#include "init.hpp"
#include "parameters.hpp"

#include "SDL2.framework/Headers/SDL.h"
#include "glm/glm.hpp"
#include "GL/glew.h"

#include <iostream>
#include <fstream>
#include <thread>
#include <stdio.h>
#include <unistd.h>


void init_sdl() {
    if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
        std::cerr << "could not initialize sdl2: " << SDL_GetError() << "\n";
        exit(1);
    }
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
    
    SDL_ShowCursor(SDL_DISABLE);
    SDL_SetRelativeMouseMode(SDL_TRUE);    
}

void glew_init() {
    glewExperimental = GL_TRUE;
    GLenum status = glewInit();
    if (status != GLEW_OK) {
        std::cerr <<  "could not initialize glew: " << glewGetString(status) << "\n";
        exit(1);
    }
    glEnable(GL_DEPTH_TEST);
}

SDL_Window* create_window() {
    auto window = SDL_CreateWindow(window_name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
                                   window_width, window_height,
                                   SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | 
                                   SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL);
    if (!window) {
        std::cerr <<  "could not create window: " << SDL_GetError() << "\n";
        exit(1);
    }
    
    SDL_WarpMouseInWindow(window, window_width / 2.0, window_height / 2.0);
    
    return window;
}



