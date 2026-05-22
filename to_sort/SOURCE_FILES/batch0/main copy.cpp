/**
 this is a server for the game i made for CS447 game design class, called "U^2"
  
    U2 is a multiplayer game, and thus needs a seperate high performance server, such as this. 
 */


#include "connection.hpp"
#include "helpers.hpp"
#include "serializer.hpp"
#include "game_state.hpp"

#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <algorithm>
#include <mutex>


#include <cstdlib>
#include <cstring>
#include <ctime>
#include <math.h>

#include <unordered_map>
#include <map>


#include <unistd.h>
#include <stdnoreturn.h>

#include <sys/socket.h>


std::mutex space_lock;


__attribute__((always_inline)) inline static nat& at(const nat x, const nat y) { return game.space[x + game.size * y]; }
__attribute__((always_inline)) inline static nat& at(const nat x, const nat y, space& space) { return space[x + game.size * y]; }
__attribute__((always_inline)) inline static nat& at(const point p) { return game.space[p.x + game.size * p.y]; }
__attribute__((always_inline)) inline static nat wrap(const int v) { return (v + game.size) % game.size; }
__attribute__((always_inline)) inline static nat max_health() { return game.m - 1; }



// --------------------------- spawn player -----------------------------


static inline bool is_spawnable(const point& r) {
    bool is_spawnable = true;
    for (int dy = -1; dy < 1; dy++)
        for (int dx = -1; dx < 1; dx++)
            if (at(wrap(static_cast<int>(r.x) + dx),
                   wrap(static_cast<int>(r.y) + dy)))
                is_spawnable = false;
    return is_spawnable;
}

static inline point generate_spawn_location() {
    for (nat i = 0; i < game.max_spawn_attempts; i++) {
        const auto random_location = point {rand() % game.size, rand() % game.size};
        if (is_spawnable(random_location)) return random_location;
    }
    abort_if(true, "could not spawn something after " + std::to_string(game.max_spawn_attempts) + " iterations, aborting...\n");
    return {};
}

struct bucket {
    std::vector<point> points = {};
    nat m = 0;
};

inline static bool was_not_inserted_into_existing(std::vector<bucket>& buckets, const point& location, nat value) {
    for (auto& bucket : buckets) {
        if (bucket.m == value) {
            bucket.points.push_back(location);
            return false;
        }
    }
    return true;
}

















// --------------------- debug functions ---------------------------------

