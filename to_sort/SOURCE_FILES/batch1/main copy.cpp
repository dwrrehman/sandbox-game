// the server side code for the
// game, "u3", which im making.

#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <pwd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <thread>
#include <vector>
#include <functional>
#include <algorithm>
#include <mutex>

typedef uint8_t byte;
typedef size_t nat;
typedef ssize_t integer;

namespace command {
    enum type { display, move, change, view, mode, halt, };
}

enum direction { self, forward, backward, right, left, up, down, };

enum gamemode { survival, spectator, creative, };

struct point {
    nat x;
    nat y;
    nat z;
};

constexpr nat P[] = { 1, 1, 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, };
constexpr nat port = 65111;
constexpr nat tick_us_delay = 1000000;
constexpr nat default_side_length = 20;
constexpr nat max_spawn_attempts = 10000;
constexpr nat default_game_mode = survival;
constexpr nat default_render_distance = 7;
constexpr byte player_initial_health = 6;
constexpr byte rogue_max_health = 10;

struct player {
    point location = {};
    nat render_distance = default_render_distance;
    nat gamemode = default_game_mode;
    std::string name = "";
    std::string UID = "";
};

struct bucket {
    std::vector<point> points = {};
    nat m = 0;
};

struct game_state {
    std::vector<player> players = {};
    nat s = default_side_length;
    nat size = 0;
    unsigned int rng_seed = 0;
    std::string game_state_file = "";
};

std::mutex space_lock;
byte* space = nullptr;
byte* internal = nullptr;
game_state game = {};

byte at(nat x, nat y, nat z) { return space[z + game.s * y + game.s * game.s * x]; }
void at_is(nat x, nat y, nat z, nat n) { space[z + game.s * y + game.s * game.s * x] = n; }
byte at(nat x, nat y, nat z, byte* space) { return space[z + game.s * y + game.s * game.s * x]; }
void at_is(nat x, nat y, nat z, byte* space, nat n) { space[z + game.s * y + game.s * game.s * x] = n; }
byte at(point p) { return space[p.z + game.s * p.y + game.s * game.s * p.x]; }
void at_is(point p, nat n) { space[p.z + game.s * p.y + game.s * game.s * p.x] = n; }
byte wrap(integer v) { return (v + game.s) % game.s; }

void abort_if(bool condition, const char* message) {
    if (condition) {
        printf("%s\n", message);
        abort();
    }
}
//
//std::string random_string() {
//    std::stringstream stream;
//    stream << std::hex << rand();
//    return std::string(stream.str());
//}
//
//static void read_player(player& player, std::ifstream& file) {
//    file >> player.name;
//    file >> player.gamemode;
//    file >> player.location.x;
//    file >> player.location.y;
//    file >> player.location.z;
//    file >> player.render_distance;
//}

void seed_rng() {
    if (not game.rng_seed) {
        game.rng_seed = static_cast<unsigned>(time(0));
        printf("using random seed = %u\n", game.rng_seed);
    }
    srand(game.rng_seed);
}

void write_game() {
//    std::cout << "saving world to: " << game.game_state_file << "\n";
//    std::ofstream file {game.game_state_file, std::ios_base::trunc};
//    file << game.s << " " << game.rng_seed << "\n";
//
//    for (nat x = 0; x < game.s; x++) {
//        for (nat y = 0; y < game.s; y++) {
//            for (nat z = 0; z < game.s; z++) {
//                file << std::setw(4) << at(x, y, z) << " ";
//            }
//            file << "\n";
//        }
//        file << "\n";
//    }
//    file << "\n";
//
//    file << game.players.size() << "\n";
//    for (auto player : game.players) {
//        file << player.name << " " << player.gamemode << "\n";
//        file << player.location.x << " " << player.location.y << " " << player.location.z << "\n";
//        file << player.render_distance << "\n";
//        file << "\n";
//    }
//    file << std::endl;
    abort(); // unimplemnted;
}
void signal_write_game(int unused) { write_game(); exit(0); }

void check(bool condition, const char* message, bool should_exit) {
    if (not condition) {
        perror(message);
        if (should_exit) exit(1);
    }
}

