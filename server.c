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

static const u32 max_block_count = 1 << 16;

static const u8 ack = 1;

enum commands {	
	null_command = 0,
	display = 9, 
	window_resized = 6,
	move_right = 13,
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

#define not_acked() \
	{do { \
		printf("debug: error: command not acknowledged from client file:%s line:%d func:%s \n", __FILE__, __LINE__, __func__); \
		abort(); \
	} while(0);} \

#define disconnected() \
	{do { \
		printf("debug: error: disconnected:%s line:%d func:%s \n", __FILE__, __LINE__, __func__); \
		abort(); \
	} while(0);} \



static inline void check(ssize_t n) {
	if (n == 0) { disconnected(); }
	else if (n < 0) { read_error(); }
}



static inline void show() {
	printf("\nstate:  s = %llu, count = %llu\n\n", s, count);
	printf("{ \n");
	for (u64 i = 0; i < count; i++) {
		if (i and (i % 8) == 0) puts("");
		printf("%02hhx ", universe[i]);
	}
	printf("}\n");
}

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
}

static inline void load_players(const char* source) {
	FILE* file = fopen(source, "r");
	if (not file) { perror("fopen"); exit(3); }
	fread(&player_count, 4, 1, file);
	fread(players, sizeof (struct player), player_count, file);
	fclose(file);
}

static inline void generate(const char* base) {
	count = s * s;
	universe = malloc(count);
	printf("debug: generating universe of %llu bytes ...\n", count);

	for (u64 i = 0; i < count; i++) universe[i] = 0;
	universe[2] = 5;


	mkdir(base, 0700);
}

static inline void halt_server() {
	printf("SERVER: halting...\n");
	server_running = false;
	shutdown(server, SHUT_RDWR);
	close(server);
}

static inline void spawn_player(u32 p) {
	u32 x = 0, y = 0;
	
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



static void* display_client_handler(void* raw) {

	ssize_t n = 0;
	u8 response = 0;

	struct client parameters = *(struct client*)raw;

	struct player* player = players + parameters.player;
	const char* ip = parameters.ip;
	u16 port = parameters.port;

	printf("DISPLAY HANDLER: connected to  %s : %d,   player # %d.\n", ip, port + 1, parameters.player);
	
	u32 screen_block_count = 0;
	u16* screen = malloc(max_block_count * 2);

	int udp_connection = socket(AF_INET, SOCK_DGRAM, 0);
	if (!udp_connection) { perror("socket"); abort(); }

	struct sockaddr_in servaddr, cliaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port + 1);
	servaddr.sin_family = AF_INET;
	socklen_t len = sizeof(cliaddr);
	bind(udp_connection, (struct sockaddr*) &servaddr, sizeof(servaddr));
	printf("debug: display client handler: setup udp server on port %d\n", port + 1);

	printf("debug: display client handler: waiting for client to sendto ACK first...\n");
	n = recvfrom(udp_connection, &response, 1, 0, (struct sockaddr*)&cliaddr, &len);
	check(n); if (response != 1) not_acked();
	printf("debug: display client handler: DONE! received.\n");

	while (player->active) {
		
		screen_block_count = (rand() % 30) * 2;

		for (u32 i = 0; i < screen_block_count; i += 2) {
			screen[i] = rand() % player->width;
			screen[i + 1] = rand() % player->height;
		}
		
		printf("debug: sending DP with %d blocks...\n", screen_block_count);

		sendto(udp_connection, &screen_block_count, 4, 0, (struct sockaddr*)&cliaddr, len);

		n = recvfrom(udp_connection, &response, 1, 0, (struct sockaddr*)&cliaddr, &len);
		check(n); if (response != 1) not_acked();

		sendto(udp_connection, screen, screen_block_count * 2, 0, (struct sockaddr*)&cliaddr, len);

		n = recvfrom(udp_connection, &response, 1, 0, (struct sockaddr*)&cliaddr, &len);
		check(n); if (response != 1) not_acked();

	}
	close(udp_connection);
	free(screen);
	return 0;
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

	ssize_t n = 0;
	u8 command = 0;
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

	pthread_t display_handler_thread;
	pthread_create(&display_handler_thread, NULL, display_client_handler, &parameters);

	while (server_running) {

		printf("server: debug: waiting for command...\n");
		n = read(client, &command, 1);
		if (n == 0) { printf("{CLIENT DISCONNECTED}\n"); break; } 
		check(n);

		if (command == halt) {
			write(client, &ack, 1);
			halt_server();
			continue;

		} else if (command == move_right) {
			
			write(client, &ack, 1);
			player->x++;
			printf("debug: moving player to the right... now, at (x=%llu,y=%llu)\n", player->x, player->y);

		} else if (command == window_resized) {
			
			n = read(client, &player->width, 2); check(n);
			n = read(client, &player->height, 2); check(n); 
			write(client, &ack, 1); 
			printf("server: screen resized: w=%d, h=%d\n", player->width, player->height);
			if (not player->height or not player->width) abort();

		} else printf("error: command not recognized:  %d\n", (int) command);

	}
	player->active = false;
	pthread_join(display_handler_thread, NULL);

leave:
	close(client); 
	free(raw);
	return 0;
}

static void* compute(void* __attribute__((unused)) unused) {
	printf("computing world thread...\n");
	while (server_running) {
		printf("universe ticked\n");
		sleep(10);
	}
	return 0;
}


