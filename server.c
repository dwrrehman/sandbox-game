// server for a temporary 2D version of a minecraft-like sandbox game, 
// which im going to make to simply have a multiplayer sandbox game that i can use 
// for my own enjoyment. just for fun.

// the universe is a 2D dimensional CA, where m = 255, always.
// s, the sidelength of the square, (which is a 3d torus) is user-supplied.
//   its the size of universe.

#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdnoreturn.h>
#include <pthread.h>
#include <math.h>
#include <time.h>
#include <pwd.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

static const u32 max_player_count = 32;
static const u32 attempt_count = 10000;

enum commands {	
	null_command = 0,
	display = 9, 
	view_resized = 6,
	move_left = 12,
	move_right = 13,
	move_up = 14,
	move_down = 15,
	halt = 255, 
};

struct client {
	const char* ip;
	int connection;
	u32 player;
	u16 port;
	u16 padding;
	u32 padding1;
};

struct player {
	u64 x, y;
	u16 width, height;
	u32 padding;
	u8 hand; 
	i8 active;
	char name[30];
};

static bool server_running = true;
static int server = 0;

static u64 s = 0;
static u64 count = 0;
static u8* universe = NULL;

static u32 player_count = 0;
static struct player* players = NULL;

#define read_error() \
	{do { \
		printf("debug: server: read error! (n < 0) file:%s line:%d func:%s\n", __FILE__, __LINE__, __func__); \
		abort(); \
	} while(0);} \
/*
#define not_acked() \
	{do { \
		printf("debug: error: command not acknowledged from client file:%s line:%d func:%s \n", __FILE__, __LINE__, __func__); \
		abort(); \
	} while(0);} \
*/

#define disconnected() \
	{do { \
		printf("debug: error: disconnected:%s line:%d func:%s \n", __FILE__, __LINE__, __func__); \
		abort(); \
	} while(0);} \

#define check(n) \
	if (n == 0) { disconnected(); } \
	else if (n < 0) { read_error(); } \

static inline u8 at(u64 x, u64 y, i64 x_off, i64 y_off) {
	u64 yo = (u64)((i64)y + y_off + (i64)s) % s;
	u64 xo = (u64)((i64)x + x_off + (i64)s) % s;
	return universe[xo * s + yo];
}

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
	fwrite(universe, 1, count, file);
	fclose(file);

	printf("saved %llu bytes of universe (s=%llu)...\n", count, s);
}

static inline void save_players(const char* destination) {
	FILE* file = fopen(destination, "w");
	if (not file) { perror("open"); exit(3); }
	fwrite(&player_count, 4, 1, file);
	fwrite(players, sizeof(struct player), player_count, file);
	fclose(file);
}

static inline void load_state(const char* source) {
	FILE* file = fopen(source, "r");
	if (not file) { perror("fopen"); exit(3); }
	fseek(file, 0, SEEK_END);
	count = (u64) ftell(file);
	s = square_root(count);
	fseek(file, 0, SEEK_SET);
	universe = malloc(count);	
	fread(universe, 1, count, file);
	fclose(file);
	printf("loaded %llu bytes of existing universe (s=%llu)...\n", count, s);
}

static inline void load_players(const char* source) {
	FILE* file = fopen(source, "r");
	if (not file) { perror("fopen"); exit(3); }
	fread(&player_count, 4, 1, file);
	fread(players, sizeof (struct player), player_count, file);
	fclose(file);
	printf("loaded %d player's data.\n", player_count);
}

static inline void generate() {
	count = s * s;
	universe = malloc(count);
	printf("debug: generating universe of %llu bytes ...\n", count);

	// for (u64 i = 0; i < count; i++) universe[i] = 0;
	for (u64 i = 0; i < count; i++) universe[i] = (rand() % 2) * (rand() % 2) * (rand() % 2);
}

static inline void halt_server() {
	printf("SERVER: halting...\n");
	server_running = false;
	shutdown(server, SHUT_RDWR);
	close(server);
}