void allow_reuse_port(int client) {
    int one = 1;
    setsockopt(client, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(int));
#ifdef SO_REUSEPORT
    setsockopt(client, SOL_SOCKET, SO_REUSEPORT, &one, sizeof(int));
#endif
}

namespace network {

    bool server_running = true;
    
    int server = 0;
    
    void halt_server() {
        printf("halting server!\n");
        server_running = false;
        shutdown(server, SHUT_RDWR);
        close(server);
    }

    void listen(unsigned int port, std::function<void(int, const char*)> handler) {
        
        server = socket(AF_INET, SOCK_STREAM, 0);
        check(server, "socket", true);
        allow_reuse_port(server);
        
        sockaddr_in server_address = {0}, client_address = {0};
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(port);
        server_address.sin_addr.s_addr = htonl(INADDR_ANY);
        socklen_t client_length = sizeof client_address;
        
        auto bind_result = bind(server, (struct sockaddr*) &server_address, sizeof server_address);
        check(bind_result >= 0, "bind", true);
        ::listen(server, 4);

        while (server_running) {
            usleep(100000);
            printf("listening...\n");
            auto connection = accept(server, (struct sockaddr*) &client_address, &client_length);
            check(connection >= 0, "accept", false);
            
            const char* client_name = gethostbyaddr(&client_address.sin_addr.s_addr, client_length, AF_INET)->h_name;
            printf("connected to %s\n", client_name);
            std::thread handler_thread(handler, connection, client_name);
            handler_thread.detach();
        }
    }

    bool send(int client, const unsigned char* data, size_t length) {
        auto n = ::send(client, data, length, 0);
        if (n < 0) {
            perror("network::send(bytes)");
            abort();
            
        } else if (!n) return false; else return true;
    }

    bool send(int client, const nat* data) {
        auto n = ::send(client, reinterpret_cast<const unsigned char*>(data), 4, 0);
        if (n < 0) {
            perror("network::send(int)");
            abort();
            
        } else if (!n) return false; else return true;
    }


    bool receive(int client, unsigned char* data, size_t length) {
        auto n = ::recv(client, data, length, 0);
        if (n < 0) {
            perror("network::receive(bytes)");
            abort();
            
        } else if (!n) return false; else return true;
    }

    bool receive(int client, nat* data) {
        auto n = ::recv(client, reinterpret_cast<unsigned char*>(data), 4, 0);
        if (n < 0) {
            perror("network::receive(int)");
            abort();
            
        } else if (!n) return false; else return true;
    }
}

void generate_universe(const int argc, const char** argv) {
    if (argc <= 1) {
        seed_rng();
//        game.game_state_file = "game_" + random_string() + ".state";
        game.size = game.s * game.s * game.s;
        
        printf(":: generating a default universe...\n");
        space = (byte*) malloc(game.size);
        for (nat i = 0; i < game.size; i++)
            space[i] = !(rand() % 3);
      
    } else if (std::string(argv[1]) == "empty" and argc == 5) {
        
        game.s = atoi(argv[3]);
        game.size = game.s * game.s * game.s;
        game.rng_seed = atoi(argv[4]);
        seed_rng();
        game.game_state_file = argv[2];
        
        printf(":: generating the void...\n");
        space = (byte*) malloc(game.size);
        for (nat i = 0; i < game.size; i++) space[i] = 0;
        
    } else if (std::string(argv[1]) == "random" and argc == 5) {
        
        game.s = atoi(argv[3]);
        game.size = game.s * game.s * game.s;
        game.rng_seed = atoi(argv[4]);
        seed_rng();
        game.game_state_file = argv[2];
        
        printf(":: generating a universe...\n");
        space = (byte*) malloc(game.size);
        for (nat i = 0; i < game.size; i++) space[i] = !(rand() % 3);
                        
    } else if (std::string(argv[1]) == "from" and argc == 3) {
//        std::ifstream file {argv[2]};
//        if (not file.good()) {
//            perror("tried to open game.state");
//            abort();
//        }
//
//        file >> game.s;
//        game.size = game.s * game.s * game.s;
//        file >> game.rng_seed;
//        seed_rng();
//        game.game_state_file = argv[2];
//
//        printf(":: reading universe from: %s\n", argv[2]);
//        space = (cell*) malloc(game.size);
//        for (nat i = 0; i < game.size; i++) file >> space[i];
//
//        nat player_count = 0;
//        file >> player_count;
//        printf(":: initialized %lu players.\n", player_count);
//        game.players.resize(player_count);
//        for (auto& player : game.players)
//            read_player(player, file);
    } else {
        printf("error: bad usage.\n");
        printf("usage: \n"
               "  ./u \n"
               "  ./u empty <outfile> <s> <seed> \n"
               "  ./u random <outfile> <s> <seed> \n"
               "  ./u from <inout file> \n"
               "\n"
               );
        abort();
    }
}

