//
//  camera.hpp
//  block-game
//
//  Created by Daniel Rehman on 1906097.
//                                                       
//

#ifndef camera_hpp
#define camera_hpp

#include "parameters.hpp"
#include "init.hpp"
#include "utilities.hpp"

#include "SDL2.framework/Headers/SDL.h"
#include "glm/glm.hpp"
#include "glm/gtx/transform.hpp"
#include "GL/glew.h"

#include <iostream>
#include <fstream>
#include <thread>
#include <stdio.h>
#include <unistd.h>

class camera {
    
public:
    glm::mat4 perspective;
    glm::vec3 forward;
    glm::vec3 upward;
    glm::vec3 position;
    glm::vec2 mouse_position;
    float aspect, fov, z_closest, z_farthest;
    
    camera(const glm::vec3 position, float fov, float z_closest, float z_farthest) {        
        this->position = position;
        this->fov = fov;
        this->z_closest = z_closest;
        this->z_farthest = z_farthest;
        
        forward = glm::vec3(0,0,1);
        upward = glm::vec3(0,1,0);        
        resized_window();
    }

    glm::mat4 get_view_projection() {        
        return perspective * glm::lookAt(position, position + forward, upward);
    }
    
    void resized_window() {
        aspect = (float) window_width / (float) window_height;
        perspective = glm::perspective(fov, aspect, z_closest, z_farthest);
    }
    
    void rotate_camera(glm::vec2 delta) {
        if (glm::length(delta) > 50.0) return;        
        forward = glm::mat3(glm::rotate(-delta.x * sensitivity, upward)) * forward;        
        glm::vec3 torotate_around = glm::cross(forward, upward);
        forward = glm::mat3(glm::rotate(-delta.y * sensitivity, torotate_around)) * forward;                        
    }
};

#endif /* camera_hpp */
