// server for block based sandbox game.
// written by dwrr, started on 202406156.001556:
// network test written on 202406156.021136: by dwrr
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdatomic.h>

typedef uint64_t nat;

static const uint16_t server_port = 12000;

#define max_player_count 64

static const int hash[] = {
	208,34,231,213,32,248,233,56,161,78,24,140,71,48,140,254,245,255,247,247,40,
	185,248,251,245,28,124,204,204,76,36,1,107,28,234,163,202,224,245,128,167,204,
	9,92,217,54,239,174,173,102,193,189,190,121,100,108,167,44,43,77,180,204,8,81,
	70,223,11,38,24,254,210,210,177,32,81,195,243,125,8,169,112,32,97,53,195,13,
	203,9,47,104,125,117,114,124,165,203,181,235,193,206,70,180,174,0,167,181,41,
	164,30,116,127,198,245,146,87,224,149,206,57,4,192,210,65,210,129,240,178,105,
	228,108,245,148,140,40,35,195,38,58,65,207,215,253,65,85,208,76,62,3,237,55,89,
	232,50,217,64,244,157,199,121,252,90,17,212,203,149,152,140,187,234,177,73,174,
	193,100,192,143,97,53,145,135,19,103,13,90,135,151,199,91,239,247,33,39,145,
	101,120,99,3,186,86,99,41,237,203,111,79,220,135,158,42,30,154,120,67,87,167,
	135,176,183,191,253,115,184,21,233,58,129,233,142,39,128,211,118,137,139,255,
	114,20,218,113,154,27,127,246,250,1,8,198,250,209,92,222,173,21,88,102,219
};

struct nat3 { nat x, y, z; };

static pthread_t threads[max_player_count] = {0};
static thread_count = 0;

static const char* player_name[max_player_count] = {0};
static struct nat3 player_position[max_player_count] = {0};
static nat player_count = 0;

static nat sidelength = 0;

static int seed = 0;
static int server = 0;

static _Atomic uint8_t* space = NULL;
static _Atomic nat running;

struct arguments {
	nat player;
	struct sockaddr_in6 address;
};

static nat at(nat x, nat y, nat z) { return sidelength * sidelength * x + sidelength * y + z; }
static nat at3(struct nat3 p) { return sidelength * sidelength * p.x + sidelength * p.y + p.z; }
static void set(nat x, nat y, nat z, uint8_t block) {
	atomic_store_explicit(space + at(x, y, z), block, memory_order_relaxed);
}
static void set3(struct nat3 p, uint8_t block) {
	atomic_store_explicit(space + at3(p), block, memory_order_relaxed);
}

static char ip_string_buffer[INET6_ADDRSTRLEN + 16] = {0};

static char* ip_string(struct sockaddr_in6 address) {
	char string[INET6_ADDRSTRLEN] = {0};
	inet_ntop(AF_INET6, &(address.sin6_addr), string, sizeof(string));
	snprintf(ip_string_buffer, sizeof ip_string_buffer, "[%s,%llu]", string, ntohs(address.sin6_port));
	return ip_string_buffer;
}

static int noise2(int x, int y) {
    int tmp = hash[(y + seed) % 256];
    return hash[(tmp + x) % 256];
}

static float smooth_inter(float x, float y, float s) {
	return x + (s * s * ( 3 - 2 * s )) * (y - x);
}

static float noise2d(float x, float y) {
	int x_int = (int) x;
	int y_int = (int) y;
	float x_frac = x - x_int;
	float y_frac = y - y_int;
	int s = noise2(x_int, y_int);
	int t = noise2(x_int+1, y_int);
	int u = noise2(x_int, y_int+1);
	int v = noise2(x_int+1, y_int+1);
	float low = smooth_inter(s, t, x_frac);
	float high = smooth_inter(u, v, x_frac);
	return smooth_inter(low, high, y_frac);
}

static float perlin2d(float x, float y, float freq, int depth) {
	float xa = x*freq;
	float ya = y*freq;
	float amp = 1.0;
	float fin = 0;
	float div = 0.0;

	for (int i = 0; i < depth; i++) {
		div += 256 * amp;
		fin += noise2d(xa, ya) * amp;
		amp /= 2;
		xa *= 2;
		ya *= 2;
	}
	return fin / div;
}

