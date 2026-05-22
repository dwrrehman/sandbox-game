//
//  shaders.cpp
//  block-game
//
//  Created by Daniel Rehman on 1906086.
//                                                       
//

#include "shaders.hpp"
#include "utilities.hpp"
#include "parameters.hpp"
#include "camera.hpp"
#include "SDL2.framework/Headers/SDL.h"
#include "glm/glm.hpp"
#include "GL/glew.h"

#include <iostream>
#include <fstream>
#include <thread>
#include <stdio.h>
#include <unistd.h>

void Shaders::check_shader_error(GLuint shader, GLuint flag, bool is_program, std::string error_message) {
    GLint success = 0;
    GLchar error[1024];    
    if (is_program) glGetProgramiv(shader, flag, &success);
    else glGetShaderiv(shader, flag, &success);
    if (!success) {
        if (is_program) glGetProgramInfoLog(shader, sizeof(error), NULL, error);
        else glGetShaderInfoLog(shader, sizeof(error), NULL, error);
        std::cerr << error_message << ": `" << error << "`\n";
    }
}

GLuint Shaders::create_shader(const std::string& text, GLenum shader_type) {
    GLuint shader = glCreateShader(shader_type);
    if (!shader) {
        std::cerr << "error: could not create shader!\n";
        throw "could not create shader";
    }
    
    const GLchar* shader_sources[1];
    GLint shader_lengths[1];
    shader_sources[0] = text.c_str();
    shader_lengths[0] = (int) text.length();
    
    glShaderSource(shader, 1, shader_sources, shader_lengths);
    glCompileShader(shader);    
    check_shader_error(shader, GL_COMPILE_STATUS, false, "shader compilation error: ");
    
    return shader;
}

void Shaders::create_shaders() {    
   
    shaders[vertex_shader] = create_shader(find_file(shader_filepath + ".vs"), GL_VERTEX_SHADER);
    shaders[fragment_shader] = create_shader(find_file(shader_filepath + ".fs"), GL_FRAGMENT_SHADER);
    
    for (unsigned i = 0; i < shader_count; i++) glAttachShader(program, shaders[i]);
    
    glBindAttribLocation(program, 0, "position");
    glBindAttribLocation(program, 1, "color");
    
    glLinkProgram(program);
    check_shader_error(program, GL_LINK_STATUS, true, "program linking failure: ");    
    glValidateProgram(program);
    check_shader_error(program, GL_VALIDATE_STATUS, true, "program validation failure: ");

    uniforms[transform_uniform] = glGetUniformLocation(program, "transform");
}

void Shaders::delete_shaders_and_program() { 
    for (unsigned i = 0; i < shader_count; i++) {
        glDetachShader(program, shaders[i]);
        glDeleteShader(shaders[i]);        
    }
    glDeleteProgram(program);
}

static glm::mat4 getmodel(struct transform_data data) {
    glm::mat4 position_matrix = glm::translate(data.position);
    
    glm::mat4 rotation_x_matrix = glm::rotate(data.rotation.x, glm::vec3(1, 0, 0));
    glm::mat4 rotation_y_matrix = glm::rotate(data.rotation.y, glm::vec3(0, 1, 0));
    glm::mat4 rotation_z_matrix = glm::rotate(data.rotation.z, glm::vec3(0, 0, 1));
    
    glm::mat4 scale_matrix = glm::scale(data.scale);
    
    glm::mat4 rotation_matrix = rotation_z_matrix * rotation_y_matrix * rotation_x_matrix;
    
    return position_matrix * rotation_matrix * scale_matrix;
}

Shaders::Shaders() {
    program = glCreateProgram();
    create_shaders();
}

Shaders::~Shaders() {
    delete_shaders_and_program();
}

void Shaders::bind() {
    glUseProgram(program);
}

void Shaders::update(const struct transform_data& transform, camera& camera) {    
    glm::mat4 model = camera.get_view_projection() * getmodel(transform);    
    glUniformMatrix4fv(uniforms[transform_uniform], 1, GL_FALSE, &model[0][0]);
}
