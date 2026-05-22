//
//  space.hpp
//  block-game
//
//  Created by Daniel Rehman on 1906086.
//                                                       
//

#ifndef space_hpp
#define space_hpp

#include "utilities.hpp"
#include "parameters.hpp"

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <exception>
#include <fstream>

#include <stdlib.h>
#include <time.h>


class space {
public:
    std::vector<std::vector<std::vector<size_t>>> S = {}; ///TODO: make this simply a single vector of nats, which uses the dimensionality to find elements.
    
    void create_empty() {
        try {                                
            S.resize(size, {size, std::vector<size_t>(size, 0)});
        } catch(const std::bad_alloc& e) {
            std::cout << "error: not enough memory for that M.\n";
        }
    }
    
    void initialize_with(std::string string) {
        std::stringstream stream {string};
        create_empty();
        for (size_t x = 0; x < size; x++) {
            for (size_t y = 0; y < size; y++) {
                for (size_t z = 0; z < size; z++) {
                    size_t v = 0;
                    stream >> v;
                    S[x][y][z] = v;
                }
            }
        }
    }
    
    space() { create_empty(); }
    
    space(std::string filepath) {
        initialize_with(find_file(filepath));        
    }
    
    
    std::string stringify() {
        std::stringstream stream {};
        stream << size << '\n'; 
        for (auto s : S) {
            for (auto t : s) {
                for (auto v : t) {
                    stream << v << ' ';
                }
            }
            stream << '\n';
        }        
        return stream.str();
    }
    
    void serialize(std::string filepath) {
        std::ofstream file {filepath, std::ios_base::trunc | std::ofstream::out};
        file << stringify();
    } 
    
    void print() {
        std::cout << "units: ";
//        std::cout << cubed();
        std::cout << "\n";
        std::cout << stringify() << "\n";
    }
    
    void set_unit(size_t x, size_t y, size_t z, size_t value) {
        S[x][y][z] = value;
    }
    
    void randomize() {        
        for (size_t x = 0; x < size; x++) {
            for (size_t y = 0; y < size; y++) {
                for (size_t z = 0; z < size; z++) {
                    set_unit(x, y, z, rand() % size);
                }
            }
        }
    }
    
    void binary_randomize() {
        for (size_t x = 0; x < size; x++) {
            for (size_t y = 0; y < size; y++) {
                for (size_t z = 0; z < size; z++) {
                    set_unit(x, y, z, !(rand() % 3));
                }
            }
        }
    }
    
    space(const space& s) = default;
};



void evolve(space& S, space& output);
void game_of_life_evolve(space& current);

#endif /* space_hpp */
