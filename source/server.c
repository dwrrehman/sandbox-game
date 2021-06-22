// UDP server for my multiplayer game.
#include <iso646.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

struct player {
	u64 id0;
	u64 id1;
	u64 x;
	u64 y;
	u64 width;
	u64 height;
	u64 active;
	u64 unused;
	socklen_t length;
	struct sockaddr_in6 address;
};

static u8* universe = NULL;
static u64 universe_count = 0;
static u64 side_length = 0;

static struct player* players = NULL;
static u32 player_count = 0;

static int server = 0;
static bool server_running = true;

static bool debug_mode = false;

#define check(n) { if (n == 0 or n < 0) printf("error(%ld): in file:%s line:%d func:%s\n", n, __FILE__, __LINE__, __func__); }

static inline void ipv6_string(char buffer[40], u8 ip[16]) {
	sprintf(buffer,
	"%02hhx%02hhx:%02hhx%02hhx:" "%02hhx%02hhx:%02hhx%02hhx:"
	"%02hhx%02hhx:%02hhx%02hhx:" "%02hhx%02hhx:%02hhx%02hhx",
	ip[0], ip[1], ip[2], ip[3],  ip[4], ip[5], ip[6], ip[7], 
	ip[8], ip[9], ip[10], ip[11], ip[12], ip[13], ip[14], ip[15]);
}

static inline void generate() {
	universe_count = side_length * side_length;
	universe = calloc(universe_count, 1);
	if (not universe) { perror("calloc"); abort(); }
	
	printf("server: generating %llu bytes for universe...\n", universe_count);
	for (u64 i = 0; i < universe_count; i++) {
		universe[i] = (rand() % 2) * (rand() % 2) * (rand() % 2);
	}
}

static inline void spawn_player(u32 p) {

	const u32 max_spawn_attempts = 100;

	u64 x = 0, y = 0;
	for (u32 _ = 0; _ < max_spawn_attempts; _++) {
		x = (u64) rand() % side_length;
		y = (u64) rand() % side_length;
		if (universe[side_length * y + x] == 0) {
			players[p].x = x;
			players[p].y = y;
		}
	}
	printf("server: error: spawn aborted for player %u after %u attempts\n", p, max_spawn_attempts);
}

static inline void move_up(u32 p) {

	u64 y = players[p].y;
	if (y) y--; else y = side_length - 1;
	if (universe[side_length * y + players[p].x]) return;

	if (debug_mode) printf("server: MOVE UP : player #%d, player id: %llx_%llx : was at x=%llu y=%llu\n", p, players[p].id0, players[p].id1, players[p].x, players[p].y);
	universe[side_length * players[p].y + players[p].x] = 0;
	if (players[p].y) players[p].y--; else players[p].y = side_length - 1;
	universe[side_length * players[p].y + players[p].x] = 2;
}

static inline void move_down(u32 p) {
	u64 y = players[p].y;
	if (y == side_length - 1) y = 0; else y++;
	if (universe[side_length * y + players[p].x]) return;
	if (debug_mode) printf("server: MOVE DOWN : player #%d, player id: %llx_%llx : was at x=%llu y=%llu\n", p, players[p].id0, players[p].id1, players[p].x, players[p].y);
	universe[side_length * players[p].y + players[p].x] = 0;
	if (players[p].y == side_length - 1) players[p].y = 0; else players[p].y++;
	universe[side_length * players[p].y + players[p].x] = 2;
}

static inline void move_left(u32 p) {
	u64 x = players[p].x;
	if (x) x--; else x = side_length - 1;
	if (universe[side_length * players[p].y + x]) return;
	if (debug_mode) printf("server: MOVE LEFT : player #%d, player id: %llx_%llx : was at x=%llu y=%llu\n", p, players[p].id0, players[p].id1, players[p].x, players[p].y);
	universe[side_length * players[p].y + players[p].x] = 0;
	if (players[p].x) players[p].x--; else players[p].x = side_length - 1;
	universe[side_length * players[p].y + players[p].x] = 2;
}

