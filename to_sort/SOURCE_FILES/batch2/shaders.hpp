//
//  shaders.hpp
//  block-game
//
//  Created by Daniel Rehman on 1906086.
//                                                       
//

#ifndef shaders_hpp
#define shaders_hpp

#include "SDL2.framework/Headers/SDL.h"
#include "glm/glm.hpp"
#include "GL/glew.h"
#include "glm/gtx/transform.hpp"

#include "shaders.hpp"
#include "camera.hpp"
#include "utilities.hpp"
#include "parameters.hpp"
#include "init.hpp"
#include "input.hpp"
#include "mesh.hpp"

#include <iostream>
#include <fstream>
#include <thread>
#include <unistd.h>



struct transform_data {
    glm::vec3 position = glm::vec3();
    glm::vec3 rotation = glm::vec3(); 
    glm::vec3 scale = glm::vec3(1.0, 1.0, 1.0);
};

class Shaders {
    
    enum uniform_types {
        transform_uniform,
        uniform_count
    };
    enum shader_types {
        vertex_shader,
        fragment_shader,
        shader_count
    };

    GLuint program;
    GLuint shaders[shader_count];  
    GLuint uniforms[uniform_count];
    
    void check_shader_error(GLuint shader, GLuint flag, bool is_program, std::string error_message);
    GLuint create_shader(const std::string& text, GLenum shader_type);    
    void create_shaders();
    void delete_shaders_and_program();
    
public:
    Shaders();
    ~Shaders();
    void bind();
    void update(const struct transform_data& transform, camera& camera); 
};


#endif /* shaders_hpp */
