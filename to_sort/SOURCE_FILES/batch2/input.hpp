//
//  input.hpp
//  block-game
//
//  Created by Daniel Rehman on 1906086.
//                                                       
//

#ifndef input_hpp
#define input_hpp

#include "camera.hpp"
#include "shaders.hpp"

void handle_input(bool& quit, SDL_Window* window, struct transform_data& transform, camera& camera, unsigned& gamemode, unsigned& metagamemode); 

#endif /* input_hpp */
