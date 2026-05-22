//
//  parameters.hpp
//  block-game
//
//  Created by Daniel Rehman on 1906086.
//                                                       
//

#ifndef parameters_hpp
#define parameters_hpp

#include <string>

extern const std::string shader_filepath;

extern const char* window_name;

extern unsigned window_width;
extern unsigned window_height;

extern const float FOV;
extern const float sensitivity;
extern const float camera_speed;
extern const float framerate_us_delay;


extern const size_t size;

#endif /* parameters_hpp */














/*

std::vector<vertex> vertices = {                 
    {glm::vec3(0,0,0), glm::vec4(0.0, 1.0, 1.0, 1.0)},        
    {glm::vec3(0,1,0), glm::vec4(1.0, 0.0, 1.0, 1.0)},        
    {glm::vec3(1,1,0), glm::vec4(0.5, 1.0, 0.0, 1.0)},        
    {glm::vec3(1,0,0), glm::vec4(0.0, 1.0, 1.0, 1.0)},                
    {glm::vec3(0,0,1), glm::vec4(0.0, 0.5, 1.0, 1.0)},        
    {glm::vec3(0,1,1), glm::vec4(0.5, 1.0, 1.0, 1.0)},        
    {glm::vec3(1,1,1), glm::vec4(0.0, 1.0, 1.0, 1.0)},
    {glm::vec3(1,0,1), glm::vec4(0.0, 1.0, 1.0, 1.0)},
};

std::vector<unsigned int> indicies = {
    0, 1, 2,
    0, 2, 3, 
    
    0, 1, 4,
    1, 4, 5,
    
    0, 3, 7,
    0, 4, 7,
    
    1, 2, 6,
    6, 5, 1,
    
    4, 5, 6,
    4, 6, 7,
    
    2, 3, 7,
    2, 6, 7,
    };
*/
