//
//  serializer.cpp
//  server
//
//  Created by Daniel Rehman on 1910023.
//                                                       
//

#include "serializer.hpp"

#include "helpers.hpp"
#include "game_state.hpp"

#include <fstream>
#include <iostream>
#include <unistd.h>

#include <iomanip>


//#include <filesystem>


//#include 

/// reading:

static void read_player(player& player, std::ifstream& file) {
    file >> player.name;
    file >> player.gamemode;
    
    file >> player.location.x;
    file >> player.location.y;
    
    file >> player.view.width;
    file >> player.view.height;
}

void read_game(const std::string& path) {
    std::cout << "reading in game.state from: " << path << "\n";
    std::ifstream file {path};
    
    file >> game.m;
    file >> game.size;
    file >> game.seed;
    srand(game.seed);
    
    game.space.resize(game.size * game.size);
    for (auto& e : game.space) file >> e;
    
    nat player_count = 0;
    file >> player_count;
    game.players.resize(player_count);
    for (auto& player : game.players) read_player(player, file);
}

/// writing:

static void write_player(player player, std::ofstream& file) {
    file << player.name << " " << player.gamemode << "\n";
    file << player.location.x << " " << player.location.y << "\n";
    file << player.view.width << " " << player.view.height << "\n";
    file << "\n";
}

static void write_space(std::ofstream& file) {
    for (nat j = 0; j < game.size; j++) {
        for (nat i = 0; i < game.size; i++) {
            file << std::setw(4) << game.space[i + game.size * j] << " ";
        }
        file << "\n";
    }
    file << "\n";
}

void write_game(const std::string& path) {
    std::cout << "saving game state to: " << path << "\n";
    std::ofstream file {path, std::ios_base::trunc};    
    file << game.m << " " << game.size << " " << game.seed << "\n";
    write_space(file);
    
    file << game.players.size() << "\n";
    for (auto player : game.players) write_player(player, file);
    file << std::endl;
}

std::string save_destination(const int argc, const char** argv) {
    
    if (argc > 6) return argv[6];
    else return "game_" + random_string() + ".state";
    
    // std::string cwd = std::filesystem::current_path();
    //TODO: when MacOS has std::filesystem::current_path(), make this use that.    
}



/// TODO: allow for the user to specofy manditoruily, both a read path, and a  write path.


    /// all the time.