static inline void move_right(u32 p) {
	u64 x = players[p].x;
	if (x == side_length - 1) x = 0; else x++;
	if (universe[side_length * players[p].y + x]) return;
	if (debug_mode) printf("server: MOVE RIGHT : player #%d, player id: %llx_%llx : was at x=%llu y=%llu\n", p, players[p].id0, players[p].id1, players[p].x, players[p].y);
	universe[side_length * players[p].y + players[p].x] = 0;
	if (players[p].x == side_length - 1) players[p].x = 0; else players[p].x++;
	universe[side_length * players[p].y + players[p].x] = 2;
}

static inline void place_up(u32 p) {

	if (debug_mode) printf("server: PLACE UP : player #%d, player id: %llx_%llx \n", p, players[p].id0, players[p].id1);
	u64 y = players[p].y;
	if (y) y--; else y = side_length - 1;
	universe[side_length * y + players[p].x] = 1; 
}

static inline void place_down(u32 p) {

	if (debug_mode) printf("server: PLACE DOWN : player #%d, player id: %llx_%llx \n", p, players[p].id0, players[p].id1);
	u64 y = players[p].y;
	if (y == side_length - 1) y = 0; else y++;
	universe[side_length * y + players[p].x] = 1; 
}

static inline void place_left(u32 p) {

	if (debug_mode) printf("server: PLACE LEFT : player #%d, player id: %llx_%llx \n", p, players[p].id0, players[p].id1);
	u64 x = players[p].x;
	if (x) x--; else x = side_length - 1;
	universe[side_length * players[p].y + x] = 1;
}

static inline void place_right(u32 p) {

	if (debug_mode) printf("server: PLACE RIGHT : player #%d, player id: %llx_%llx \n", p, players[p].id0, players[p].id1);
	u64 x = players[p].x;
	if (x == side_length - 1) x = 0; else x++;
	universe[side_length * players[p].y + x] = 1;
}

static inline void break_up(u32 p) {

	if (debug_mode) printf("server: PLACE UP : player #%d, player id: %llx_%llx \n", p, players[p].id0, players[p].id1);
	u64 y = players[p].y;
	if (y) y--; else y = side_length - 1;
	universe[side_length * y + players[p].x] = 0; 
}

static inline void break_down(u32 p) {

	if (debug_mode) printf("server: PLACE DOWN : player #%d, player id: %llx_%llx \n", p, players[p].id0, players[p].id1);
	u64 y = players[p].y;
	if (y == side_length - 1) y = 0; else y++;
	universe[side_length * y + players[p].x] = 0; 
}

static inline void break_left(u32 p) {

	if (debug_mode) printf("server: PLACE LEFT : player #%d, player id: %llx_%llx \n", p, players[p].id0, players[p].id1);
	u64 x = players[p].x;
	if (x) x--; else x = side_length - 1;
	universe[side_length * players[p].y + x] = 0;
}

static inline void break_right(u32 p) {

	if (debug_mode) printf("server: PLACE RIGHT : player #%d, player id: %llx_%llx \n", p, players[p].id0, players[p].id1);
	u64 x = players[p].x;
	if (x == side_length - 1) x = 0; else x++;
	universe[side_length * players[p].y + x] = 0;
}

static inline u32 identify_player_from_ip(unsigned char ip[16]) {
	u64 id0 = 0, id1 = 0;
	memcpy(&id0, ip, 8);
	memcpy(&id1, ip + 8, 8);
	
	for (u32 p = 0; p < player_count; p++) {
		if (players[p].id0 == id0 and 
		    players[p].id1 == id1) return p;
	}

	return player_count;
}


// static inline u8 at(u64 x, u64 y, i64 x_off, i64 y_off) {
// 	u64 yo = (u64)((i64)y + y_off + (i64)side_length) % side_length;
// 	u64 xo = (u64)((i64)x + x_off + (i64)side_length) % side_length;
// 	return universe[xo * side_length + yo];
// }

