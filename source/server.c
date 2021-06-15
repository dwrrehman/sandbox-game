// example udp server.
#include <iso646.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

enum commands {
	null = 0, 
	ack = 'A',
	halt = 'H',
	connect_request = 'C',
	disconnect_request = 'D',
	display = 'G',
	move_up = 'w',
	move_down = 's',
	move_left = 'a',
	move_right = 'd',
	unknown = 255,
};

static bool server_running = true;

#define check(n) { if (n == 0 or n < 0) printf("error(%ld): in file:%s line:%d func:%s\n", n, __FILE__, __LINE__, __func__); }

static inline void ipv6_string(char buffer[40], u8 ip[16]) {
	sprintf(buffer,
	"%02hhx%02hhx:%02hhx%02hhx:" "%02hhx%02hhx:%02hhx%02hhx:"
	"%02hhx%02hhx:%02hhx%02hhx:" "%02hhx%02hhx:%02hhx%02hhx",
	ip[0], ip[1], ip[2], ip[3],  ip[4], ip[5], ip[6], ip[7], 
	ip[8], ip[9], ip[10], ip[11], ip[12], ip[13], ip[14], ip[15]);
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
	u16 port = (u16) atoi(argv[1]);
	if (port < 1024) port = 12000;

	int server = socket(PF_INET6, SOCK_DGRAM, 0);
	if (server < 0) { perror("socket"); abort(); }
	struct sockaddr_in6 server_address = {0};
	server_address.sin6_family = PF_INET6;
	server_address.sin6_port = htons(port);
	server_address.sin6_addr = in6addr_any;
	int result = bind(server, (struct sockaddr*) &server_address, sizeof server_address);
	if (result < 0) { perror("bind"); abort(); }

	pthread_t compute_thread;
	pthread_create(&compute_thread, NULL, compute, NULL);

	char ip[40] = {0};
	u8 command = 0;	
	struct sockaddr_in6 address = {0};
	socklen_t length = sizeof address;

	printf("server: listening on %hu...\n", port);
	while (server_running) {
		
		ssize_t error = recvfrom(server, &command, 1, 0, (struct sockaddr*)&address, &length);
		check(error);
		ipv6_string(ip, address.sin6_addr.s6_addr);

		printf("client[%s] : ", ip);
		if (command == 0) printf("Null command.\n");
		else if (command == 'N') printf("no operation.\n");
		else if (command == 'C') printf("server: they connected to server!\n");
		else if (command == 'D') printf("info: client sent a disconnection request!\n"); 
		else if (command == 'P') printf("server was PINGED!!!\n");
		else if (command == 'H') {
			printf("SERVER: halting...\n"); 
			error = sendto(server, "A", 1, 0, (struct sockaddr*)&address, length);
			check(error);
			if (shutdown(server, SHUT_RDWR) < 0) perror("shutdown");	
			if (close(server) < 0) perror("shutdown");
			server_running = false;
	 		break;

		} else printf("warning: received unknown commmand: %c\n", command);

		error = sendto(server, "A", 1, 0, (struct sockaddr*)&address, length);
		check(error);
	}
	pthread_join(compute_thread, NULL);
}




// #include <string.h>
// #include <stdnoreturn.h>
// #include <stdint.h>
// #include <math.h>
// #include <time.h>
// #include <netdb.h>
// #include <netinet/in.h>

// #include <sys/socket.h>
// #include <sys/types.h>
// #include <sys/wait.h>
// #include <sys/stat.h>