static inline void print_space() {
    std::cout << "printing space: \n";
    for (int y = 0; y < game.size; y++) {
        for (int x = 0; x < game.size; x++) {
            std::cout << at(x, y) << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

static inline void print_buckets(std::vector<bucket>& buckets) {
    std::cout << "printing buckets: \n";
    for (auto& bucket : buckets) {
        std::cout << "[" << bucket.m << "] :: {\n";
        for (auto& point : bucket.points) {
            std::cout << "\t(" << point.x << ", " << point.y << ")\n";
        }
        std::cout << "}\n";
    }
    std::cout << "\n";
}

static inline void print_vector(std::vector<nat>& v) {
    std::cout << "[ ";
    for (auto& e : v) {
        std::cout << e << " ";
    }
    std::cout << "]\n";
}

static inline void print_nodes(std::vector<node>& nodes) {
    std::cout << "{ \n";
    for (auto& node : nodes) {
        std::cout << "\t{(" << node.point.x << "," << node.point.y << "):" <<node.f<< ":" << node.g << ":" << node.h << "}\n";
    }
    std::cout << "} \n";
}

static inline void print_points(std::vector<point>& points) {
    std::cout << "{ \n";
    for (auto& point : points) {
        std::cout << "\t{(" << point.x << "," << point.y << ")\n";
    }
    std::cout << "} \n";
}


static inline void print_players(players players) {
    nat i = 0;
    std::cout << "printing players: \n";
    for (auto& player : players) {
        std::cout << "player #"<< i << " = " << player.name << "\n";
        std::cout << "\tgamemode = " << player.gamemode << "\n";
        std::cout << "\tlocation = " << player.location.x << ", " << player.location.y << "\n";
        std::cout << "\tview size = " << player.view.width << ", " << player.view.height << "\n";
        i++;
    }
}

static inline nat convert(enum rogue_state state) {
    if (state == rogue_state::wandering) {
        return 0;
    } else if (state == rogue_state::searching) {
        return 1;
    }
    return -1;
}

static inline void print_rogues(rogues rogues) {
    nat i = 0;
    std::cout << "printing rogues: \n";
    for (auto& rogue : rogues) {
        std::cout << "rogue #"<< i << "\n";
        std::cout << "\tlocation = " << rogue.location.x << ", " << rogue.location.y << "\n";
        std::cout << "\tlifetime = " << rogue.lifetime << "\n";
        std::cout << "\tstate: " << convert(rogue.state) << "\n";
        std::cout << "\ttarget = " << rogue.target.x << ", " << rogue.target.y << "\n";
        std::cout << "\tpath:  {\n";
        for (auto& p : rogue.path) {
            std::cout << "\t\t(" << p.x << "," << p.y << ") \n";
        }
        std::cout << "\t} \n";
        i++;
    }
}

static inline void debug() {
    //    print_space();
    print_rogues(game.rogues);
    print_players(game.players);
}









// ----------------------------------- rendering packet code --------------------------------

static inline void add_players_to_view(const point &location, players &players_in_view, int view_x, int view_y) {
    auto players = game.players;
    for (auto& p : players) {
        if (p.location.x == view_x and p.location.y == view_y) {
            p.location = location;
            players_in_view.push_back(p);
        }
    }
}

static inline void add_rogues_to_view(const point &location, rogues &rogues_in_view, int view_x, int view_y) {
    for (auto& r : game.rogues) {
        if (r.location.x == view_x and r.location.y == view_y) {
            rogues_in_view.push_back({location, {}, r.state, r.target, r.lifetime, r.target_valid});
        }
    }
}

static inline void add_path_to_view(player &player, nat &value, int view_x, int view_y) {
    if (player.gamemode == 2) {
        for (auto& rogue : game.rogues) {
            for (auto& p : rogue.path) {
                if (view_x == p.x and view_y == p.y) {
                    value = 255;
                }
            }
        }
    }
}

// --------------------------- packet sending function ----------------------------

static inline void generate_packet(std::vector<nat>& packet, const nat p) {
    
    auto& player = game.players[p];
    const auto [player_x, player_y] = player.location;
    const int width_radius = player.view.width / 2;
    const int height_radius = player.view.height / 2;
    
    packet.reserve(player.view.width * player.view.height);
    
    std::vector<bucket> buckets = {};
    players players_in_view = {};
    rogues rogues_in_view = {};
    
    for (int view_y = (int)player_y - height_radius, view_j = 0; view_y < (int)player_y + height_radius; view_y++, view_j++) {
        for (int view_x = (int)player_x - width_radius, view_i = 0; view_x < (int)player_x + width_radius; view_x++, view_i++) {
            const auto wvx = wrap(view_x);
            const auto wvy = wrap(view_y);
            nat value = at({wvx, wvy});
            add_path_to_view(player, value, wvx, wvy);
            if (not value) continue;
            const auto location = point {static_cast<nat>(view_i), static_cast<nat>(view_j)};
            add_players_to_view(location, players_in_view, wvx, wvy);
            add_rogues_to_view(location, rogues_in_view, wvx, wvy);
            if (was_not_inserted_into_existing(buckets, location, value)) buckets.push_back({{location}, value});
        }
    }
    
    // send player list:
    packet.push_back(static_cast<nat>(players_in_view.size()));
    for (auto& player : players_in_view) {
        packet.push_back(player.location.x);
        packet.push_back(player.location.y);
        packet.push_back(static_cast<nat>(player.name.size()));
        for (auto& c : player.name) packet.push_back(c);
    }
    
    // send rogues list:
    packet.push_back(static_cast<nat>(rogues_in_view.size()));
    for (auto& rogue : rogues_in_view) {
        packet.push_back(rogue.location.x);
        packet.push_back(rogue.location.y);
    }

    // send display packet:
    packet.push_back(player.gamemode); // alive
    packet.push_back(at(player.location) > 0); // alive
    packet.push_back(game.m);
    packet.push_back(game.players[p].view.width);
    packet.push_back(game.players[p].view.height);
    packet.push_back(static_cast<nat>(buckets.size()));
    for (auto& bucket : buckets) {
        packet.push_back(bucket.m);
        packet.push_back(static_cast<nat>(bucket.points.size()));
        for (auto& point : bucket.points) {
            packet.push_back(point.x);
            packet.push_back(point.y);
        }
    }
}

static inline void send_packet(const int client, const nat p) {
    std::vector<nat> packet = {};    
    generate_packet(packet, p);
    auto data_length = static_cast<nat>(packet.size() * 4);    
    network::send(client, &data_length);
    auto data = reinterpret_cast<unsigned char*>(packet.data());
    network::send(client, data, data_length);
}















// ------------------------- player functions ---------------------------------

static inline point get_location_based_on(int direction, player player) {
    auto desired_location = player.location;
    const auto [x, y] = desired_location;
    
    if (direction == left)       desired_location = {(x + game.size - 1) % game.size, y};
    else if (direction == right) desired_location = {(x + 1) % game.size, y};
    else if (direction == up)    desired_location = {x, (y + game.size - 1) % game.size};
    else if (direction == down)  desired_location = {x, (y + 1) % game.size};
    return desired_location;
}

static inline void move_to(const point& desired_location, point& previous_location) {
    if (not at(desired_location)) {
        space_lock.lock();
        at(desired_location) = at(previous_location);
        at(previous_location) = 0;
        previous_location = desired_location;
        space_lock.unlock();
    }
}

static inline void move_player(const int direction, const nat p) {
    player& player = game.players[p];
    auto desired_location = get_location_based_on(direction, player);
    move_to(desired_location, player.location);
}

static inline std::vector<nat> get_neighborhood(nat x, nat y, space& space) { // Moore neighborhood.
    std::vector<nat> neighbors = {};
    for (int i = -1; i <= 1; i++)
        for (int j = -1; j <= 1; j++)
            if (i or j) neighbors.push_back(at((x + i + game.size) % game.size,
                                               (y + j + game.size) % game.size, space));
    return neighbors;
}


static inline void change_space(const int direction, const int hand, const nat p) {
    
    player& player = game.players[p];        
    const auto desired_location = get_location_based_on(direction, player);
    
    space_lock.lock();
    
    if (hand == 1) at(desired_location)++;
    else if (hand == 0) at(desired_location) = 0;
    at(desired_location) %= game.m;

    space_lock.unlock();
}

static inline void move(const int client, const nat player) {
    unsigned char direction = 0;
    network::receive(client, &direction, 1);
    move_player(direction, player);
}

static inline void change(const int client, const nat player) {
    unsigned char direction = 0, hand = 0;
    network::receive(client, &direction, 1);
    network::receive(client, &hand, 1);
    change_space(direction, hand, player);
    
}

static inline void update_view_size(const int client, const nat player) {
    auto& view = game.players[player].view;
    network::receive(client, &view.width); 
    network::receive(client, &view.height);
}

static inline nat add_new_player(const std::string& name, const rect& view) {
    game.players.push_back({generate_spawn_location(), view, 1, 0, name});
    space_lock.lock();
    at(game.players.back().location) = max_health();
    space_lock.unlock();
    return static_cast<nat>(game.players.size() - 1);
}

static inline nat handshake(const int client) {
    if (not network::server_running) return 0; // error
    
    nat length = 0;
    network::receive(client, &length);
    abort_if(length > 30, "could not receive name of length " + std::to_string(length) + ", greater than 30.\n");
    
    unsigned char name[length + 1];
    network::receive(client, name, length);
    name[length] = '\0';
    
    nat i = 0;
    for (auto& player : game.players) {
        if (player.name == (char*)name) {
            network::send(client, &player.view.width);
            network::send(client, &player.view.height);
            network::send(client, &game.size);
            return i;
        }
        i++;
    }
    rect view {15, 12}; // default view size.
    network::send(client, &view.width);
    network::send(client, &view.height);
    network::send(client, &game.size);
    return add_new_player((char*) name, view);
}

static inline void update_mode(const int client, const nat player) {
    unsigned char mode = 0;
    network::receive(client, &mode, 1);
    game.players[player].gamemode = mode;
}

static inline void add_rules(const int client, const nat p) {
    unsigned char rule = 0;
    
    auto player = game.players[p];
    network::receive(client, &rule, 1);
    if (rule == 0) {
//        player.hx = get_neighborhood(player.location.x, player.location.y, game.space);
        
    } else if (rule == 1) {
//        nat future = get
    }
    
}

void handler(const int client, const char* client_name) {
    auto player = handshake(client);
    
    std::cout << "---> connected to player: " << game.players[player].name << "\n";
    
//    print_players(game.players);
    
    while (network::server_running) {
        unsigned char command = 0;        
        auto disconnected = not network::receive(client, &command, 1);
        if (disconnected) {
            std::cout << "player disconnected.\n";
            break;
        }
        switch (command) {
            case command::display: send_packet(client, player); break;
            case command::move: move(client, player); break;
            case command::change: change(client, player); break;
            case command::view: update_view_size(client, player); break;
            case command::mode: update_mode(client, player); break;
            case command::save: write_game(game.state); break;
            case command::halt: network::halt_server(); break;
            case command::add: add_rules(client, player); break;
            default: break;
        }        
    }
    close(client);
}











// ------------------------ evolving the space -------------------------------

static inline nat count_ones(const std::vector<nat>& neighbors) {
    nat result = 0;
    for (auto& n : neighbors) if (n == 1) result++;
    return result;
}

static inline nat game_of_life(nat me, const std::vector<nat>& neighbors) {
    if (me == 1 and (count_ones(neighbors) < 2 or count_ones(neighbors) > 3)) return 0;
    else if (me == 0 and count_ones(neighbors) == 3) return 1;
    else return me;
}

static inline void evolve_space_according_to_local_rules() {
//    std::cout << "evolving space according to local rules.\n";
    ///TODO:  adds to the target list.
    
    
    for (auto& local : game.local) {
        for (auto rule : local.rules) {
            
        }
    }
    
}

static inline void evolve_space_according_to_ca() {
    space_lock.lock();
    space copy = game.space;
    for (nat y = 0; y < game.size; y++) {
        for (nat x = 0; x < game.size; x++) {
            auto neighbors = get_neighborhood(x, y, copy);
            game.space[x + game.size * y] = game_of_life(at(x, y, copy), neighbors);
        }
    }
    space_lock.unlock();
}


// ------------------------ spawning of rogues -------------------------------

static inline void spawn_rogue() {
//    std::cout << "SPANWED ROGUE!\n";
    auto spawn_location = generate_spawn_location();
    at(spawn_location) = max_health();
    rogue new_rogue {spawn_location};
    new_rogue.lifetime = game.rogue_wandering_lifetime;
    game.rogues.push_back(new_rogue);
}

static inline void try_to_spawn_rogues() {
    nat shouldnt_spawn = rand() % game.rogue_spawn_modulo;
    if (not shouldnt_spawn and game.rogues.size() < game.rogue_max_count) {
        spawn_rogue();
    }
}



// ------------------------ pathfinding -------------------------------

static inline std::vector<node> get_valid_path_neighbors(const node& me, space& space, nat g) {
    std::vector<node> successors = {};
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            node p {
                point {
                    (me.point.x + i + game.size) % game.size,
                    (me.point.y + j + game.size) % game.size
                },
            };
            p.g = g + 1;
            p.parent = {me};
            
            if (((i and not j) or (not i and j)) and not at(p.point)) {
                successors.push_back(p);
            }
        }
    }
    return successors;
}

static inline std::vector<point> get_path_neighbors(const point& me) {
    std::vector<point> successors = {};
    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            
            point p {
                (me.x + i + game.size) % game.size,
                (me.y + j + game.size) % game.size
            };
            
            if (((i and not j) or (not i and j))) {
                successors.push_back(p);
            }
        }
    }
    return successors;
}

