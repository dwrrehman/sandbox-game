// server for the universe sandbox game. 

/*--------------------------- command line usage ---------------------------------------




	./server <n: nonzero natural number> <new_filename string>


		creates a new universe, sized the given number,
		and upon save, will its data will be written to the new filename,
			and such file will be overwritten if it already exists.

			n is the log base 2 of the number of bytes per side, 

			(note: the number of bytes per side is half 
				the number of bits per side).



			ie, the universe will have this many binary cells (bits):

				             (2 ^ (n + 1)) ^ 3




	./server load <existing_filename string> 
		

		loads an existing universe, reading the contents of the file named 
			existing_filename, into the state for the universe.
			the size of the file is the size of he universe.

	



----------------------------------- state format --------------------------------------------

	the state is an array of bytes.
	each byte in the state is actually 8 binary modnats (bits) in the universe, 
	these 8 modnats are arranged in a 3D 2 x 2 x 2 cube as follows:

	
				bottom layer:

					0 1 
					2 3 

				top layer:

					4 5
					6 7

		
	and so when simulating the universe cellular automaton, bit masking must be done to get 
	at the various neighbors, as some of them will be present in the current byte, 
	and some in a different byte.

	the neighborhood used is a vonneuman radius 1, (2n + 1, including the center cell) neighborhood.
	this implies that there are exactly 7 neighbors per cell.
	this is close to 8, and so a byte is also used to represent the state of the neighborhood. 

	a function can be called, "get_neighborhood()", which, given a byte index (consisting of 3 uints, the last bit of each denotes a bit offset within the given byte specified by right-shifted (by 1) versions of the ints.),
	which specifies a unique cell, and the function returns its neighborhood, as a byte.
	
	

-----------------------------------------------------------------------------------------*/

#include <iso646.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

typedef  uint8_t byte;
typedef uint64_t nat;

static nat n;
static nat count;
static byte* state;

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
	for (nat i = 0; i < count; i++) 
		state[i] = (byte)(rand() % 256);
}

static inline void show() {
	printf("\nstate:  side = %llu,  count = %llu\n\n", count);
	printf("{ \n");
	for (nat i = 0; i < count; i++) {
		if (i and (i % 8) == 0) puts("");
		printf("%02hhx ", state[i]);
	}
	printf("}\n");
}



/*

	compute_8_cells_at_a_time(ruleset, nat byte_index);

	
this function computes 8 cell outputs at a time, 
given the rule set, 
and uses the given byte index, to address the bit-[0]th cell in the byte.

neighbors are put in the following ordering:

	[ 0 1 2 3 4 5 6 7 ]

	[ C R L U D F B - ]      // natural ordering of the 7 neighbors, for n = 7 (3 dimensional).




*/

static inline void iterate
	(uint64_t rule_top, uint64_t rule_bottom) 
{

	// assert    size   is a power of two! 

	for (nat cell = 0; cell < count; cell++) {
		// does 8 cells at a time!

		
	}

	nat c = 0x0; //byte index.
	byte n = 3; // log2 of side length of bytes.


	byte k = n << 1;
	nat s = 1 << n;
	nat t = s - 1;

	nat cn = c >> n;
	nat ck = c >> k;
	nat ct = c & t;

	nat cnt = cn & t;
	nat cnt = ck & t;

	nat c1t = (c + 1) & t;	
	nat cn1t = (cn + 1) & t;
	nat ck1t = (ck + 1) & t;

	nat ctt = (c + t) & t;
	nat cntt = (cn + t) & t;
	nat cktt = (ck + t) & t;

	byte C = state[c];

	byte R = state[c + c1t - ct];
	byte L = state[c + ctt - ct];

	byte U = state[c + ((cn1t - cnt) << n)];
	byte D = state[c + ((cntt - cnt) << n)];

	byte F = state[c + ((cktt - ckt) << k)];
	byte B = state[c + ((ck1t - ckt) << k)];

		// bottom layer:

		// 	0 1 
		// 	2 3 

		// top layer:

		// 	4 5
		// 	6 7




// ---------- compute bit [0]: ------------

	C[bit0] // C

	C[bit1] // R
	L[bit1] // L

	C[bit2] // D
	U[bit2] // U

	C[bit4] // B
	F[bit4] // F

	



	// the ruleset is exactly 128 bits. 

	// meaning that we can actually passs it in, using only two uint64_t's.

	// however, we have to do some checks to see if we are accessing the top half bits, or the bottom half bits... so thats the next challenge. 

	// this is SO COOL.


	// i love this.




}




int main(const int argc, const char** argv) {
	

	if (argc < 3) exit(puts( "usage: \n\t./server <size: nat> <filename: string>\n"));
	byte n = (byte) atoi(argv[1]);
	
	if (n) {
		nat s = 1 << n;
		count = s * s * s;
		state = malloc(count);
		printf("generating %llu bytes...\n", count);
		generate();

	} else {
		printf("loading universe file...\n");
		load(argv[2]);
	}

	printf("testing saving and loading the universe modnats. \nheres the state:\n");
	show();
	printf("saving...\n");
	save(argv[2]);
	printf("done.\n");
}








	// for (int x = 0; x < size; x++) {
	// 	for (int y = 0; y < size; y++) {
	// 		for (int z = 0; z < size; z++) {
				
				

	// 		}
	// 	}
	// }




// for (nat i = 1; i < count; i *= size) {

 // 	read_array[c + ((c + s - 1) % s - c % s)];
	// read_array[c + s * ((c / s + s - 1) % s - c / s % s)];
	// read_array[c + (s * s) * ((c / (s * s) + s - 1) % s - c / (s * s) % s)];

	// read_array[c + ((c / 1 + 1) % s - c / 1 % s)];
	// read_array[c + s * ((c / s + 1) % s - c / s % s)];
	// read_array[c + (s * s) * ((c / (s * s) + 1) % s - c / (s * s) % s)];
	// // }


	// }

	// 	int x = 23;
	// int m = 8.
	// int y = x & (m - 1); // y = x % m
	// printf("x = x mod %d  |  %d = %d mod (2^%d)\n", y,x,n);
	// exit(0);




// this would be the operation, if, we used a bit per state byte.
	// but we use 8 bits per byte (obviously)
	// so we need to do some extra fancy stuff now... hmm...