int main(const int argc, const char** argv) {
	if (argc < 4) exit(puts( "usage: \n\t./server <s> <port> <universe>\n"));
	srand((unsigned)time(0));

	s = (u64) atoll(argv[1]);
	u16 port = (u16) atoi(argv[2]);

	char players_file[128] = {0};
	strncpy(players_file, argv[3], 127);
	strncat(players_file, "/players.blob", 127);

	char state_file[128] = {0};
	strncpy(state_file, argv[3], 127);
	strncat(state_file, "/state.blob", 127);

	players = malloc(max_player_count * sizeof(struct player));
	if (s) generate(argv[3]); else { load_state(state_file); load_players(players_file); }
	show();

	pthread_t thread;
	pthread_create(&thread, NULL, compute, NULL);
	
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









































	// for (u64 i = 0, a = 0; i < 100; a = i * i, i++) 
	// 	printf("\rsqrt(%llu) = %llu\n", a, square_root(a));

	// exit(1);




                        
        //wprintf("SERVER:> ");
        //fgets(buffer, sizeof buffer, stdin);
        //n = write(client, buffer, sizeof buffer);
        //if ( n == 0) {
        //    printf("server:write disconnected.\n");
        //    break;
        //}
        //if (!strcmp(buffer, "quit\n")) break;    






/*
 void* handler(void* raw) {
     struct client parameters = *(struct client*)raw;
     int client = parameters.connection;
     const char* ip = parameters.ip;
     
     printf("connected to %s\n", ip);
     
     char buffer[1000] = {0};
     ssize_t n = 0;
     while (server_running) {
         
         memset(buffer, 0, sizeof buffer);
         n = read(client, buffer, sizeof buffer);
         if ( n == 0) {
             printf("server:read disconnected.\n");
             printf("{CLIENT DISCONNECTED}\n");
             break;
         }
         printf("client says: %s\n", buffer);
         
         if (!strcmp(buffer, "halt\n")) {
             printf("halting server...\n");
             halt_server(); continue;
         }
         
         printf("SERVER:> ");
         fgets(buffer, sizeof buffer, stdin);
         n = write(client, buffer, sizeof buffer);
         if ( n == 0) {
             printf("server:write disconnected.\n");
             break;
         }
         if (!strcmp(buffer, "quit\n")) break;
     }
     close(client);
     free(raw);
     return NULL;
 }
 
 */






/*static void connect_to_udp_client(unsigned int port) {

    int connection = socket(AF_INET, SOCK_DGRAM, 0);
    if (!connection) { perror("socket"); return; }
    
    struct sockaddr_in servaddr, cliaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    servaddr.sin_family = AF_INET;
    socklen_t len = sizeof(cliaddr);
    bind(connection, (struct sockaddr*) &servaddr, sizeof(servaddr));
    
    printf("listening for UDP clients...\n");
    
    char buffer[1000] = {0};
        
    while (server_running) {

        memset(buffer, 0, sizeof buffer);
        ssize_t n = recvfrom(connection, buffer, sizeof buffer, 0, (struct sockaddr*)&cliaddr, &len);
        if (n == 0) {
            printf("{UDP CLIENT DISCONNECTED}\n");
            break;
        }
        printf("UDP client says: %s\n", buffer);
        
        if (!strcmp(buffer, "halt\n")) {
            printf("halting UDP server...\n");
            halt_server(); continue;
        }

        printf("UDPSERVER:> ");
        fgets(buffer, sizeof buffer, stdin);
        if (!strcmp(buffer, "quit\n")) break;
        sendto(connection, buffer, strlen(buffer), 0, (struct sockaddr*)&cliaddr, len);
    }
    close(connection);
}
*/








// connect_to_udp_client(port); // ?





// static inline noreturn void not_acked() {
// 	printf("debug: error: command not acknowledged from client\n");
// 	abort();
// }




		// } else if (command == ping) {
		// 	printf("SERVER WAS PINGED!!!\n");
		// 	write(client, &ack, 1);

		// } else if (command == chat) {

		// 	memset(buffer, 0, sizeof buffer);
		// 	n = read(client, buffer, sizeof buffer);
		// 	if (n == 0) { printf("{CLIENT DISCONNECTED}\n"); break; } 
		// 	else if (n < 0) { read_error(); break; }
		// 	printf("client said chat message: %s\n", buffer);
		// 	strcat(transcript, buffer);
		// 	write(client, &ack, 1);









// n = read(client, &response, 1);
			// if (n == 0) { printf("{CLIENT DISCONNECTED}\n"); break; } 
			// else if (n < 0) { read_error(); break; }
			// if (response != 1) not_acked();




	// while (server_running) {

//		ssize_t n = recvfrom(udp_connection, buffer, sizeof buffer, 0, (struct sockaddr*)&cliaddr, &len);
		// if (n == 0) { disconnected(); }

		// printf("UDP client says: %s\n", buffer);

		// if (!strcmp(buffer, "halt\n")) {
		// 	printf("halting UDP server...\n");
		// 	halt_server(); continue;
		// }

		// printf("UDPSERVER:> ");
		// fgets(buffer, sizeof buffer, stdin);
		// if (!strcmp(buffer, "quit\n")) break;

//		sendto(udp_connection, buffer, strlen(buffer), 0, (struct sockaddr*)&cliaddr, len);
	// }