static inline nat distance(const point& a, const point& b) { // manthattan distance on a toroid.
    auto dx = std::min(abs((long)a.x - (long)b.x), game.size - abs((long)b.x - (long)a.x));
    auto dy = std::min(abs((long)a.y - (long)b.y), game.size - abs((long)b.y - (long)a.y));
    return (nat) (dx + dy);
}

static inline void compute_f(const point &goal, const node &current, node &neighbor) {
    neighbor.g = current.g + 1;
    neighbor.h = distance(neighbor.point, goal);
    neighbor.f = neighbor.g + neighbor.h;
}

std::vector<point> construct_path(node& current) {
    std::vector<point> path = {};
    while (current.parent.size()) {
        path.push_back(current.point);
        current = current.parent[0];
    }
    std::reverse(path.begin(), path.end());
    return path;
}

static inline std::vector<point> astar(const point& start, const point& goal) {
    
    auto h = distance(start, goal);
    std::vector<node> open = {{start, h, 0, h}};
    std::vector<point> closed = {};
    
    while (open.size()) {
        std::sort(open.begin(), open.end(), [start, goal](const node& a, const node& b) { return a.f > b.f; });
        node current = open.back();
        open.pop_back();
        if (current.g > game.astar_cost_limit) return {};
        for (auto& child : get_valid_path_neighbors({goal}, game.space, 0))
            if (current.point == child.point) return construct_path(current);
        closed.push_back(current.point);
        
        for (auto& neighbor : get_valid_path_neighbors(current, game.space, current.g)) {
            compute_f(goal, current, neighbor);
            if (std::find(closed.begin(), closed.end(), neighbor.point) == closed.end() and
                std::find(open.begin(), open.end(), neighbor) == open.end()) {
                open.push_back(neighbor);
            } else {
                std::vector<point> points = {};
                for (auto& node : open) {
                    if (node.g > neighbor.g) {
                        points.push_back(node.point);
                    }
                }
                if (std::find(points.begin(), points.end(), neighbor.point) != points.end()) {
                    std::vector<point> points = {};
                    std::vector<nat> indicies = {};
                    nat i = 0;
                    for (auto& node : open) {
                        if (node.g > neighbor.g and node.point == neighbor.point) {
                            points.push_back(node.point);
                            indicies.push_back(i);
                        }
                        i++;
                    }
                    open[indicies.front()] = neighbor;
                }
            }
        }
    }

    return {};
}

