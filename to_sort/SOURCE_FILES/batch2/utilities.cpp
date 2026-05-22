//
//  utilities.cpp
//  block-game
//
//  Created by Daniel Rehman on 1906086.
//

#include "utilities.hpp"

#include <fstream>

size_t cubed(size_t n) {
    return n * n * n;
}

std::string find_file(const std::string& filepath) {
    std::ifstream file {filepath};
    if (file.good()) {
        std::string text {std::istreambuf_iterator<char>(file), {}};
        return text;
    } else throw "could not find file!";    
}
