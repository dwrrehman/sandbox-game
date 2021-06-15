// example udp client.

#include <iso646.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>

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

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

#define check(n) { if (n == 0 || n < 0) printf("error(%ld): %s line:%d func:%s\n", n, __FILE__, __LINE__, __func__); }

int main(const int argc, const char** argv) {
	if (argc < 3) return puts("usage: <ip> <port>");

	const char* ip = argv[1];
	const u16 port = (u16) atoi(argv[2]);
		
	int fd = socket(PF_INET6, SOCK_DGRAM, 0);
	if (fd < 0) { perror("socket"); abort(); }

	struct sockaddr_in6 address = {0};
	address.sin6_family = PF_INET6;
	address.sin6_port = htons(port);
	inet_pton(PF_INET6, ip, &address.sin6_addr); 
	socklen_t size = sizeof address;
	
	u8 command = 0, response = 0;

	command = 'C';
	ssize_t error = sendto(fd, &command, 1, 0, (struct sockaddr*) &address, size);
	check(error);

	printf("Connecting to [%s]:%hd ...\n", ip, port);

	error = recvfrom(fd, &response, 1, 0, (struct sockaddr*) &address, &size); 
	check(error);

	printf("connection response = %c\n", response);

	printf(":client:>> ");

	bool client_running = true;

	while (client_running) {

		int c = getchar();

		if (c == 10) continue;
		if (c == 'D' or c == 'H') client_running = false;

		command = (u8) c;

		error = sendto(fd, &command, 1, 0, (struct sockaddr*) &address, size);
		check(error);

		error = recvfrom(fd, &response, 1, 0, (struct sockaddr*) &address, &size); 
		check(error);

		printf("response = %c\n", response);
		printf(":client:>> ");
	}
	printf("client: terminating...\n");
	close(fd);
}



// #include <string.h>
// #include <stdint.h>
// #include <stdnoreturn.h>
// #include <pthread.h>
// #include <math.h>
// #include <time.h>
// #include <netdb.h>
// #include <netinet/in.h>

// #include <sys/socket.h>
// #include <sys/types.h>
// #include <sys/wait.h>
// #include <sys/stat.h>


