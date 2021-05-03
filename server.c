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

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

enum commands {	
	null_command = 0,
	ping = 5, 
	display = 9, 
	chat = 13, 
	halt = 255, 
};

struct client {
	const char* ip;
	int connection;
	int _padding0;
};

static bool server_running = true;
static int server = 0;
static char transcript[256] = {0}; // temp

static u64 s = 0;
static u64 count = 0;
static u8* universe = NULL;

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

static inline void save(const char* destination) {
	FILE* file = fopen(destination, "w");
	if (not file) { perror("open"); exit(3); }
	fwrite(universe, 1, count, file);
	fclose(file);
}

static inline void load(const char* source) {
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

static inline void show() {
	printf("\nstate:  s = %llu, count = %llu\n\n", s, count);
	printf("{ \n");
	for (u64 i = 0; i < count; i++) {
		if (i and (i % 8) == 0) puts("");
		printf("%02hhx ", universe[i]);
	}
	printf("}\n");
}

static inline void generate() {
	count = s * s;
	universe = malloc(count);
	printf("debug: generating universe of %llu bytes ...\n", count);

	for (u64 i = 0; i < count; i++) universe[i] = 0;
	universe[2] = 5;

}

static inline void halt_server() {
	printf("SERVER: halting...\n");
	server_running = false;
	shutdown(server, SHUT_RDWR);
	close(server);
}


static inline void read_error() {
	printf("debug: server: read error! (n < 0)\n");
	return;
}

static void* client_handler(void* raw) {
	struct client parameters = *(struct client*)raw;
	int client = parameters.connection;
	const char* ip = parameters.ip;
	const u8 ack = 1;
	printf("server: connected to IP = %s\n", ip);

	char buffer[256] = {0};
	while (server_running) {

		u8 command = 0;
		ssize_t n = read(client, &command, 1);
		if (n == 0) { printf("{CLIENT DISCONNECTED}\n"); break; } 
		else if (n < 0) { read_error(); break; }

		if (command == halt) {
			write(client, &ack, 1);
			halt_server();
			continue;

		} else if (command == ping) {
			printf("SERVER WAS PINGED!!!\n");
			write(client, &ack, 1);

		} else if (command == chat) {

			memset(buffer, 0, sizeof buffer);
			n = read(client, buffer, sizeof buffer);
			if (n == 0) { printf("{CLIENT DISCONNECTED}\n"); break; } 
			else if (n < 0) { read_error(); break; }
			printf("client said chat message: %s\n", buffer);
			strcat(transcript, buffer);
			write(client, &ack, 1);

		} else if (command == display) write(client, transcript, sizeof transcript);

		else printf("error: command not recognized:  %d\n", (int) command);
		usleep(2000);
	}
	close(client); 
	free(raw);
	return 0;
}

static void* compute(void* __attribute__((unused)) unused) {
	printf("computing world thread...\n");
	while (server_running) {
		printf("universe ticked\n");
		sleep(5);
	}
	return 0;
}


int main(const int argc, const char** argv) {
	if (argc < 4) exit(puts( "usage: \n\t./server <s> <port> <file>\n"));
	s = (u64) atoll(argv[1]);
	i16 port = (i16) atoi(argv[2]);
	if (s) generate(); else load(argv[3]);
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
		pthread_t handler_thread;
		pthread_create(&handler_thread, NULL, client_handler, client);
		pthread_detach(handler_thread);
		usleep(10000);
	}

	pthread_join(thread, NULL);
	printf("saving universe...\n");
	save(argv[3]);
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

