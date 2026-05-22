//
//  mesh.hpp
//  block-game
//
//  Created by Daniel Rehman on 1906086.
//                                                       
//

#ifndef mesh_hpp
#define mesh_hpp

#include "parameters.hpp"
#include "shaders.hpp"
#include "init.hpp"
#include "utilities.hpp"
#include "input.hpp"

#include "SDL2.framework/Headers/SDL.h"
#include "glm/glm.hpp"
#include "GL/glew.h"

#include <iostream>
#include <fstream>
#include <thread>
#include <stdio.h>
#include <unistd.h>


struct vertex {
    glm::vec3 position;
    glm::vec4 color;
};

class mesh {
    
    enum buffer_type {
        position,
        color,
        index,
        buffer_count
    };
    
    GLuint vertex_array;
    GLuint vertex_array_buffers[buffer_count];    
    unsigned draw_count;
    
public:
    mesh(vertex* vertices, unsigned vertex_count, unsigned int* indicies, unsigned index_count);
    ~mesh();
    void draw();
};

#endif /* mesh_hpp */
