//
//  helpers.cpp
//  server
//
//  Created by Daniel Rehman on 1909297.
//                                                       
//

#include "helpers.hpp"

#include "game_state.hpp"

#include <iostream>
#include <sstream>

#include <cmath>

#include <stdnoreturn.h>

bool is_little_endian() { // unused.
    const short word = 0x0001;
    char *b = (char *)&word;
    return (b[0] ? true : false);
}


void abort_if(bool condition, std::string message) {
    if (condition) {
        std::cout << message; 
        abort();
    }
}

noreturn void print_usage() {
    printf("usage: \n\t./server M size seed [game.state]\n\n\t"
           "<M: nat> \n\t"
           "<size: nat> \n\t"
           "<seed: nat (0 for random)> \n\t"
           "[game.state: file] \n\t"
           "\n");
    exit(1);
}


std::string random_string() {
    std::stringstream stream;
    stream << std::hex << rand();
    return std::string(stream.str());
}


// fill methods:

void generate_universe() {
    
    if (not game.seed) game.seed = static_cast<unsigned>(time(nullptr));
    srand(game.seed);
    
    game.space.resize(game.size * game.size);
    
    for (auto& e : game.space) {
//        auto G = 1;
//        for (int i = game.p; i--;) G *= rand() % game.k;
        e = (rand() % 2); // assuming binary generation.
    }
}

void generate_zero_space() {
    
    if (not game.seed) game.seed = static_cast<unsigned>(time(nullptr));
    srand(game.seed);
    
    game.space.resize(game.size * game.size);
    for (auto& e : game.space) e = 0;
}



void get_arguments(int argc, const char** argv) {
    if (argc <= 4) print_usage();
    else {
        game.m = atoi(argv[1]);
        game.size = atoi(argv[2]);                
        game.seed = atoi(argv[3]);
        game.random = atoi(argv[4]);
        game.should_read_state = atoi(argv[5]);
        abort_if(game.size <= 0 or game.m <= 0, "error: m and size must be positive natural numbers.\n");        
    }
}
