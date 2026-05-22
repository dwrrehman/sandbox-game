//
//  mesh.cpp
//  block-game
//
//  Created by Daniel Rehman on 1906086.
//                                                       
//

#include "mesh.hpp"
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
#include <vector>

mesh::mesh(vertex* vertices, unsigned vertex_count, unsigned int* indicies, unsigned int index_count) {
    
    draw_count = index_count;
    
    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    glGenBuffers(buffer_count, vertex_array_buffers);
        
    std::vector<glm::vec3> positions;
    std::vector<glm::vec4> colors;
    
    positions.reserve(vertex_count);
    colors.reserve(vertex_count);
    
    for (unsigned int i = 0; i < vertex_count; i++) {
        positions.push_back(vertices[i].position);
        colors.push_back(vertices[i].color);        
    }
        
    glBindBuffer(GL_ARRAY_BUFFER, vertex_array_buffers[buffer_type::position]);    
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof positions[0], &positions[0], GL_STATIC_DRAW); // change the draw hint.    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);    
        
    glBindBuffer(GL_ARRAY_BUFFER, vertex_array_buffers[buffer_type::color]);    
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof colors[0], &colors[0], GL_STATIC_DRAW); // change the draw hint.
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_array_buffers[buffer_type::index]);    
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_count * sizeof *indicies, indicies, GL_STATIC_DRAW); // change the draw hint.
    
    glBindVertexArray(0);
}

mesh::~mesh() {    
    glDeleteVertexArrays(1, &vertex_array);
}

void mesh::draw() {    
    glBindVertexArray(vertex_array);
    glDrawElements(GL_TRIANGLES, draw_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

