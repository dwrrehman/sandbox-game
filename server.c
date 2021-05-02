// server for the universe sandbox game. 

#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint32_t u64;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int32_t i64;

typedef uint8_t byte;
typedef uint64_t nat;

static nat n, m, * universe;

static inline void save(const char* destination) {
	FILE* file = fopen(destination, "w");
	if (not file) { perror("open"); exit(3); }
	fwrite(universe, sizeof(nat), n, file);
	fclose(file);
}

static inline void load(const char* source) {
	FILE* file = fopen(source, "r");
	if (not file) { perror("fopen"); exit(3); }
	fseek(file, 0, SEEK_END);
	nat size = (nat) ftell(file);
	n = size >> sizeof(nat);
	universe = malloc(size);
	fseek(file, 0, SEEK_SET);
	fread(universe, sizeof(nat), n, file);
	fclose(file);
}

// static inline void show() {
// 	printf("\nstate:  side = %llu,  count = %llu\n\n", count);
// 	printf("{ \n");
// 	for (nat i = 0; i < count; i++) {
// 		if (i and (i % 8) == 0) puts("");
// 		printf("%02hhx ", state[i]);
// 	}
// 	printf("}\n");
// }

int main(const int argc, const char** argv) {
	
	if (argc < 5) exit(puts( "usage: \n\t./server <m: nat> <n: nat> <port: nat> <filename: string>\n"));
	nat m = (u16) atoi(argv[1]);
	nat n = (u8) atoi(argv[2]);
	u16 port = (u8) atoi(argv[3]);
	
	if (n) {
		universe = calloc(n, sizeof(nat));
		printf("generating %llu bytes...\n", count);

	} else {
		printf("loading universe file...\n");
		load(argv[4]);
	}

	printf("testing saving and loading the universe modnats. \n");
	printf("saving...\n");
	save(argv[4]);
	printf("done.\n");
}



/// okay, so theres a problem with this. 

/// we need to decide how we are going to actually do the universe, in this game.
/// are we going to run   (1,2048)   ?    always binary?   have m be 255? ie, have each cell always be a byte,
////   which actually, according to our current theory says is impossible, because you actually need natural numbers, because the last natural number has range n, even though you never dereference it... hmm... 

/// so basically, you can see, we are basically forced into like...     storing the universe as an array of nats.....?


/// althoguh, actually my plan for this, was to just run a 3-dimensional (whatever that means)    and binary universe, 
//  and the idea was that i am just going to hard code the valaues of m and n by finding a particular binary 3D cellular automaton, which woul be the laws of physics for the universe.

/// however, actually, im actually thinking of just making the server run a arbitary given m and n,.....  although like... thats just like ....      NOT POSSIBLE,
//. if you think about client side.     like, yes, itss possible serverside,    but then the client, and even the player body itself, needs to constantly change for the given vlaue of m and n. if the laws of physics change for the universe, (ie, m or n changes, ie the server is supplied with a different value of m and n the next time, you want to simultae a new universe)) then that kinda blows awawy any engineering we did on the body, and the player apparaturs....    so we actually NEED to hardcode a particular m and n,   (PROBABLY AS A CA, SO WE CAN MAKE M SMALL, (SO THINGS ARE SIMPLE,  AND MANAGEABLE..?)   BUT STILL HAVE A LARGE UNIVERSE!)

// my idea is that        if we cn get away with m = 1,  (ie, binary)   then its actually super good idea to like hard code the ua universe rules as a CA, by finding the particular thing, because... well.... wait... 

/// the other probleem, is that the neighborhood for a 3D universe, with gravity, (depending if we even want gravity..!?!?)

/// would actually be problematic...     what value of n are we going to choose..?   well, ideally, whatever value of n  (probably very small!) that we need to in order to have h = 7, ie, 3D..? hmm... (3 dimensional)... i feel like i dont neccessarily need gravity... although that would make it like our universe, buttttt i kinda specfically dont want it to be like our universe, i want the laws o physics to be different... im just not sure howwwww differentttttttt...

// so thats where im at right now, i think. it would be awfully helpful to like ... have the ua figured out    before we continue making the server. thats kinda SUPER DUPER required lol. so yeah. might not wokr on this code for a bit, until i figure it out. shouldnt take long lol. just be patient. ill get there. 


// okay, thanks for listening, bit of a tb on the ua sandbox game.