static inline u64 square_root(u64 op) {
    u64 res = 0, one = 0x4000000000000000; 
    while (one > op) one >>= 2;
    while (one) {
        if (op >= res + one) {
            op -= (res + one);
            res += one << 1;
        }
        res >>= 1;
        one >>= 2;
    }
    return res;
}










static inline void save_state(const char* destination) {
	FILE* file = fopen(destination, "w");
	if (not file) { perror("open"); exit(3); }
	fwrite(universe, 1, universe_count, file);
	fclose(file);

	printf("saved %llu bytes of universe (s=%llu)...\n", universe_count, side_length);
}

static inline void save_players(const char* destination) {
	FILE* file = fopen(destination, "w");
	if (not file) { perror("open"); exit(3); }
	fwrite(&player_count, 4, 1, file);
	fwrite(players, sizeof(struct player), player_count, file);
	fclose(file);

	printf("saved %d player's data.\n", player_count);
}

static inline void load_state(const char* source) {
	FILE* file = fopen(source, "r");
	if (not file) { perror("fopen"); exit(3); }
	fseek(file, 0, SEEK_END);
	universe_count = (u64) ftell(file);
	side_length = square_root(universe_count);
	fseek(file, 0, SEEK_SET);
	universe = malloc(universe_count);	
	fread(universe, 1, universe_count, file);
	fclose(file);
	printf("loaded %llu bytes of existing universe (side_length=%llu)...\n", universe_count, side_length);
}

static inline void load_players(const char* source) {
	FILE* file = fopen(source, "r");
	if (not file) { perror("fopen"); exit(3); }
	fread(&player_count, 4, 1, file);
	fread(players, sizeof (struct player), player_count, file);
	fclose(file);
	printf("loaded %d player's data.\n", player_count);
}

