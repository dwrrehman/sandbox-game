//
//  serializer.hpp
//  server
//
//  Created by Daniel Rehman on 1910023.
//                                                       
//

#ifndef serializer_hpp
#define serializer_hpp

#include <string>

void write_game(const std::string& path); 
void read_game(const std::string& path);

std::string save_destination(const int argc, const char** argv);

#endif /* serializer_hpp */