static inline void find_targets() {

    for (auto& rogue : game.rogues) {
        
        for (auto& target : game.targets) {
            rogue.target = target;
            rogue.state = rogue_state::searching;
            rogue.target_valid = true;
        }
        
        for (auto& player : game.players) {
            if (player.gamemode) {
                rogue.target = player.location;
                rogue.state = rogue_state::searching;
                rogue.target_valid = true;
            }
        }
    }
}

static inline void move_rogues() {

    for (auto& rogue : game.rogues) {
        
        bool still = false;
        auto visible = distance(rogue.location, rogue.target) < game.rogue_radius;
        
        if (rogue.target_valid and rogue.path.size() and visible) move_to(rogue.path.front(), rogue.location);
        else if (rogue.path.empty()) {
            rogue.target_valid = false;
            still = true;
        }
        find_targets();
        if (visible) rogue.path = astar(rogue.location, rogue.target);
        else still = true;
        
        if (still and rogue.lifetime > 0) rogue.lifetime--;
        if (rogue.lifetime == 0) at(rogue.location) = 0;
        
    }
}

static void make_enviornment_affect_players() {
    for (auto& player : game.players) {
        if (count_ones(get_neighborhood(player.location.x, player.location.y, game.space)) > 2 and player.gamemode) {
            at(player.location) = 0;
        }
    }
}