int main(const int argc, const char** argv) {
	
	// u16 port = default_port;
	// if (argc >= 2) {
	// 	u16 n = (u16) atoi(argv[1]);
	// 	if (n >= 1024) port = n;
	// }

	// srand((unsigned)time(0));

	const int max_player_count = 10;

	if (argc < 4) exit(puts( "usage: ./server <port> <s> <universe>\n[s?generate(s):load(universe)]"));
	
	srand((unsigned)time(0));
	u16 port = (u16) atoi(argv[1]);
	side_length = (u64) atoll(argv[2]);

	char players_file[128] = {0};
	strncpy(players_file, argv[3], 127);
	strncat(players_file, "/players.blob", 127);

	char state_file[128] = {0};
	strncpy(state_file, argv[3], 127);
	strncat(state_file, "/state.blob", 127);
	
	players = malloc(max_player_count * sizeof(struct player));

	if (side_length) { mkdir(argv[3], 0700); generate(); }
	else { load_state(state_file); load_players(players_file); }

	printf("server: listening on %hu...\n", port);

	server = socket(PF_INET6, SOCK_DGRAM, 0);
	if (server < 0) { perror("socket"); abort(); }
	struct sockaddr_in6 server_address = {0};
	server_address.sin6_family = PF_INET6;
	server_address.sin6_port = htons(port);
	server_address.sin6_addr = in6addr_any;

	int result = bind(server, (struct sockaddr*) &server_address, sizeof server_address);
	if (result < 0) { perror("bind"); abort(); }

	u8 command = 0;	
	char ip[40] = {0};
	struct sockaddr_in6 address = {0};
	socklen_t length = sizeof address;

	while (server_running) {
		usleep(50000);
	
		for (uint32_t player = 0; player < player_count; player++) {
			if (not players[player].active) continue;

			if (debug_mode) printf("sending DP for player #%u!\n", player);
			u8 packet[1200] = {0};

			struct player p = players[player];

			int64_t width = p.width / 2;
			int64_t height = p.height / 2;
			
			for (int64_t y_p = 0, y_off = -height; y_p < 15; y_off++, y_p++) {
				for (int64_t x_p = 0, x_off = -width; x_p < 20; x_off++, x_p++) {

					const uint64_t xo = (uint64_t)((int64_t)p.x + (int64_t)x_off + (int64_t)side_length) % side_length;
					const uint64_t yo = (uint64_t)((int64_t)p.y + (int64_t)y_off + (int64_t)side_length) % side_length;
					const uint8_t n = universe[yo * side_length + xo];

					if (n == 0) continue;

					const int64_t i = 20 * y_p + x_p;

					const u8 colors[] = {
						0x00,0x00,0x00,0x00,  
						0xff,0xff,0xff,0xff, 
						0xff,0xff,0x00,0xff 
					};

					packet[4 * i + 0] = colors[4 * n + 0];
					packet[4 * i + 1] = colors[4 * n + 1];
					packet[4 * i + 2] = colors[4 * n + 2];
					packet[4 * i + 3] = colors[4 * n + 3];
				}
			}

			ssize_t error = sendto(server, packet, 4 * 20 * 15, 0, 
					(struct sockaddr*)& (players[player].address), players[player].length);
			check(error);
		}

		if (debug_mode) printf("server: universe ticked!\n");
		universe[0] = !universe[0];
		
		ssize_t error = recvfrom(server, &command, 1, MSG_DONTWAIT, (struct sockaddr*)&address, &length);
		if (error < 0) continue;
		
		ipv6_string(ip, address.sin6_addr.s6_addr); // put in connect requ.

		u32 player = identify_player_from_ip(address.sin6_addr.s6_addr);
		if (player == player_count and command != 'C') { 
				printf("server: received packet from unknown IP: %s, ignoring...\n", ip); 
				continue; 
		} else 
			{ if (debug_mode) printf("server: received command byte from player #%d, IP: %s, processing...\n", player, ip); }

		if (command == 'H') server_running = false;
		else if (command == 'w') move_up(player);
		else if (command == 's') move_down(player);
		else if (command == 'a') move_left(player);
		else if (command == 'd') move_right(player);
		else if (command == 'i') place_up(player);
		else if (command == 'k') place_down(player);
		else if (command == 'j') place_left(player);
		else if (command == 'l') place_right(player);
		else if (command == 't') break_up(player);
		else if (command == 'g') break_down(player);
		else if (command == 'f') break_left(player);
		else if (command == 'h') break_right(player);

		else if (command == 'C') {
			if (debug_mode) printf("server: [%s]: new player connected to server! generating new player...\n", ip);
			error = sendto(server, "A", 1, 0, (struct sockaddr*)&address, length); check(error);

			players[player].address = address;
			players[player].length = length;
			players[player].active = true; 

			if (player < player_count) { 
				if (debug_mode) printf("server: [%u / %u]: RETURNING/EXISTING player's uuid is: %llx_%llx\n", 
					player, player_count, players[player].id0, players[player].id1);

			} else if (player == player_count) {

				memcpy(&players[player].id0, address.sin6_addr.s6_addr, 8);
				memcpy(&players[player].id1, address.sin6_addr.s6_addr + 8, 8);
				
				players[player].width = 20;
				players[player].height = 15;

				spawn_player(player);
				universe[side_length * players[player].y + players[player].x] = 2; // notate where player is now, in universe.
				
				player_count++;

				if (debug_mode) printf("server: [%u / %u]: player's uuid is: %llx_%llx\n", 
					player, player_count, players[player].id0, players[player].id1);
			}

		} else if (command == 'D') {
			if (debug_mode) printf("server: [%s]: info: client sent a disconnection request!\n", ip); 
			players[player].active = false;

		} else printf("server: [%s]: warning: received unknown commmand: %c\n", ip, command);
	}

	for (u32 i = 0; i < player_count; i++) players[i].active = false;

	printf("server: saving universe...\n");
	save_state(state_file);
	save_players(players_file);
	printf("server: halting...\n"); 
	close(server);
}
