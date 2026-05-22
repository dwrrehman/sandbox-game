//
//  helpers.hpp
//  server
//
//  Created by Daniel Rehman on 1909297.
//                                                       
//

#ifndef helpers_hpp
#define helpers_hpp

#include <string>
#include <stdnoreturn.h>

void abort_if(bool condition, std::string message);

noreturn void print_usage();

void get_arguments(int argc, const char **argv);

std::string random_string();


void generate_universe();

void generate_zero_space();



#endif /* helpers_hpp */
