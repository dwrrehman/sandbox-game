

// to research:

// how to look up write in man?





#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
//  width must be divisible by 2,   and  
// height must be divisible by 4.

//  0  3
//  1  4
//  2  5
//  6  7


static char* screen = NULL;

static inline void print_as_braille(uint8_t* bytes, int width, int height) {

	int length = 9;
	memcpy(screen, "\033[?25l\033[H", 9);
	
	char c[2] = {0};
	for (int h = 0; h < height; h += 4) {
		for (int w = 0; w < width; w += 2) {
			
			uint8_t b = 
				((bytes[(h + 0) * width + (w + 0)]) << 0) |
				((bytes[(h + 1) * width + (w + 0)]) << 1) |
				((bytes[(h + 2) * width + (w + 0)]) << 2) |
				((bytes[(h + 0) * width + (w + 1)]) << 3) |
				((bytes[(h + 1) * width + (w + 1)]) << 4) |
				((bytes[(h + 2) * width + (w + 1)]) << 5) |
				((bytes[(h + 3) * width + (w + 0)]) << 6) |
				((bytes[(h + 3) * width + (w + 1)]) << 7);
			length += sprintf(screen + length, "\xe2%c%c", 0xa0 | (b >> 6), 0x80 | (0x3F & b));
		}
		screen[length++] = '\033';
		screen[length++] = '[';	
		screen[length++] = 'K';
		screen[length++] = '\r';
		screen[length++] = '\n';
	}

	length += sprintf(screen + length, "\033[%ld;%ldH\033[?25h", 1, 1);

	write(1, screen, (size_t) length);
}


static void clear_screen(void) { printf("\033[2J\033[H"); }




int main(void) {
	int w = 900, h = 900;
	uint8_t* bits = calloc(w * h, 1);
	screen = calloc((w/2) * (h/4), 1);
	
	for (double a = 0; a < 300.0; a += 0.02) {

		int radius1 = 240.0 + (int)(200.0 * sin(a));

		int radius2 = 240.0 + (int)(200.0 * cos(a));

		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {
				int y = i - h / 2;
				int x = j - w / 2;
				int s = x * x + y * y;
				int d = radius1 * radius1;
				int e = (int)((float)d * 0.05);
				
				bits[i * w + j] = (s >= d - e && s <= d + e) ? 1 : 0;
			}
		}
		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {
				int y = i - h / 2;
				int x = j - w / 2;
				int s = x * x + y * y;
				int d = radius2 * radius2;
				int e = (int)((float)d * 0.05);
				
				bits[i * w + j] |= (s >= d - e && s <= d + e) ? 1 : 0;
			}
		}
		// clear_screen();
		print_as_braille(bits, w, h);
		// fflush(stdout);
		usleep(1000);
	}
	
}


















// int8_t c[2] = {0x28, 0xFF}
// printf("%c%c",c[0], c[1]);
// printf("\u28ff\n");
// printf("\u2800\n");

// char a[] = "⣿";//    "⠀";
// printf("strlen(a) = %lu\n", strlen(a));
// printf("[0] = %02hhx, [1] = %02hhx, [2] = %02hhx\n", a[0], a[1], a[2]);


// 00 
// strlen(a) = 3
// [0] = e2, [1] = a0, [2] = 80

// ff

// strlen(a) = 3
// [0] = e2, [1] = a3, [2] = bf



//  1010 0011     1011 1111





// for (int i = 0; i < h; i++) {
	// 	for (int j = 0; j < w; j++) {
	// 		int y = i - h / 2;
	// 		int x = j - w / 2;
	// 		int s = x * x + y * y;
	// 		int d = radius2 * radius2;
	// 		int e = (int)((float)d * 0.05);
	// 		bits[i * w + j] |= (s > d - e && s < d + e) ? 1 : 0;
	// 	}
	// }














/*


float noise(int x, int y) {
    int n;

    n = x + y * 57;
    n = (n << 13) ^ n;
    return (1.0 - ( (n * ((n * n * 15731) + 789221) +  1376312589) & 0x7fffffff) / 1073741824.0);
}




float perlin_two(float x, float y, float gain, int octaves, int hgrid) {
    int i;
    float total = 0.0f;
    float frequency = 1.0f/(float)hgrid;
    float amplitude = gain;
    float lacunarity = 2.0;

    for (i = 0; i < octaves; ++i)
    {
        total += noise_handler((float)x * frequency, (float)y * frequency) * amplitude;         
        frequency *= lacunarity;
        amplitude *= gain;
    } 

    return (total);
}



*/




