
#ifndef game_state_hpp
#define game_state_hpp


#include <stdlib.h>
#include <vector>
#include <string>


namespace command {
    enum command_type {
        display,
        move, 
        change, 
        view,
        mode,
        save,
        halt,
        
        add, // temp ?
    };
}


enum direction {
    self,
    left,
    right, 
    up,
    down
};

using nat = uint32_t;

struct point {
    nat x = 0;
    nat y = 0;
    
    bool operator==(const point& p) const {
        return x == p.x and y == p.y;
    }
};

struct node {
    point point = {0, 0};
    nat f = 0;
    nat g = 0;
    nat h = 0;
    
    std::vector<node> parent = {}; /// treating as a pointer.
    
    bool operator==(const node& p) const {
        return point.x == p.point.x and point.y == p.point.y;
    }
};



struct rect {
    nat width = 0;
    nat height = 0;
};

struct player {
    point location = {};
    rect view = {};
    nat gamemode = 0;
    nat hand = 0;
    std::string name = "";
    std::vector<nat> hx = {};
};

enum class rogue_state {
    searching,
    wandering,
};

struct rogue {
    point location = {};
    std::vector<point> path = {};
    enum rogue_state state = rogue_state::wandering;
    point target = {};
    nat lifetime = 0;
    bool target_valid = false;
};

using players = std::vector<player>;

using rogues = std::vector<rogue>;

struct rule {
    std::vector<nat> h = {};
    nat f = 0;
};

using ruleset = std::vector<rule>;

struct local_rules {
    point location = {};
    ruleset rules = {};
};

using space = std::vector<nat>; 

struct game_state {
    space space = {};
    players players = {};
    rogues rogues = {};
    
    std::vector<local_rules> local = {};
    std::vector<point> targets = {};
    
    // constants of the game:
    static constexpr nat port = 65111;                               /// minecraft:  25565,   traditional:  9090
    static constexpr nat tick_delay = 60000;
    static constexpr nat max_spawn_attempts = 10000;
    
    // rogues:
    static constexpr nat rogue_max_count = 10;
    static constexpr nat rogue_spawn_modulo = 40;
    static constexpr nat rogue_wandering_lifetime = 1000;
    
    static constexpr nat rogue_radius = 30;
    static constexpr nat astar_cost_limit = 30;
    
    /// default parameters:
    
    nat size = 10;
    nat m = 8;
    nat seed = 0;
    nat random = 0;
    nat should_read_state = 0;
    
    std::string state = "";
};

extern game_state game;

#endif /* game_state_hpp */
