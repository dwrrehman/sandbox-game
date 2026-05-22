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
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef i64 nat;

static const bool debug_mode = false;
static const i64 max_spawn_attempts = 100000;
static const i64 max_player_count = 10;

enum blocks {
	void_block,
	grass_block,
	dirt_block,
	sand_block,
	rock_block,
	stone_block,
	calcite_block,
	coal_block,
	gem_block,
	player_block,
};

// f00f=blue f0f0=pink 0fff=yellow fff0=white ffff=white 0f0f=green 00ff=red 000f=black 0000=black.

///     B    G    R    A
static const u8 colors[] = {
	0,0,0,0, 		// void
	73,132,30,255, 		// grass
	0,44,110,255, 		// dirt
	13,172,212,255, 	// sand
	183,182,179,255, 	// rock
	126,125,113,255,    	// stone
	199,195,189,255, 	// calcite
	71,55,40,255, 		// coal
	0,255,0,255, 		// gem
	255,255,0,255, 		// player

	0xff,0xf8,0x00,0xff,
	0xff,0x09,0x00,0xff,
	0xff,0xf8,0x00,0xff,
	0xff,0x09,0x00,0xff,
	0xff,0xf8,0x00,0xff,
	0xff,0x09,0x00,0xff,
};



struct player {
	i64 id0;
	i64 id1;

	nat x;
	nat y;

	nat width;
	nat height;

	nat active;

	nat selected;

	nat break_progress;    // player local number, less than the block being broken..?  must be based on time!!! 

	socklen_t length;
	struct sockaddr_in6 address;

	u8 inventory[32];
};

static u8* universe = NULL;
static nat universe_count = 0;
static nat side_length = 0;

static struct player* players = NULL;
static nat player_count = 0;

static int server = 0;
static bool server_running = true;

#define check(n) do { if (n == 0 || n < 0) printf("error(%ld): %s line:%d func:%s\n", n, __FILE__, __LINE__, __func__); } while (0)

static inline void ipv6_string(char buffer[40], u8 ip[16]) {
	sprintf(buffer,
	"%02hhx%02hhx:%02hhx%02hhx:" "%02hhx%02hhx:%02hhx%02hhx:"
	"%02hhx%02hhx:%02hhx%02hhx:" "%02hhx%02hhx:%02hhx%02hhx",
	ip[0], ip[1], ip[2], ip[3],  ip[4], ip[5], ip[6], ip[7], 
	ip[8], ip[9], ip[10], ip[11], ip[12], ip[13], ip[14], ip[15]);
}