static void generate_world(void) {

	printf("generate_world: using seed = %u, and space size = %llu...\n", seed, s);

	space = calloc(sidelength * sidelength * sidelength, sizeof(_Atomic uint8_t));

	for (int x = 0; x < sidelength; x++) {
		for (int z = 0; z < sidelength; z++) {

			const float f = perlin2d(x, z, 0.01f, 20);
			const int H = (int) (f * 50);

			const int divide = H / 2;
			for (int y = 0; y < H; y++) {
				if (y >= divide) set(x, y, z, dirt_block);
				if (y < divide) set(x, y, z, stone_block + (rand() % 2) * (rand() % 2));

			}
			set(x, H, z, grass_block);
		}
	}

	uint8_t block = 0;
	for (nat i = 4; i < 32; i += 2) {
		set(50, 50, i, block);
		block++;
	}
}

static void worker_thread(void* raw_arg) {

	struct arguments args = *(struct arguments*) raw_arg;
	free(raw_arg);

	const nat ti = args.player;
	const char* name = player_name[ti];
	const char* ip = strdup(ip_string(args.address));
	struct sockaddr_in6 client = args.address;
	socklen_t length = sizeof(client);

	printf("info: player connected! started worker thread: ");
	printf("%llu : %s : %s...\n", ti, name, ip);

	bool player_quit = false;
	while (not player_quit) {
		
		printf("worker %llu: listening for messages from player %s : %s...\n", ti, name, ip);

		char data[128] = {0};
		ssize_t n = recvfrom(server, data, sizeof data, 0, (struct sockaddr*) &client, &length);

		if (n <= 0) { puts("worker data recvfrom: n <= 0"); abort(); }

		printf("worker %llu: recevied %llub from %s, at %s...\n", ti, n, name, ip_string(client));
		printf("player data = <%s>\n", data);


		if (data[0] == 'q') player_quit = true;

		

		n = sendto(server, "data good", 9, 0, (struct sockaddr*) &client, sizeof(client));
		if (n <= 0) { puts("sendto: n <= 0"); abort(); }

		printf("worker %llu: sendto: sending data good reply to player %s...\n", ti, name);
	}

	printf("info: player disconnected! terminating worker thread: ");
	printf("%llu : %s : %s...\n", ti, name, ip);

	free(ip);
}


static struct nat3 spawn_player(void) {

	// unimpl

	return (struct nat3) {20, 30, 20};
}

int main(void) {

	seed = 42;  sidelength = 100;

	atomic_init(&running, 1);
	generate_world();

	server = socket(AF_INET6, SOCK_DGRAM, 0);
	if (server < 0) { perror("socket"); exit(1); }

	struct sockaddr_in6 server_addr = {0};
	server_addr.sin6_family = AF_INET6;
	server_addr.sin6_addr = in6addr_any;
	server_addr.sin6_port = htons(server_port);

	int r = bind(server, (struct sockaddr*) &server_addr, sizeof(server_addr));
	if (r < 0) { perror("bind"); exit(1); }

	while (running) {
		struct sockaddr_in6 client = {0};
		socklen_t length = sizeof(client);

		printf("main: listening on port %llu for players to join...\n", server_port);
		char player_name[128] = {0};
		ssize_t n = recvfrom(server, player_name, sizeof player_name - 1, 0, (struct sockaddr*) &client, &length);
		if (n <= 0) { puts("main: player_name recvfrom: n <= 0"); abort(); }

		printf("main: recevied %llub from address: %s...\n", n, ip_string(client));
		printf("player name = <%s>\n", player_name);

		n = sendto(server, "c", 1, 0, (struct sockaddr*) &client, sizeof(client));
		if (n <= 0) { puts("sendto: n <= 0"); abort(); }
		printf("main: sendto: sent ack to player %s\n", player_name);

		struct arguments* args = calloc(sizeof(struct arguments));
		args->address = client;
		args->player = player_count;

		player_position[player_count] = spawn_player();
		player_name[player_count++] = strdup(player_name);

		pthread_create(threads + thread_count, NULL, worker_thread, args);
		pthread_detach(threads[thread_count]);
		thread_count++;
	}
	close(server);
}





// note:
//
//    max size of a udp ipv6 packet is  1,212 bytes.
//