static void make_rogues_affect_players() {
    for (auto& player : game.players) {
        for (auto& rogue : game.rogues) {
            auto neighbors = get_path_neighbors(player.location);
            for (auto& p : neighbors) {
                if (p == rogue.location and at(player.location) > 0) {
                    at(player.location)--;
                    break;
                }
            }
        }
    }
}

void simulate() {
    while (network::server_running) {
        evolve_space_according_to_ca();
        evolve_space_according_to_local_rules();
        try_to_spawn_rogues();
        move_rogues();
        make_enviornment_affect_players();
        make_rogues_affect_players();
        game.players.erase(std::remove_if(game.players.begin(), game.players.end(), [&](const player& player){ return not at(player.location); }), game.players.end());
        game.rogues.erase(std::remove_if(game.rogues.begin(), game.rogues.end(), [&](const rogue& rogue){ return not at(rogue.location); }), game.rogues.end());
        usleep(game.tick_delay);
    }
}

int main(const int argc, const char** argv) {
    get_arguments(argc, argv);
    if (argc > 6 and game.should_read_state) read_game(argv[6]);
    else if (game.random) generate_universe();
    else generate_zero_space();
    game.state = save_destination(argc, argv);
    std::thread thread(simulate);
    network::listen(game.port, handler);
    thread.join();
    write_game(game.state);
}
