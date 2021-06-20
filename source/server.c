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
	ack = 'A',
	halt = 'H',
	connect_request = 'C',
	disconnect_request = 'D',
	display_command = 'G',
	move_up = 'w',
	move_down = 's',
	move_left = 'a',
	move_right = 'd',
	unknown = 255,
};

struct player {
	u64 id0;
	u64 id1;
	u64 x;
	u64 y;
	socklen_t length;
	struct sockaddr_in6 address;
};

static u8* universe = NULL;
static u64 universe_count = 0;

static struct player players[5];
static u32 player_count = 0;

static int server = 0;
static bool server_running = true;

#define check(n) { if (n == 0 or n < 0) printf("error(%ld): in file:%s line:%d func:%s\n", n, __FILE__, __LINE__, __func__); }

static inline void ipv6_string(char buffer[40], u8 ip[16]) {
	sprintf(buffer,
	"%02hhx%02hhx:%02hhx%02hhx:" "%02hhx%02hhx:%02hhx%02hhx:"
	"%02hhx%02hhx:%02hhx%02hhx:" "%02hhx%02hhx:%02hhx%02hhx",
	ip[0], ip[1], ip[2], ip[3],  ip[4], ip[5], ip[6], ip[7], 
	ip[8], ip[9], ip[10], ip[11], ip[12], ip[13], ip[14], ip[15]);
}

static void generate() {
	universe = calloc(100, 1);
	universe_count = 100;
}

static void* compute(void* _) {
	printf("in compute thread!\n");

	while (server_running) {

		// we need to cache the player state here.

		for (u32 player = 0; player < player_count; player++) {

			u8 packet[4] = {1,4,5,7};
			const size_t packet_size = 4;

			ssize_t error = sendto(server, packet, packet_size, 0, (struct sockaddr*)& (players[player].address), players[player].length);
			check(error);

		}

		// then we need to update our cache...?


		sleep(1);
	}

	return _;
}

int main(const int argc, const char** argv) {

	if (argc < 2) return puts("usage: <port>");
	u16 port = (u16) atoi(argv[1]);
	if (port < 1024) port = 12000;

	server = socket(PF_INET6, SOCK_DGRAM, 0);
	if (server < 0) { perror("socket"); abort(); }
	struct sockaddr_in6 server_address = {0};
	server_address.sin6_family = PF_INET6;
	server_address.sin6_port = htons(port);
	server_address.sin6_addr = in6addr_any;

	int result = bind(server, (struct sockaddr*) &server_address, sizeof server_address);
	if (result < 0) { perror("bind"); abort(); }

	generate();

	pthread_t thread;
	pthread_create(&thread, NULL, compute, NULL);

	u8 command = 0;	
	char ip[40] = {0};
	struct sockaddr_in6 address = {0};
	socklen_t length = sizeof address;

	printf("server: listening on %hu...\n", port);

	while (server_running) {
		
		ssize_t error = recvfrom(server, &command, 1, 0, (struct sockaddr*)&address, &length);
		check(error);

		ipv6_string(ip, address.sin6_addr.s6_addr); // put in connect requ.
		printf("client[%s] : ", ip); // delete me.

		if (command == halt) server_running = false;
		else if (command == move_down) printf("server: MOVE DOWN\n");
		else if (command == move_up) printf("server: MOVE UP\n");
		else if (command == move_right) printf("server: MOVE RIGHT\n");		
		else if (command == move_left) printf("server: MOVE LEFT\n");

		else if (command == connect_request) {
			printf("server: they connected to server!\n");

			error = sendto(server, "A", 1, 0, (struct sockaddr*)&address, length);
			check(error);
			
			players[player_count].id0 = (u64) rand();
			players[player_count].id1 = (u64) rand();

			players[player_count].x = (u64) rand() % 10;
			players[player_count].y = (u64) rand() % 10;

			players[player_count].address = address;
			players[player_count].length = length;
		
			// players[player_count].id0 = (u64) * (u32*) (address.sin6_addr.s6_addr + 0);
			// players[player_count].id0 |= (u64) * (u32*) (address.sin6_addr.s6_addr + 4);
			// players[player_count].id1 = (u64) * (u64*) (address.sin6_addr.s6_addr + 8);
			// players[player_count].id1 |= (u64) * (u32*) (address.sin6_addr.s6_addr + 12);

			printf("[%u]: new player uuid is: %llu_%llu\n", player_count + 1, 
				players[player_count].id0, players[player_count].id1);

			player_count++;
		}

		else if (command == disconnect_request) {
			printf("info: client sent a disconnection request!\n"); 
			

			//TODO: get the player index based on the address.
			int player_index = 0;   // lets say.

			// swap the last player, with us. then delete the last.
			players[player_index] = players[--player_count];

		} else printf("warning: received unknown commmand: %c\n", command);
	}
	printf("SERVER: halting...\n"); 
	pthread_join(thread, NULL);
	close(server);
}













	/*
		instead of sending acknoledgement, we should be sending display packets. 

		i also think that the display packets need to be sent in sequence, each 
		of them being disposable, really. 

		that means that will need a sequence number. also they are not acknoledged at all. 

		if you get a lost display packet, then so what. it doesnt matter. 
		also i know that we need to be sending display packets like, 
		totally concurrently. we need to be sending that data 
		like one thread per client, essentially. 
		maybe even faster than that. 

		because the reality is that they are like all totally disposable, 
		and reordeable. thats the beautiful part. 
	
		so i think that actually having a bunch of threads to multiplex on, 
		is a good thing to be able to not have to start up a bunch of threads each time, 
		but still be able to send a ton of packets asynrchonously. 

		also i know that we need to send like at least 1500 bytes or so, (alittle less) 
		in each display frame, so we need to really packet in our data into that space, 
		while still having each packet be completley independent in terms of rendering. 
		thats the goal/dream.

		also,  i know that the movement commands, etc,  are only received, on the 
		server side, and they are not ack'd at all. 

		the server is only sending display packets and receiving commands/movements, 
		and the client is only sendinging commands/movements, and receiving display packets.

		and then, 
		the client isnt using any multi-threading, because it is just sending 
		commands based on the render loop, i think.. (maybe ill rework that?)
		but then the render loop is also just rendering the display packets..
		but, the important thing is that when the algorithm receives a packet 
		which has a lowerer number than what it has received, oh wait. 
		no we cant use that at all, because of the fact that the packets can be 
		reordered... crap... hmm..

	
		

		

	*/



