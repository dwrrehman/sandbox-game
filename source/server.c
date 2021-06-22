// UDP server for my multiplayer game.
#include <iso646.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

static const u16 default_port = 12000;

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

static struct player players[10];
static u32 player_count = 0;

static bool server_running = true;
static int server = 0;

#define check(n) { if (n == 0 or n < 0) printf("error(%ld): in file:%s line:%d func:%s\n", n, __FILE__, __LINE__, __func__); }

static inline void ipv6_string(char buffer[40], u8 ip[16]) {
	sprintf(buffer,
	"%02hhx%02hhx:%02hhx%02hhx:" "%02hhx%02hhx:%02hhx%02hhx:"
	"%02hhx%02hhx:%02hhx%02hhx:" "%02hhx%02hhx:%02hhx%02hhx",
	ip[0], ip[1], ip[2], ip[3],  ip[4], ip[5], ip[6], ip[7], 
	ip[8], ip[9], ip[10], ip[11], ip[12], ip[13], ip[14], ip[15]);
}

static inline void generate(u64 s) {
	side_length = s;
	universe_count = side_length * side_length;
	universe = calloc(universe_count, 1);
	if (not universe) { perror("calloc"); abort(); }

	printf("server: generating %llu bytes for universe...\n", universe_count);
	for (u64 i = 0; i < universe_count; i++) {
		universe[i] = (rand() % 2) * (rand() % 2) * (rand() % 2) * (rand() % 2);
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

static inline void tick() {
	universe[0] = !universe[0];
	// universe[11] = !universe[11];
	return;
}

static inline void move_up(u32 p) {
	if (universe[side_length * players[p].y + players[p].x]) return;
	printf("server: MOVE UP : player #%d, player id: %llx_%llx : was at x=%llu y=%llu\n", p, players[p].id0, players[p].id1, players[p].x, players[p].y);
	universe[side_length * players[p].y + players[p].x] = 0;
	if (players[p].y) players[p].y--; else players[p].y = side_length - 1;
	universe[side_length * players[p].y + players[p].x] = 5;
}

static inline void move_down(u32 p) {
	if (universe[side_length * players[p].y + players[p].x]) return;
	printf("server: MOVE DOWN : player #%d, player id: %llx_%llx : was at x=%llu y=%llu\n", p, players[p].id0, players[p].id1, players[p].x, players[p].y);
	universe[side_length * players[p].y + players[p].x] = 0;
	if (players[p].y == side_length - 1) players[p].y = 0; else players[p].y++;
	universe[side_length * players[p].y + players[p].x] = 5;
}

static inline void move_left(u32 p) {
	if (universe[side_length * players[p].y + players[p].x]) return;
	printf("server: MOVE LEFT : player #%d, player id: %llx_%llx : was at x=%llu y=%llu\n", p, players[p].id0, players[p].id1, players[p].x, players[p].y);
	universe[side_length * players[p].y + players[p].x] = 0;
	if (players[p].x) players[p].x--; else players[p].x = side_length - 1;
	universe[side_length * players[p].y + players[p].x] = 5;
}

static inline void move_right(u32 p) {
	if (universe[side_length * players[p].y + players[p].x]) return;
	printf("server: MOVE RIGHT : player #%d, player id: %llx_%llx : was at x=%llu y=%llu\n", p, players[p].id0, players[p].id1, players[p].x, players[p].y);
	universe[side_length * players[p].y + players[p].x] = 0;
	if (players[p].x == side_length - 1) players[p].x = 0; else players[p].x++;
	universe[side_length * players[p].y + players[p].x] = 5;
}


static inline void place_up(u32 p) {
	if (universe[side_length * players[p].y + players[p].x]) return;
	printf("server: PLACE UP : player #%d, player id: %llx_%llx \n", p, players[p].id0, players[p].id1);
	u64 y = players[p].y;
	if (y) y--; else y = side_length - 1;
	universe[side_length * y + players[p].x] = 1; 
}

static inline void place_down(u32 p) {
	if (universe[side_length * players[p].y + players[p].x]) return;
	printf("server: PLACE DOWN : player #%d, player id: %llx_%llx \n", p, players[p].id0, players[p].id1);
	u64 y = players[p].y;
	if (y == side_length - 1) y = 0; else y++;
	universe[side_length * y + players[p].x] = 1; 
}

static inline void place_left(u32 p) {
	if (universe[side_length * players[p].y + players[p].x]) return;
	printf("server: PLACE LEFT : player #%d, player id: %llx_%llx \n", p, players[p].id0, players[p].id1);
	u64 x = players[p].x;
	if (x) x--; else x = side_length - 1;
	universe[side_length * players[p].y + x] = 1;
}

static inline void place_right(u32 p) {
	if (universe[side_length * players[p].y + players[p].x]) return;
	printf("server: PLACE RIGHT : player #%d, player id: %llx_%llx \n", p, players[p].id0, players[p].id1);
	u64 x = players[p].x;
	if (x == side_length - 1) x = 0; else x++;
	universe[side_length * players[p].y + x] = 1;
}

static inline void break_up(u32 p) {
	if (universe[side_length * players[p].y + players[p].x] == 0) return;
	printf("server: PLACE UP : player #%d, player id: %llx_%llx \n", p, players[p].id0, players[p].id1);
	u64 y = players[p].y;
	if (y) y--; else y = side_length - 1;
	universe[side_length * y + players[p].x] = 0; 
}

static inline void break_down(u32 p) {
	if (universe[side_length * players[p].y + players[p].x] == 0) return;
	printf("server: PLACE DOWN : player #%d, player id: %llx_%llx \n", p, players[p].id0, players[p].id1);
	u64 y = players[p].y;
	if (y == side_length - 1) y = 0; else y++;
	universe[side_length * y + players[p].x] = 0; 
}

static inline void break_left(u32 p) {
	if (universe[side_length * players[p].y + players[p].x] == 0) return;
	printf("server: PLACE LEFT : player #%d, player id: %llx_%llx \n", p, players[p].id0, players[p].id1);
	u64 x = players[p].x;
	if (x) x--; else x = side_length - 1;
	universe[side_length * players[p].y + x] = 0;
}

static inline void break_right(u32 p) {
	if (universe[side_length * players[p].y + players[p].x] == 0) return;
	printf("server: PLACE RIGHT : player #%d, player id: %llx_%llx \n", p, players[p].id0, players[p].id1);
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

int main(const int argc, const char** argv) {
	srand((unsigned)time(0));
	
	u16 port = default_port;
	if (argc >= 2) {
		u16 n = (u16) atoi(argv[1]);
		if (n >= 1024) port = n;
	}

	printf("server: listening on %hu...\n", port);

	server = socket(PF_INET6, SOCK_DGRAM, 0);
	if (server < 0) { perror("socket"); abort(); }
	struct sockaddr_in6 server_address = {0};
	server_address.sin6_family = PF_INET6;
	server_address.sin6_port = htons(port);
	server_address.sin6_addr = in6addr_any;

	int result = bind(server, (struct sockaddr*) &server_address, sizeof server_address);
	if (result < 0) { perror("bind"); abort(); }

	generate(10);

	// pthread_t thread;
	// pthread_create(&thread, NULL, compute, NULL);

	u8 command = 0;	
	char ip[40] = {0};
	struct sockaddr_in6 address = {0};
	socklen_t length = sizeof address;

	while (server_running) {
		usleep(1000);

		while (server_running) {
			for (u32 player = 0; player < player_count; player++) {
				if (not players[player].active) continue;

				u8 packet[400] = {0};
				for (u64 i = 0; i < universe_count; i++) {
					if (universe[i] == 1) {
						packet[4 * i + 0] = 255;
						packet[4 * i + 1] = 255;
						packet[4 * i + 2] = 255;
						packet[4 * i + 3] = 255;

					} else if (universe[i] == 5) {
						packet[4 * i + 0] = 255;
						packet[4 * i + 1] = 255;
						packet[4 * i + 2] = 0;
						packet[4 * i + 3] = 255;
					}
				}
				ssize_t error = sendto(server, packet, 4 * 10 * 10, 0, (struct sockaddr*)& (players[player].address), players[player].length);
				check(error);
			}
			tick();
		}
		
		ssize_t error = recvfrom(server, &command, 1, MSG_DONTWAIT, (struct sockaddr*)&address, &length);
		check(error);

		ipv6_string(ip, address.sin6_addr.s6_addr); // put in connect requ.

		u32 player = identify_player_from_ip(address.sin6_addr.s6_addr);
		if (player == player_count and command != 'C') { 
				printf("received packet from unknown IP: %s, ignoring...\n", ip); 
				continue; 
		} else 
			printf("received command byte from player #%d, IP: %s, processing...\n", player, ip);

		if (command == 'H') server_running = false;
		else if (command == 'w') move_up(player);
		else if (command == 's') move_down(player);
		else if (command == 'a') move_left(player);
		else if (command == 'd') move_right(player);
		else if (command == 'i') place_up(player);
		else if (command == 'j') place_down(player);
		else if (command == 'k') place_left(player);
		else if (command == 'l') place_right(player);
		else if (command == 't') break_up(player);
		else if (command == 'f') break_down(player);
		else if (command == 'g') break_left(player);
		else if (command == 'h') break_right(player);

		else if (command == 'C') {
			printf("server: [%s]: new player connected to server! generating new player...\n", ip);
			error = sendto(server, "A", 1, 0, (struct sockaddr*)&address, length); check(error);

			if (player < player_count) { 
				printf("[%u / %u]: RETURNING/EXISTING player's uuid is: %llx_%llx\n", player, player_count, players[player].id0, players[player].id1);
				players[player].active = true; 
				continue;
			}

			players[player].address = address;
			players[player].length = length;
			memcpy(&players[player].id0, address.sin6_addr.s6_addr, 8);
			memcpy(&players[player].id1, address.sin6_addr.s6_addr + 8, 8);
			spawn_player(player);
			players[player].width = 10;
			players[player].height = 10;
			universe[side_length * players[player].y + players[player].x] = 5; // notate where player is now, in universe.
			players[player].active = true;
			player_count++;

			printf("[%u / %u]: player's uuid is: %llx_%llx\n", player, player_count, players[player].id0, players[player].id1);

		} else if (command == 'D') {
			printf("server: [%s]: info: client sent a disconnection request!\n", ip); 
			players[player].active = false;

		} else printf("server: [%s]: warning: received unknown commmand: %c\n", ip, command);
		
	}
	printf("SERVER: halting...\n"); 
	// pthread_join(thread, NULL);
	close(server);
}