bool is_spawnable(nat x, nat y, nat z) {
    bool is_spawnable = true;
    for (integer dy = -2; dy < 2; dy++)
        for (integer dx = -2; dx < 2; dx++)
            for (integer dz = -2; dz < 2; dz++)
                if (at(wrap(static_cast<integer>(x) + dx),
                       wrap(static_cast<integer>(y) + dy),
                       wrap(static_cast<integer>(z) + dz)))
                    is_spawnable = false;
    return is_spawnable;
}

point generate_spawn_location() {
    for (nat i = 0; i < max_spawn_attempts; i++) {
        nat x = rand() % game.s, y = rand() % game.s, z = rand() % game.s;
        if (is_spawnable(x, y, z)) return {x, y, z};
    }
    
//    std::cout << "could not spawn something after " + std::to_string(max_spawn_attempts) + " iterations, aborting...\n";
    abort();
}

bool was_not_inserted_into_existing(std::vector<bucket>& buckets, const point& location, nat value) {
    for (auto& bucket : buckets) {
        if (bucket.m == value) {
            bucket.points.push_back(location);
            return false;
        }
    }
    return true;
}

//void print_space() {
//    std::cout << "printing space: \n";
//    for (nat y = 0; y < game.s; y++) {
//        for (nat x = 0; x < game.s; x++) {
//            for (nat z = 0; z < game.s; z++) {
//                std::cout << (nat) at(x, y, z) << " ";
//            }
//            std::cout << "\n";
//        }
//        std::cout << "\n";
//    }
//    std::cout << "\n";
//}

//void print_buckets(std::vector<bucket>& buckets) {
//    std::cout << "printing buckets: \n";
//    for (auto& bucket : buckets) {
//        std::cout << "[" << bucket.m << "] :: {\n";
//        for (auto& point : bucket.points) {
//            std::cout << "\t(" << point.x << ", " << point.y << ")\n";
//        }
//        std::cout << "}\n";
//    }
//    std::cout << "\n";
//}

//void print_players(std::vector<player> players) {
//    nat i = 0;
//    std::cout << "printing players: \n";
//    for (auto& player : players) {
//        std::cout << "player #"<< i << " = " << player.name << "\n";
//        std::cout << "\tgamemode = " << player.gamemode << "\n";
//        std::cout << "\tlocation = " << player.location.x << ", " << player.location.y << "\n";
//        std::cout << "\tview render_distance = " << player.render_distance << "\n";
//        i++;
//    }
//}

//void debug() {
//    print_space();
//    print_players(game.players);
//}

void add_players_to_view(const point &location, std::vector<player>& players_in_view,
                         integer view_x, integer view_y, integer view_z) {
    
    auto players = game.players;
    for (auto& p : players) {
        
        if (p.location.x == view_x and
            p.location.y == view_y and
            p.location.z == view_z) {
            
            p.location = location;
            players_in_view.push_back(p);
        }
    }
}

