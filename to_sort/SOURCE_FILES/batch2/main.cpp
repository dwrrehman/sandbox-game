//
//  main.cpp
//  block-game
//
//  Created by Daniel Rehman on 1906075.
//                                                       
//


/**
 
--------------------------- GAME PLAY NOTES: -----------------------------------
 
 
    - movement of the camera in XYZ directions uses the 
        minecraft "WASD-LSHIFT-SPACE" keyboard mapping. which should be familar.
    
    - for now, the mouse is used to turn the camera, because thats easiest, and most famiarl.
 
    - press "<ESC>" then "q"   to quit the game without saving.
    - press "<ESC>" then "`"   to enter metagamemode 1: freed mouse, change settings. 
    - press "<ESC>" then "1"   to enter metagamemode 1: the default, filled blocks.
    - press "<ESC>" then "2"   to enter metagamemode 2: wireframe view.
    - press "<ESC>" then "3"   to enter metagamemode 3: ?
 
    - press "<TAB>" then "q"   save and quit the game.
    - press "<TAB>" then "`"   to enter gamemode 0: paused game. 
    - press "<TAB>" then "1"   to enter gamemode 1: the default.
    - press "<TAB>" then "2"   to enter gamemode 2: ?
    - press "<TAB>" then "3"   to enter gamemode 3: ?
        
 
 ------------------------- general notes: -----------------------------
 
    - there is no undoing actions in the game.
 
    - blocks are placed and destroyed using interactions.
  
    - the world is a finite, and discrete, cellular automaton.
 
 
    - 
    
 */

#include "parameters.hpp"
#include "shaders.hpp"
#include "init.hpp"
#include "utilities.hpp"
#include "input.hpp"
#include "mesh.hpp"
#include "camera.hpp"
#include "space.hpp"

#include "SDL2.framework/Headers/SDL.h"
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "GL/glew.h"

#include <iostream>
#include <fstream>
#include <thread>
#include <stdio.h>
#include <unistd.h>


struct vertex_mesh {
    std::vector<vertex> vertices = {};
    std::vector<unsigned int> indicies = {};    
};

///////////////// globals: //////////////////// 

bool quit = false; // make this gamemode 0.

unsigned gamemode = 1; // make this part of the game global object.
unsigned metagamemode = 1;

space S {};
space O {};

static void clear_display() {
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void draw(mesh& mesh, SDL_Window* window, SDL_GLContext context) {    
    clear_display();
    if (metagamemode == 2) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    mesh.draw();
    SDL_GL_SwapWindow(window);
}

static void cleanup(SDL_Window* window, SDL_GLContext context) {        
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

struct vertex_mesh compute_vertex_mesh(space& S) {
    
    std::vector<vertex> vertices = {};
    std::vector<unsigned int> indicies = {};
    auto& s = S.S;
    
    size_t block_count = 0;
    
    for (size_t i = 0; i < size; i++) {
        for (size_t j = 0; j < size; j++) {
            for (size_t k = 0; k < size; k++) {
                
                const size_t value = s[i][j][k];
                
                const float ratio = ((float) value) * 0.9;
                const auto color = glm::vec4(ratio, ratio, ratio, 1.0);
                
                if (value) {
                    vertices.insert(vertices.end(), {
                        {glm::vec3(i,   j,   k), glm::vec4{0.0, 0.0, 1.0, 1.0}},
                        {glm::vec3(i,   j,   k+1), color},
                        {glm::vec3(i,   j+1, k), color},
                        {glm::vec3(i,   j+1, k+1), glm::vec4{1.0, 0.0, 0.0, 1.0}},
                        {glm::vec3(i+1, j,   k), color},
                        {glm::vec3(i+1, j,   k+1), glm::vec4{0.0, 1.0, 0.0, 1.0}},
                        {glm::vec3(i+1, j+1, k), color},
                        {glm::vec3(i+1, j+1, k+1), color},
                    });
                    
                    unsigned v = (unsigned)block_count * 8;
                    indicies.insert(indicies.end(), {
                        v, v+1, v+2,   // bottom
                        v+1, v+2, v+3, 
                        
                        v+4, v+5, v+6, // top
                        v+5, v+6, v+7, 
                        
                        v+0, v+4, v+5,  // top sides
                        v+1, v+5, v+7,  
                        v+0, v+4, v+6,  
                        v+2, v+6, v+7,  
                        
                        v+1, v+3, v+7, // bottom sides
                        v, v+2, v+6,   
                        v, v+1, v+5,   
                        v+2, v+3, v+7, 
                    });
                    block_count++;
                }
            }
        }
    }
    return {vertices, indicies};
}

void render_loop(SDL_Window* window, SDL_GLContext context) {
    auto camera_starting_position = glm::vec3(0,0,0); /// make this also be part of the game object.
    Shaders shaders {};
    struct transform_data transform = {};
    camera camera(camera_starting_position, FOV, 0.01f, 1000.0f); // make these parameters:  z_near, and z_far.
    while (!quit) {
        handle_input(quit, window, transform, camera, gamemode, metagamemode); 
        shaders.bind();
        shaders.update(transform, camera);
        
        auto vm = compute_vertex_mesh(O);
        
        mesh mesh {
            &vm.vertices[0], (unsigned) vm.vertices.size(), 
            &vm.indicies[0], (unsigned) vm.indicies.size()
        };
        draw(mesh, window, context);
        usleep(framerate_us_delay);
    }
}

void compute() {
    while(!quit) {
        usleep(2000000); ///TODO: make this use a global game parameter.
//        evolve(S, O);
//        game_of_life_evolve(S);
        game_of_life_evolve(O);
//        O = S;
//        S.binary_randomize();
//        S.set_unit(0,0,0, 1);
        
    }
}

int main(int argc, const char * argv[]) {
    
    init_sdl();
    SDL_Window* window = create_window();
    SDL_GLContext context = SDL_GL_CreateContext(window);    
    glew_init();
    
    srand((unsigned) time(NULL));
    
    O.binary_randomize();
    
    std::thread compute_thread(compute);
    render_loop(window, context);
    compute_thread.join();
    cleanup(window, context);
}