static inline void spawn_player(u32 p) {
	u32 x = 0, y = 0;

	printf("spawning player...\n");

	for (u32 t = 0; t < attempt_count; t++) {
		x = (u32)rand() % (u32)s;
		y = (u32)rand() % (u32)s;
		for (i64 i = -2; i <= 2; i++) {
			for (i64 j = -2; j <= 2; j++) {
				if (at(x,y,i,j)) goto next;
			}
		}
		players[p].x = x;
		players[p].y = y;
		return; next: continue;
	}
	printf("error: could not spawn player after %d attempts, spawning at 0,0...\n", attempt_count);
	players[p].x = 0;
	players[p].y = 0;
}

static inline u32 find_player(const char* player_name) {
	u32 p = 0;
	for (; p < player_count; p++) {
		if (not strcmp(players[p].name, player_name)) {
			if (not players[p].active) {
				players[p].active = true;
				break;
			} else check(0);
		}
	}

	if (p == player_count) {
		strncpy(players[p].name ,player_name, 30);
		spawn_player(p);
		players[p].hand = 0;
		players[p].active = 1;
		player_count++;
	}
	return p;
}

static void* client_handler(void* raw) {

	printf("[arrived in client handler function.]\n");

	ssize_t n = 0;
	u8 command = 0, ack = 1, response = 0;
	char player_name[32] = {0};

	struct client parameters = *(struct client*)raw;
	int client = parameters.connection;
	const char* ip = parameters.ip;
	u16 port = parameters.port;

	n = read(client, &player_name, 29);
	check(n); if (n == 29) write(client, &ack, 1); else goto leave;

	parameters.player = find_player(player_name);
	struct player* player = players + parameters.player;

	printf("server: connected to IP = %s:%d :  player name = \"%s\"\n", ip, port, player->name);

	n = read(client, &player->width, 2); check(n); 	
	n = read(client, &player->height, 2); check(n);
	write(client, &ack, 1); 

	printf("server: screen size: w=%d, h=%d\n", player->width, player->height);
	if (not player->height or not player->width) abort();

	u32 screen_block_count = 0;
	const u32 max_block_count = 10000000;
	u16* screen = malloc(max_block_count * 2);

	while (server_running) {

		// printf("server: debug: waiting for command...\n");
		n = read(client, &command, 1);
		if (n == 0) { printf("{CLIENT DISCONNECTED}\n"); break; } 
		check(n);
		// printf("[received command --> %d]\n", command);

		if (command == halt) {
			write(client, &ack, 1);
			halt_server();
			continue;

		} else if (command == move_left) {
			write(client, &ack, 1);
			if (not at(player->x, player->y, -1, 0)) 
				player->x = (player->x + s - 1) % s;
			// printf("debug: moving player to the left... now, at (x=%llu,y=%llu)\n", player->x, player->y);

		} else if (command == move_right) {
			write(client, &ack, 1);
			if (not at(player->x, player->y, 1, 0)) 
				player->x = (player->x + 1) % s;
			// printf("debug: moving player to the right... now, at (x=%llu,y=%llu)\n", player->x, player->y);

		} else if (command == move_up) {
			write(client, &ack, 1);
			if (not at(player->x, player->y, 0, -1)) 
				player->y = (player->y + s - 1) % s;
			// printf("debug: moving player to upwards... now, at (x=%llu,y=%llu)\n", player->x, player->y);

		} else if (command == move_down) {
			write(client, &ack, 1);
			if (not at(player->x, player->y, 0, 1)) 
				player->y = (player->y + 1) % s;
			// printf("debug: moving player to downwards... now, at (x=%llu,y=%llu)\n", player->x, player->y);

		} else if (command == view_resized) {
			
			n = read(client, &player->width, 2); check(n);
			n = read(client, &player->height, 2); check(n); 
			write(client, &ack, 1);
			// printf("server: screen resized: w=%d, h=%d\n", player->width, player->height);
			if (not player->height or not player->width) {
				printf("error: window cannot be null sized! closing client connection.\n");
				goto leave;
			}

		} else if (command == display) {
			
			screen_block_count = 0;
			
			int width_radius = (player->width - 1) >> 1;
			int height_radius = (player->height - 1) >> 1;

			for (int i = 0, x_off = -width_radius; i < player->width; i++, x_off++) {
				for (int j = 0, y_off = -height_radius; j < player->height; j++, y_off++) {
					if (screen_block_count >= max_block_count - 1) goto screen_full;
					if (at(player->x, player->y, x_off, y_off)) {
						screen[screen_block_count++] = (u16) i;
						screen[screen_block_count++] = (u16) j;
					}
				}
			}

		screen_full:
			// pad zeros for last packet:
			for (u32 i = screen_block_count; i % 64; i++) screen[i] = 0;
			// printf("sending %d coords,(2-per-block)...\n", screen_block_count);
			write(client, &screen_block_count, sizeof(u32));	

			n = read(client, &response, 1);
			check(n); // if (response != 1) not_acked();
			
			u32 local_count = 0;
			while (local_count < screen_block_count) {
				write(client, screen + local_count, 64 * sizeof(u16));
				local_count += 64;
			}

			n = read(client, &response, 1);
			check(n); // if (response != 1) not_acked();
	
		} else printf("error: command not recognized:  %d\n", (int) command);

	}

	player->active = false;

leave:
	printf("debug: closing control connection...\n");
	close(client); 
	// free(screen);
	free(raw);
	return 0;
}

