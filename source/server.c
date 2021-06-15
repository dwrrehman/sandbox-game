// example udp server.
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

static u16 port = 0;
static int server = 0;
static bool server_running = true;

#define check(n) { if (n == 0 || n < 0) printf("error(%ld): %s line:%d func:%s\n", n, __FILE__, __LINE__, __func__); }

static inline void ipv6_string(char buffer[40], u8 ip[16]) {
	sprintf(buffer,
	"%02hhx%02hhx:%02hhx%02hhx:" "%02hhx%02hhx:%02hhx%02hhx:"
	"%02hhx%02hhx:%02hhx%02hhx:" "%02hhx%02hhx:%02hhx%02hhx",
	ip[0], ip[1], ip[2], ip[3],  ip[4], ip[5], ip[6], ip[7], 
	ip[8], ip[9], ip[10], ip[11], ip[12], ip[13], ip[14], ip[15]);
}

static inline void initialize_server_socket() {
	server = socket(PF_INET6, SOCK_DGRAM, 0);
	if (server < 0) { perror("socket"); abort(); }

	struct sockaddr_in6 server_address = {0};
	server_address.sin6_family = PF_INET6;
	server_address.sin6_port = htons(port);
	server_address.sin6_addr = in6addr_any;
	
	int result = bind(server, (struct sockaddr*) &server_address, sizeof server_address);
	if (result < 0) { perror("bind"); abort(); }

	char ip[40] = {0};
	ipv6_string(ip, server_address.sin6_addr.s6_addr);

	printf("info: server running on  [%s]:%hu \n", ip, port);
}

static void* compute(void* _) {
	printf("computing world thread...\n");
	while (server_running) {
		printf("universe ticked\n");
		sleep(5);
	}
	return _;
}

int main(const int argc, const char** argv) {

	if (argc < 2) return puts("usage: <port>");

	port = (u16) atoi(argv[1]);
	if (port < 1024) port = 12000;

	initialize_server_socket();
	pthread_t compute_thread;
	pthread_create(&compute_thread, NULL, compute, NULL);
	printf("server: listening on %hu...\n", port);

	while (server_running) {
		
		char ip[40] = {0};
		u8 command = 0; // response = 0;
		
		struct sockaddr_in6 address = {0};
		socklen_t length = sizeof address;

		ssize_t error = recvfrom(server, &command, 1, 0, (struct sockaddr*)&address, &length);
		if (error == 0) { printf("CLIENT DISCONNECTED! i think..\n"); abort(); }
		else check(error);

		ipv6_string(ip, address.sin6_addr.s6_addr);

		printf("client [%s]: --> they said: %c\n", ip, command);

		// error = sendto(server, "@", 1, 0, (struct sockaddr*)&address, length);
		// check(error);

		if (command == 'N') { 
			printf("client[%s] : ", ip);
			printf("no operation.\n");
			error = sendto(server, "A", 1, 0, (struct sockaddr*)&address, length);
			check(error);
		}


		else if (command == 'C') { 
			printf("client[%s] : ", ip);
			printf("server: they connected to server!\n");
			error = sendto(server, "A", 1, 0, (struct sockaddr*)&address, length);
			check(error);
		}

		else if (command == 'H') { 
			printf("client[%s] : ", ip);
			printf("SERVER: halting...\n"); 
			error = sendto(server, "A", 1, 0, (struct sockaddr*)&address, length);
			check(error);

			server_running = false;
			shutdown(server, SHUT_RDWR); 
			close(server);
		}

		else if (command == 'D') { 
			printf("client[%s] : ", ip);
			printf("info: client sent a disconnection request. disconnecting them...\n"); 
			error = sendto(server, "A", 1, 0, (struct sockaddr*)&address, length);
			check(error);
		}

		else if (command == 'P') {
			printf("client[%s] : ", ip);
			printf("server was PINGED!!!\n");
			error = sendto(server, "A", 1, 0, (struct sockaddr*)&address, length);
			check(error);
		} 

		else {
			printf("client[%s] : ", ip);
			printf("warning: received unknown commmand: %c\n", command);
			error = sendto(server, "A", 1, 0, (struct sockaddr*)&address, length);
			check(error);
		}
	}

	pthread_join(compute_thread, NULL);
	close(server);
}
























// setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int[]){1}, sizeof(int));
  	// setsockopt(server, SOL_SOCKET, SO_REUSEPORT, &(int[]){1}, sizeof(int));



// setsockopt(connection, SOL_SOCKET, SO_REUSEADDR, &(int[]){1}, sizeof(int));
  	// setsockopt(connection, SOL_SOCKET, SO_REUSEPORT, &(int[]){1}, sizeof(int));




	// int connection = server;
	// int connection = socket(PF_INET6, SOCK_DGRAM, 0);
	// if (connection < 0) { perror("socket"); abort(); }

	// int result = bind(connection, (struct sockaddr*) &address, length);
	// if (result < 0) { perror("bind"); abort(); }

	// printf("info: client-hanlder's connection running on  [%s]:%hu \n", ip, port);






// static void* client_handler(void* raw) {

// 	struct client_data parameters = *(struct client_data*)raw;
// 	struct sockaddr_in6 address = parameters.address;
// 	socklen_t length = parameters.length;

// 	u8 command = 0;

// 	char ip[40] = {0};
// 	ipv6_string(ip, address.sin6_addr.s6_addr);
// 	printf("server: connected to IP = %s\n", ip);

// 	bool client_running = true;

// 	while (server_running and client_running) {
		
// 		printf("client[%s] : ", ip);
// 		printf("reading command...\n");
		
// 		ssize_t error = recvfrom(server, &command, 1, 0, (struct sockaddr*)&address, &length);		
// 		if (error == 0) { printf("DISCONNECTED!\n"); break; }
// 		else check(error);

// 		if (command == 'N') { 
// 			printf("client[%s] : ", ip);
// 			printf("no operation.\n");
// 			error = sendto(server, "A", 1, 0, (struct sockaddr*)&address, length);
// 			check(error);
// 		}

// 		else if (command == 'H') { 
// 			printf("client[%s] : ", ip);
// 			printf("SERVER: halting...\n"); 
// 			error = sendto(server, "A", 1, 0, (struct sockaddr*)&address, length);
// 			check(error);

// 			server_running = false;
// 			shutdown(server, SHUT_RDWR); 
// 			close(server);
// 		}

// 		else if (command == 'D') { 
// 			printf("client[%s] : ", ip);
// 			printf("info: client sent a disconnection request.\n"); 
// 			error = sendto(server, "A", 1, 0, (struct sockaddr*)&address, length);
// 			check(error);
// 			client_running = false; 
// 		}

// 		else if (command == 'P') {
// 			printf("client[%s] : ", ip);
// 			printf("server was PINGED!!!\n");
// 			error = sendto(server, "A", 1, 0, (struct sockaddr*)&address, length);
// 			check(error);
// 		} 

// 		else {
// 			printf("client[%s] : ", ip);
// 			printf("warning: received unknown commmand: %c\n", command);
// 			error = sendto(server, "A", 1, 0, (struct sockaddr*)&address, length);
// 			check(error);
// 		}
// 	}

// 	printf("client[%s] : ", ip);
// 	printf("debug: leaving client handler...\n");
// 	free(raw);
// 	return 0;
// }





		// if (response != 'C') {
		// 	printf("intercepted random message... dropping...\n");
		// 	continue;
		// }




		// struct client_data* client_data = malloc(sizeof(struct client_data));
		// client_data->address = address;
		// client_data->length = len;
		
		// printf("starting handler thread for connection...\n");
		// pthread_t handler_thread;
		// pthread_create(&handler_thread, NULL, client_handler, client_data);
		// pthread_detach(handler_thread);
		// usleep(10000);

	// 	struct client_data parameters = *(struct client_data*)raw;
	// struct sockaddr_in6 address = parameters.address;
	// socklen_t length = parameters.length;





		// ipv6_string(ip, address.sin6_addr.s6_addr);
		// printf("server: connected to IP = %s\n", ip);

			// printf("client[%s] : ", ip);
			// printf("reading command...\n");
			
			// ssize_t error = recvfrom(server, &command, 1, 0, (struct sockaddr*)&address, &length);		
			// if (error == 0) { printf("DISCONNECTED!\n"); break; }
			// else check(error);






// struct client_data {
// 	struct sockaddr_in6 address;
// 	socklen_t length;
// };

