#include <stdio.h>

int main() {
	for (char lo = 0; lo < 64; lo++) { //  6 bits.
		for (char hi = 0; hi < 4; hi++) { // 2 bit 
			printf("\xe2%c%c", 0xA0 + hi, 0x80 + lo);
		}
	}

	puts("");

	for (int x = 0; x < 16; x++) {
		for (int y = 0; y < 32; y++) {
			printf("\xe2%c%c", 0xa0 + 3, 0x80 + 63);
		}
		puts("");
	}
}







// printf("%d: (%d,%d) = [\xe2%c%c]\n\n", hi + lo_limit * lo, hi,lo, 0xA0 + hi, 0x80 + lo);