static void* compute(void* _) {
	printf("computing world thread...\n");
	while (server_running) {
		// printf("universe ticked\n");
		sleep(1);
	}
	return _;
}

int main(const int argc, const char** argv) {
	if (argc < 4) exit(puts( "usage: ./server <port> <s> <universe>\n[s?generate(s):load(universe)]"));
	
	srand((unsigned)time(0));
	u16 port = (u16) atoi(argv[1]);
	s = (u64) atoll(argv[2]);

	char players_file[128] = {0};
	strncpy(players_file, argv[3], 127);
	strncat(players_file, "/players.blob", 127);

	char state_file[128] = {0};
	strncpy(state_file, argv[3], 127);
	strncat(state_file, "/state.blob", 127);
	
	players = malloc(max_player_count * sizeof(struct player));

	if (s) { mkdir(argv[3], 0700); generate(); }
	else { load_state(state_file); load_players(players_file); }

	pthread_t thread;
	pthread_create(&thread, NULL, compute, NULL);
	
	printf("creating socket...\n");
	server = socket(AF_INET, SOCK_STREAM, 0);
	if (server < 0) { perror("socket"); exit(1); }
	setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int[]){1}, sizeof(int));
	setsockopt(server, SOL_SOCKET, SO_REUSEPORT, &(int[]){1}, sizeof(int));
	struct sockaddr_in server_address = {0}, client_address = {0};
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(port);
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	socklen_t client_length = sizeof client_address;
	int bind_result = bind(server, (struct sockaddr*) &server_address, sizeof server_address);
	if (bind_result < 0) { perror("bind"); exit(1); }
	listen(server, 5);

	while (server_running) {
		printf("server: listening for clients on port %hd...\n", port);
		int connection = accept(server, (struct sockaddr*) &client_address, &client_length);
		if (connection < 0) break;
		struct client* client = malloc(sizeof(struct client));
		client->connection = connection;
		client->ip = inet_ntoa(client_address.sin_addr);
		client->port = (u16)port;
		pthread_t handler_thread;
		printf("starting handler thread for connection...\n");
		pthread_create(&handler_thread, NULL, client_handler, client);
		pthread_detach(handler_thread);
		usleep(10000);
	}

	pthread_join(thread, NULL);
	printf("saving universe...\n");
	save_state(state_file);
	save_players(players_file);
}












// ---------------------------------------------------- dead code ------------------------------------------------------------



