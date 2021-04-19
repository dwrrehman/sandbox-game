// server for the universe sandbox game. 

#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mman.h>

typedef  uint8_t byte;
typedef uint32_t uint;
typedef uint64_t nat;

static nat side;
static nat count;
static byte* state; 

// each byte is a 2x2x2 cube of 8 binary modnats: bits.
/*
bottom:
	0 1 
	2 3 
top:
	4 5
	6 7

*/

#include <stdio.h>
#include <stdlib.h>

static inline void save(const char* destination) {
	FILE* file = fopen(destination, "w");
	if (not file) { perror("open"); exit(3); }
	fwrite(state, sizeof(byte), count, file);
	fclose(file);
}

static inline void load(const char* source) {
	FILE* file = fopen(source, "r");
	if (not file) { perror("fopen"); exit(3); }
	fseek(file, 0, SEEK_END);
	count = (nat) ftell(file);
	state = malloc(count);
	fseek(file, 0, SEEK_SET);
	fread(state, 1, count, file);
	fclose(file);
}

static inline void generate() {
	for (nat i = 0; i < count; i++) {
		state[i] = (byte)(i % 256);
	}
}

static inline void show() {
	printf("\nstate:  side = %llu,  count = %llu\n\n", side, count);
	printf("{ \n");
	for (nat i = 0; i < count; i++) {
		if (i and (i % 8) == 0) puts("");
		printf("%02hhx ", state[i]);
	}
	printf("}\n");
}

int main(const int argc, const char** argv) {
	
	printf("testing saving and loading the universe modnats.\n");

	// printf("generating...\n");
	// count = 1938;
	// state = malloc(count);
	// generate();

	// printf("saving...\n");
	// save("test.txt");

	printf("loading...\n");
	load("test.txt");

	printf("showing loaded world:\n");
	show();

	
}
