// example udp client.
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

static const char* ip = "2601:1c2:4001:ecf0:dc6a:af3e:eebb:40ca";
static const u16 port = 12000;

#define check(n) { if (n == 0 || n < 0) printf("error(%ld): %s line:%d func:%s\n", n, __FILE__, __LINE__, __func__); }

int main(void) {

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

	command = 0;
	error = recvfrom(fd, &command, 1, 0, (struct sockaddr*) &address, &size); 
	check(error);

	printf("connection response = %c\n", command);

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

	close(fd);
}