void generate_packet(std::vector<nat>& packet, const nat p) {
    
    auto& player = game.players[p];
    const auto [player_x, player_y, player_z] = player.location;
    const integer rd = (integer) player.render_distance;
    
    packet.reserve(rd * rd * rd);
    
    std::vector<bucket> buckets = {};
    std::vector<struct player> players_in_view = {};
    
    for (integer view_x = (integer)player_x - rd, view_i = 0; view_x < (integer)player_x + rd; view_x++, view_i++) {
        for (integer view_y = (integer)player_y - rd, view_j = 0; view_y < (integer)player_y + rd; view_y++, view_j++) {
            for (integer view_z = (integer)player_z - rd, view_k = 0; view_z < (integer)player_z + rd; view_z++, view_k++) {
                
                const auto wvx = wrap(view_x);
                const auto wvy = wrap(view_y);
                const auto wvz = wrap(view_z);
                
                nat value = at(wvx, wvy, wvz);
                if (not value) continue;
                
                const auto location = point {
                    static_cast<nat>(view_i),
                    static_cast<nat>(view_j),
                    static_cast<nat>(view_k)
                };
                
                add_players_to_view(location, players_in_view, wvx, wvy, wvz);
                if (was_not_inserted_into_existing(buckets, location, value))
                    buckets.push_back({{location}, value});
            }
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

    // send display packet:
    packet.push_back(player.gamemode);
    packet.push_back(at(player.location) > 0);
    packet.push_back(game.players[p].render_distance);
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

void send_display_packet(const int client, const nat p) {
    std::vector<nat> packet = {};    
    generate_packet(packet, p);
    auto data_length = static_cast<nat>(packet.size() * 4);    
    network::send(client, &data_length);
    auto data = reinterpret_cast<unsigned char*>(packet.data());
    network::send(client, data, data_length);
}

point get_location_based_on(nat direction, player player) {
    auto desired_location = player.location;
    const auto [x, y, z] = desired_location;
    
    auto xn = wrap(x - 1), xp = wrap(x + 1),
         yn = wrap(y - 1), yp = wrap(y + 1),
         zn = wrap(z - 1), zp = wrap(z + 1);
    
    if (direction == forward)       return {x, y, zp};
    else if (direction == backward) return {x, y, zn};
    else if (direction == right)    return {xp, y, z};
    else if (direction == left)     return {xn, y, z};
    else if (direction == up)       return {x, yp, z};
    else if (direction == down)     return {x, yn, z};
    else return desired_location;
}

void move_to(const point& to, point& from) {
    if (not at(to)) {
        space_lock.lock();
        at_is(to, at(from));
        at_is(from, 0);
        space_lock.unlock();
        from = to;
    }
}

void move_player(const integer direction, const nat p) {
    player& player = game.players[p];
    auto desired_location = get_location_based_on(direction, player);
    move_to(desired_location, player.location);
}

void get_moore_neighborhood(nat* h, const nat x, const nat y, const nat z) {
    nat n = 0;
    for (integer i = -1; i <= 1; i++)
        for (integer j = -1; j <= 1; j++)
            for (integer k = -1; k <= 1; k++)
                if (i or j or k) h[n++] = at((x + i + game.s) % game.s, (y + j + game.s) % game.s, (z + k + game.s) % game.s);
}

/// DELETE ME
void change_space(integer direction, nat p) {
    player& player = game.players[p];        
    const auto desired_location = get_location_based_on(direction, player);
    const byte c = at(desired_location);
    space_lock.lock();
    if (c) at_is(desired_location, c - 1);
    space_lock.unlock();
}

void move(int client, nat player) {
    unsigned char direction = 0;
    network::receive(client, &direction, 1);
    move_player(direction, player);
}

///DELETE ME:
void change(int client, nat player) {
    unsigned char direction = 0;
    network::receive(client, &direction, 1);
    change_space(direction, player);
}

nat add_new_player(const std::string& name) {
    game.players.push_back(player {generate_spawn_location(), default_render_distance, survival, name});
    space_lock.lock();
    at_is(game.players.back().location, player_initial_health);
    space_lock.unlock();
    return game.players.size() - 1;
}

integer handshake(const int client) {
    if (not network::server_running) return -1; // error
    
    nat length = 0;
    network::receive(client, &length);
    if (length > 50) return -1; // error
    
    unsigned char name[length + 1];
    network::receive(client, name, length);
    name[length] = '\0';
    
    nat i = 0;
    for (auto& player : game.players) {
        if (player.name == (char*)name) {
            network::send(client, &player.render_distance);
            return i;
        }
        i++;
    }
    nat d = default_render_distance;
    network::send(client, &d);
    return add_new_player((char*) name);
}

void update_mode(const int client, const nat player) {
    unsigned char mode = game.players[player].gamemode;
    network::receive(client, &mode, 1);
    game.players[player].gamemode = mode;
}

void handler(const int client, const char* client_name) {
    
    
    unsigned char buffer[1000] = {0};
    
    
    
    network::receive(client, buffer, sizeof buffer);
    
    network::send(int client, const unsigned char *data, size_t length);
    
    
    
    exit(0);
    
    integer player = handshake(client);
    if (player < 0) {
        printf("error: handshake went wrong, closing client.\n");
        close(client);
        return;
    }
    
//    std::cout << "connected to player: " << game.players[player].name << "\n";
    
//    print_players(game.players);
    
    while (network::server_running) {
        unsigned char command = 0;        
        auto disconnected = not network::receive(client, &command, 1);
        if (disconnected) {
//            std::cout << "player " << game.players[player].name << " disconnected.\n";
            break;
        }
        switch (command) {
            case command::display: send_display_packet(client, player); break;
            case command::move: move(client, player); break;
            case command::change: change(client, player); break;
            case command::mode: update_mode(client, player); break;
            case command::halt: network::halt_server(); break;
            default: break;
        }        
    }
    close(client);
}

nat count(nat k, byte h[26]) {
    nat r = 0;
    for (nat i = 0; i < 26; i++)
        if (h[i] == k) r++;
    return r;
}

bool is_in_special_block_transformation_state(nat me, byte h[26]) {
    ///TODO: generate these randomly at the start of the server.
    return false;
}

nat environment(nat c, byte h[26]) {
    if (is_in_special_block_transformation_state(c, h)) return c + 1;
    else if (c > 0 and (count(c, h) < 4 or count(c, h) > 5)) return c - 1;
    else if (c == 0 and count(1, h) == 5) return 1;
    else return c;
}

void evolve_space(uint32_t T, byte h[26]) {
    
    return; // TEMP, static space for debugging.
    
    byte* new_space = (byte*) malloc(game.size);
    memcpy(new_space, space, game.size);
    space_lock.lock();
    for (nat i = 0; i < game.size; i++) {
        
        byte c = new_space[i];
        if (T % P[c] == 0) {
//            get_moore_neighborhood(h, i);     /// rework this function!! make it use a single index.
            space[i] = environment(c, h);
        }
    }
    space_lock.unlock();
    free(new_space);
}

void simulate() {
    
    byte h[26] = {0};
    uint32_t timestep = 0;
    
    while (network::server_running) {
//        debug();
        
        game.players.erase(std::remove_if(game.players.begin(), game.players.end(), [](const player& player){
            return not at(player.location);
        }), game.players.end());
        
        evolve_space(timestep++, h);
        usleep(tick_us_delay);
        
        network::server_running = false;
    }
}

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>




int main(const int argc, const char** argv) {
    
    
    
    
    unsigned int port = 10000;
    
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (!fd) { perror("socket"); exit(1); }
    
    struct sockaddr_in servaddr, cliaddr;
    
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    servaddr.sin_family = AF_INET;
    
    socklen_t len = sizeof(cliaddr);
    
    printf("listening...\n");
    
    bind(fd, (struct sockaddr*) &servaddr, sizeof(servaddr));
    
    while (1) {
        
        char buffer[1000];
        const char* message = "Hello there you little Client";
        
        const size_t n = recvfrom(fd, buffer, sizeof buffer, 0, (struct sockaddr*)&cliaddr, &len);
        
        buffer[n] = '\0';
        puts(buffer);
        
        sendto(fd, message, strlen(message), 0, (struct sockaddr*)&cliaddr, len);
    }
    
    exit(0);
    
        
    generate_universe(argc, argv);
    atexit(write_game);
    signal(SIGINT, signal_write_game);
    std::thread thread(simulate);
    network::listen(port, handler);
    thread.join();
}