static inline nat square_root(nat op) {
    nat res = 0, one = 0x4000000000000000; 
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

static inline u8* at(nat x, nat y, nat x_off, nat y_off) {
	nat xo = (x + x_off + side_length) % side_length;
	nat yo = (y + y_off + side_length) % side_length;
	return universe + xo * side_length + yo;
}

// static inline u8* at_player(nat p, nat x_off, nat y_off) {
// 	nat xo = (players[p].x + x_off + side_length) % side_length;
// 	nat yo = (players[p].y + y_off + side_length) % side_length;
// 	return universe + xo * side_length + yo;
// }

static inline nat distance(nat origin_x, nat origin_y, nat point_x, nat point_y) {
	const nat x_diff = origin_x - point_x;
	const nat y_diff = origin_y - point_y;
	return square_root(x_diff * x_diff + y_diff * y_diff);
}

static inline void generate() {
	universe_count = side_length * side_length;
	universe = calloc((size_t) universe_count, 1);
	if (not universe) { perror("calloc"); abort(); }
	
	printf("server: generating %llu bytes for universe...\n", universe_count);

	// generate grass
	for (nat i = 0; i < universe_count; i++) {
		universe[i] = (u8)((rand() % (dirt_block + 1)) * (rand() % 2) * (rand() % 2));
	}

	// generate rocks, 
	for (nat i = 0; i < universe_count; i++) {
		if (universe[i] == void_block and ((rand() % 3) == 0)) 
			universe[i] = (u8)((rand() % (coal_block + 1)) * (rand() % 2) * (rand() % 2));
	}

	// generate stones, 
	for (nat x = 0; x < side_length; x++) {
		for (nat y = 0; y < side_length; y++) {

			if (*at(x, y, 0, 0) == void_block and (rand() % 300 == 0)) {

				*at(x, y, 0, 0) = gem_block;
				*at(x, y, -1, 0) = (rand() % 2) ? calcite_block : coal_block;
				*at(x, y, 1, 0) = (rand() % 2) ? calcite_block : coal_block;
				*at(x, y, 0, -1) = (rand() % 2) ? calcite_block : coal_block;
				*at(x, y, 0, 1) = (rand() % 2) ? calcite_block : coal_block;
			}
		}
	}

	
	for (nat x = 0; x < side_length; x++) {
		for (nat y = 0; y < side_length; y++) {

			if (*at(x, y, 0, 0) == void_block and (rand() % 150 == 0)) {
			
				nat size = rand() % 5;
				if (size < 1) size = 1;

				for (nat xo = -size; xo < size; xo++) {
					for (nat yo = -size; yo < size; yo++) {
						if (distance(0,0, xo, yo) < size) 
							*at(x,y,xo,yo) = (rand() % 2) ? stone_block : calcite_block;
					}
				}
			}
		}
	}
}

static inline void spawn_player(nat p) {

	nat x = 0, y = 0;
	for (nat _ = 0; _ < max_spawn_attempts; _++) {
		x = rand() % side_length;
		y = rand() % side_length;
		if (universe[side_length * y + x] == void_block) {
			players[p].x = x;
			players[p].y = y;
		}
	}
	printf("server: error: spawn aborted for player %llu after %llu attempts\n", p, max_spawn_attempts);

	players[p].x = 0;
	players[p].y = 0;
}

static inline void move_up(nat p) {

	nat y = players[p].y;
	if (y) y--; else y = side_length - 1;
	if (universe[side_length * y + players[p].x]) return;

	if (debug_mode) printf("server: MOVE UP : player #%lld, player id: %llx_%llx : was at x=%llu y=%llu\n", p, players[p].id0, players[p].id1, players[p].x, players[p].y);
	universe[side_length * players[p].y + players[p].x] = void_block;
	if (players[p].y) players[p].y--; else players[p].y = side_length - 1;
	universe[side_length * players[p].y + players[p].x] = player_block;
}

static inline void move_down(nat p) {
	nat y = players[p].y;
	if (y == side_length - 1) y = 0; else y++;
	if (universe[side_length * y + players[p].x]) return;
	if (debug_mode) printf("server: MOVE DOWN : player #%lld, player id: %llx_%llx : was at x=%llu y=%llu\n", p, players[p].id0, players[p].id1, players[p].x, players[p].y);
	universe[side_length * players[p].y + players[p].x] = void_block;
	if (players[p].y == side_length - 1) players[p].y = 0; else players[p].y++;
	universe[side_length * players[p].y + players[p].x] = player_block;
}

static inline void move_left(nat p) {
	nat x = players[p].x;
	if (x) x--; else x = side_length - 1;
	if (universe[side_length * players[p].y + x]) return;
	if (debug_mode) printf("server: MOVE LEFT : player #%lld, player id: %llx_%llx : was at x=%llu y=%llu\n", p, players[p].id0, players[p].id1, players[p].x, players[p].y);
	universe[side_length * players[p].y + players[p].x] = void_block;
	if (players[p].x) players[p].x--; else players[p].x = side_length - 1;
	universe[side_length * players[p].y + players[p].x] = player_block;
}

static inline void move_right(nat p) {
	nat x = players[p].x;
	if (x == side_length - 1) x = 0; else x++;
	if (universe[side_length * players[p].y + x]) return;
	if (debug_mode) printf("server: MOVE RIGHT : player #%lld, player id: %llx_%llx : was at x=%llu y=%llu\n", p, players[p].id0, players[p].id1, players[p].x, players[p].y);
	universe[side_length * players[p].y + players[p].x] = void_block;
	if (players[p].x == side_length - 1) players[p].x = 0; else players[p].x++;
	universe[side_length * players[p].y + players[p].x] = player_block;

	// if (*at_player(p,1,0)) return;
	// *at_player(p,0,0) = void_block;
	// if (players[p].x == side_length - 1) players[p].x = 0; else players[p].x++;
	// *at_player(p,0,0) = player_block;
}

static inline void place_up(nat p) {
	if (not players[p].inventory[players[p].selected]) return;
	if (debug_mode) printf("server: PLACE UP : player #%lld, player id: %llx_%llx \n", p, players[p].id0, players[p].id1);
	nat y = players[p].y;
	if (y) y--; else y = side_length - 1;
	if (universe[side_length * y + players[p].x]) return;
	universe[side_length * y + players[p].x] = players[p].inventory[players[p].selected];

	players[p].inventory[players[p].selected + 1]--;
	if (not players[p].inventory[players[p].selected + 1]) players[p].inventory[players[p].selected] = 0;
}

static inline void place_down(nat p) {
	if (not players[p].inventory[players[p].selected]) return;
	if (debug_mode) printf("server: PLACE DOWN : player #%lld, player id: %llx_%llx \n", p, players[p].id0, players[p].id1);
	nat y = players[p].y;
	if (y == side_length - 1) y = 0; else y++;
	if (universe[side_length * y + players[p].x]) return;
	universe[side_length * y + players[p].x] = players[p].inventory[players[p].selected];

	players[p].inventory[players[p].selected + 1]--;
	if (not players[p].inventory[players[p].selected + 1]) players[p].inventory[players[p].selected] = 0;
}

static inline void place_left(nat p) {
	if (not players[p].inventory[players[p].selected]) return;
	if (debug_mode) printf("server: PLACE LEFT : player #%lld, player id: %llx_%llx \n", p, players[p].id0, players[p].id1);
	nat x = players[p].x;
	if (x) x--; else x = side_length - 1;
	if (universe[side_length * players[p].y + x]) return;
	universe[side_length * players[p].y + x] = players[p].inventory[players[p].selected];

	players[p].inventory[players[p].selected + 1]--;
	if (not players[p].inventory[players[p].selected + 1]) players[p].inventory[players[p].selected] = 0;
}

static inline void place_right(nat p) {
	if (not players[p].inventory[players[p].selected]) return;
	if (debug_mode) printf("server: PLACE RIGHT : player #%lld, player id: %llx_%llx \n", p, players[p].id0, players[p].id1);
	nat x = players[p].x;
	if (x == side_length - 1) x = 0; else x++;
	if (universe[side_length * players[p].y + x]) return;
	universe[side_length * players[p].y + x] = players[p].inventory[players[p].selected];

	players[p].inventory[players[p].selected + 1]--;
	if (not players[p].inventory[players[p].selected + 1]) players[p].inventory[players[p].selected] = 0;
}

static inline void break_up(nat p) {

	if (debug_mode) printf("server: PLACE UP : player #%lld, player id: %llx_%llx \n", p, players[p].id0, players[p].id1);
	nat y = players[p].y;
	if (y) y--; else y = side_length - 1;
	universe[side_length * y + players[p].x] = void_block; 
}

static inline void break_down(nat p) {

	if (debug_mode) printf("server: PLACE DOWN : player #%lld, player id: %llx_%llx \n", p, players[p].id0, players[p].id1);
	nat y = players[p].y;
	if (y == side_length - 1) y = 0; else y++;
	universe[side_length * y + players[p].x] = void_block; 
}

static inline void break_left(nat p) {

	if (debug_mode) printf("server: PLACE LEFT : player #%lld, player id: %llx_%llx \n", p, players[p].id0, players[p].id1);
	nat x = players[p].x;
	if (x) x--; else x = side_length - 1;
	universe[side_length * players[p].y + x] = void_block;
}

static inline void break_right(nat p) {

	if (debug_mode) printf("server: PLACE RIGHT : player #%lld, player id: %llx_%llx \n", p, players[p].id0, players[p].id1);
	nat x = players[p].x;
	if (x == side_length - 1) x = 0; else x++;
	universe[side_length * players[p].y + x] = void_block;
}

static inline nat identify_player_from_ip(unsigned char ip[16]) {
	i64 id0 = 0, id1 = 0;
	memcpy(&id0, ip, 8);
	memcpy(&id1, ip + 8, 8);
	
	for (nat p = 0; p < player_count; p++) {
		if (players[p].id0 == id0 and 
		    players[p].id1 == id1) return p;
	}

	return player_count;
}


static inline void save_state(const char* destination) {
	FILE* file = fopen(destination, "w");
	if (not file) { perror("open"); exit(3); }
	fwrite(universe, 1, (size_t) universe_count, file);
	fclose(file);

	printf("saved %llu bytes of universe (s=%llu)...\n", universe_count, side_length);
}

static inline void save_players(const char* destination) {
	FILE* file = fopen(destination, "w");
	if (not file) { perror("open"); exit(3); }
	fwrite(&player_count, 8, 1, file);
	fwrite(players, sizeof(struct player), (size_t) player_count, file);
	fclose(file);

	printf("saved %lld player's data.\n", player_count);
}

static inline void load_state(const char* source) {
	FILE* file = fopen(source, "r");
	if (not file) { perror("fopen"); exit(3); }
	fseek(file, 0, SEEK_END);
	universe_count = (nat) ftell(file);
	side_length = square_root(universe_count);
	fseek(file, 0, SEEK_SET);
	universe = malloc((size_t) universe_count);	
	fread(universe, 1, (size_t) universe_count, file);
	fclose(file);
	printf("loaded %llu bytes of existing universe (side_length=%llu)...\n", universe_count, side_length);
}

static inline void load_players(const char* source) {
	FILE* file = fopen(source, "r");
	if (not file) { perror("fopen"); exit(3); }
	fread(&player_count, 8, 1, file);
	fread(players, sizeof (struct player), (size_t) player_count, file);
	fclose(file);
	printf("loaded %lld player's data.\n", player_count);
}


static inline void save(const char* state_file, const char* players_file) {
	printf("server: saving universe...\n");
	save_state(state_file);
	save_players(players_file);
}

int main(const int argc, const char** argv) {

	if (argc < 4) exit(puts( "usage: ./server <port> <s> <universe>\n[s?generate(s):load(universe)]"));
	
	srand((unsigned)time(0));
	i16 port = (i16) atoi(argv[1]);
	side_length = (nat) atoll(argv[2]);

	char players_file[4096] = {0};
	strncpy(players_file, argv[3], sizeof players_file - 1);
	strncat(players_file, "/players.blob", sizeof players_file - strlen(players_file) - 1);

	char state_file[4096] = {0};
	strncpy(state_file, argv[3], sizeof state_file - 1);
	strncat(state_file, "/state.blob", sizeof state_file - strlen(players_file) - 1);
	
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
	
		for (nat player = 0; player < player_count; player++) {
			if (not players[player].active) continue;

			if (debug_mode) printf("sending DP for player #%llu!\n", player);
			u8 packet[22 * 17 * 4] = {0};

			struct player p = players[player];
	
			nat width = p.width / 2;
			nat height = p.height / 2;
			
			for (nat y_p = 0, y_off = -height; y_p < 16; y_off++, y_p++) {
				for (nat x_p = 0, x_off = -width; x_p < 22; x_off++, x_p++) {

					const nat xo = (p.x + x_off + side_length) % side_length;
					const nat yo = (p.y + y_off + side_length) % side_length;
					const u8 n = universe[yo * side_length + xo];
					if (n == 0) continue;

					const nat i = 22 * y_p + x_p;
					packet[4 * i + 0] = colors[4 * n + 0];
					packet[4 * i + 1] = colors[4 * n + 1];
					packet[4 * i + 2] = colors[4 * n + 2];
					packet[4 * i + 3] = colors[4 * n + 3];
				}
			}

			for (nat slot = 0; slot < 20; slot += 2) {

				const u8 block = p.inventory[slot + 0];
				const u8 count = p.inventory[slot + 1];

				const nat index0 = 22 * 16 + slot;
				packet[4 * index0 + 0] = colors[4 * block + 0];
				packet[4 * index0 + 1] = colors[4 * block + 1];
				packet[4 * index0 + 2] = colors[4 * block + 2];
				packet[4 * index0 + 3] = colors[4 * block + 3];

				const nat index1 = 22 * 16 + slot + 1;
				packet[4 * index1 + 0] = count;
				packet[4 * index1 + 1] = count;
				packet[4 * index1 + 2] = count;
				packet[4 * index1 + 3] = 255;
			}

			const nat i = 22 * 15 + p.selected;
			packet[4 * i + 0] = 255;
			packet[4 * i + 1] = 255;
			packet[4 * i + 2] = 255;
			packet[4 * i + 3] = 255;

			ssize_t error = sendto(server, packet, 22 * 17 * 4, 0, 
					(struct sockaddr*)& (players[player].address), players[player].length);
			check(error);
		}


		if (debug_mode) printf("server: universe ticked!\n");


		ssize_t error = recvfrom(server, &command, 1, MSG_DONTWAIT, (struct sockaddr*)&address, &length);
		if (error < 0) continue;
		
		ipv6_string(ip, address.sin6_addr.s6_addr); // put in connect requ.

		nat player = identify_player_from_ip(address.sin6_addr.s6_addr);
		if (player == player_count and command != 'C') { 
				printf("server: received packet from unknown IP: %s, ignoring...\n", ip); 
				continue; 
		} else 
			{ if (debug_mode) printf("server: received command byte from player #%lld, IP: %s, processing...\n", player, ip); }

		if (command == 'H') server_running = false;
		else if (command == 'S') save(state_file, players_file);

		else if (command == 'w') move_up(player);
		else if (command == 's') move_down(player);
		else if (command == 'a') move_left(player);
		else if (command == 'd') move_right(player);

		else if (command == 'i') break_up(player);
		else if (command == 'k') break_down(player);
		else if (command == 'j') break_left(player);
		else if (command == 'l') break_right(player);

		else if (command == 't') place_up(player);
		else if (command == 'g') place_down(player);
		else if (command == 'f') place_left(player);
		else if (command == 'h') place_right(player);

		else if (command == 'p') players[player].selected = (players[player].selected + 2) % 20;
		else if (command == 'o') players[player].selected = (players[player].selected + 18) % 20;

		else if (command == 'C') {
			if (debug_mode) printf("server: [%s]: new player connected to server! generating new player...\n", ip);
			error = sendto(server, "A", 1, 0, (struct sockaddr*)&address, length); check(error);

			players[player].address = address;
			players[player].length = length;
			players[player].active = true; 

			if (player < player_count) { 
				if (debug_mode) printf("server: [%llu / %llu]: RETURNING/EXISTING player's uuid is: %llx_%llx\n", 
					player, player_count, players[player].id0, players[player].id1);

			} else if (player == player_count) {

				memcpy(&players[player].id0, address.sin6_addr.s6_addr, 8);
				memcpy(&players[player].id1, address.sin6_addr.s6_addr + 8, 8);
				
				players[player].width = 22;
				players[player].height = 17;

				players[player].selected = 0;
				memset(players[player].inventory, 0, sizeof players[player].inventory);

				//testing:

				players[player].inventory[0] = stone_block;
				players[player].inventory[1] = 32;

				players[player].inventory[2] = sand_block;
				players[player].inventory[3] = 64;

				players[player].inventory[4] = dirt_block;
				players[player].inventory[5] = 35;

				players[player].inventory[6] = calcite_block;
				players[player].inventory[7] = 4;
		
			
				spawn_player(player);
				universe[side_length * players[player].y + players[player].x] = player_block;
				player_count++;

				if (debug_mode) printf("server: [%llu / %llu]: player's uuid is: %llx_%llx\n", 
					player, player_count, players[player].id0, players[player].id1);
			}

		} else if (command == 'D') {
			if (debug_mode) printf("server: [%s]: info: client sent a disconnection request!\n", ip); 
			players[player].active = false;

		} else printf("server: [%s]: warning: received unknown commmand: %c\n", ip, command);
	}

	for (nat i = 0; i < player_count; i++) players[i].active = false;
	save(state_file, players_file);
	close(server);
}

