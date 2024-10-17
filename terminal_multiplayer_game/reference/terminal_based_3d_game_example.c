// this is 3d game that i am writing for fun. only can feel blocks, though.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <iso646.h>
// #include <ctype.h>
// #include <math.h>
#include <time.h>
// #include <sys/time.h> 




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>




/*
----------------------- todo: ---------------------------


	world:
		- make perlin noise world generation.
		- add more blocks. 
		- add more world mechanics...?... 

	player:
		- add more complex player movement, and sensor array, and body. 

		- make the user interface slightyyy more easier to read...

				local (feeler) ray casting?  maybe, yes... 
				yesssss and then the block type is displayed as 
					either a color, and or a letter or character. yay.


					- - question: does the feeler have an orientation?... 
							hmmm....
							i think it has to, in order to do ray casting.
								hmm...
								okay......



		- spawn the player in a better way?... hmm... idk what that would be... 
				i dont want a spawn attempt max parameter though.


	program:

		- load world from file, once we get world gen working.

		- make the keyboard bindings configurable..? hmm..

		- 


*/

// todo: redo how the feeler and player are related. i dont think that the movements should move the feelers?... what is the player?

// add more blocks, and terrain generation, so that we can start exploring in 3d space!

// 




static const int 

	xsize = 100, 
	ysize = 200, 
	zsize = 300,
	
	xmax = xsize - 1, 
	ymax = ysize - 1, 
	zmax = zsize - 1;

// static const char block_string[4 + 1] = ".123";

static int tick = 0;
static int32_t* world = NULL;
static int px = 10, py = 10, pz = 10;
static char message[128] = {0};

static int incr(int a, int max) { return a < max ? a + 1 : 0; }

static int decr(int a, int max) { return a ? a - 1 : max; }

static int32_t* at(int x, int y, int z) { return world + x + xsize * y + xsize * ysize * z; }





//  width must be divisible by 2,   and  
// height must be divisible by 4.

//  0  3
//  1  4
//  2  5
//  6  7


static char* screen = NULL;

static inline void draw(uint8_t* pixels, int width, int height) {

	int length = 9;
	memcpy(screen, "\033[?25l\033[H", 9);
	
	// char c[2] = {0};
	for (int h = 0; h < height; h += 4) {
		for (int w = 0; w < width; w += 2) {
			
			uint8_t b = 
				((pixels[(h + 0) * width + (w + 0)]) << 0) |
				((pixels[(h + 1) * width + (w + 0)]) << 1) |
				((pixels[(h + 2) * width + (w + 0)]) << 2) |
				((pixels[(h + 0) * width + (w + 1)]) << 3) |

				((pixels[(h + 1) * width + (w + 1)]) << 4) |
				((pixels[(h + 2) * width + (w + 1)]) << 5) |
				((pixels[(h + 3) * width + (w + 0)]) << 6) |
				((pixels[(h + 3) * width + (w + 1)]) << 7);
			length += sprintf(screen + length, "\xe2%c%c", 0xa0 | (b >> 6), 0x80 | (0x3F & b));
		}
		screen[length++] = '\033';
		screen[length++] = '[';	
		screen[length++] = 'K';
		screen[length++] = '\r';
		screen[length++] = '\n';
	}

	length += sprintf(screen + length, "\033[1;1H\033[?25h");

	write(1, screen, (size_t) length);
}



// static void clear_screen() { printf("\033[2J\033[H"); }



static inline float inversesqrt(float y) {
	float x2 = y * 0.5f;
	int32_t i = *(int32_t *)&y;
	i = 0x5f3759df - (i >> 1); 	// glm uses a86 for last three digits.
	y = *(float*) &i;
	return y * (1.5f - x2 * y * y);
}

// static inline struct vec3 normalize(struct vec3 v) {
//    float s = inversesqrt(v.x * v.x + v.y * v.y + v.z * v.z);
//    return (struct vec3) {v.x * s, v.y * s, v.z * s};
// }








// void compute() {
// 	if (tick >= 100) {
// 		tick = 0; 
// 		// gravity.
// 		if (not *at(px, decr(py, ymax), pz)) {
// 			*at(px,py,pz) = 0;
// 			if (py) py--; else py = ymax;
// 			*at(px,py,pz) = 2;
// 		}
// 	} else tick++;
// }




int main(int argc, const char** argv) {
// what is the usage message?     program 0 0 0 file_name.bin        where 0 0 0 loads an existing world...?


	srand((unsigned)time(0));
	struct termios terminal = {0};
	tcgetattr(0, &terminal);
	struct termios raw = terminal;
	raw.c_lflag &= ~( (unsigned long)ECHO | (unsigned long)ICANON | (unsigned long)IEXTEN );
	tcsetattr(0, TCSAFLUSH, &raw);
	write(1, "\033[?25l", 6);
	int flags = fcntl(0, F_GETFL, 0);



	world = calloc((size_t) xsize * ysize * zsize, 4);



	// generate_world() using perlin noise here.

	for (int z = 0; z < zsize; z++)
		for (int y = 0; y < ysize; y++) 
			for (int x = 0; x < xsize; x++) 
				*at(x,y,z) = (rand() % 2) * (rand() % 2) * (rand() % 2) * (rand() % 2) * (rand() % 2);


	*at(px,py,pz) = 2; // spawn player's feeler. (or see-er?)

	// *at(px,py,pz + 5) = 1; // spawn player's feeler. (or see-er?)
	// *at(px,py + 1,pz + 5) = 1; // spawn player's feeler. (or see-er?)
	// *at(px + 1,py,pz + 5) = 1; // spawn player's feeler. (or see-er?)
	// *at(px + 2,py,pz + 5) = 1; // spawn player's feeler. (or see-er?)










	const int w5 = 150 * 4, h5 = 100 * 4;    // both must be divisible by 4.

	screen = calloc(3 * (w5/2 + 10) * (h5/4) + 20, 1);
	uint8_t* bits = calloc(w5 * h5, 1);

	// // populate this using ray tracing!
	
	
	// for (double a = 0; a < 300.0; a += 0.02) {

	// 	int radius1 = 30.0 + (int)(20.0 * sin(a));

		

	// 	for (int i = 0; i < h; i++) {
	// 		for (int j = 0; j < w; j++) {
	// 			int y = i - h / 2;
	// 			int x = j - w / 2;
	// 			int s = x * x + y * y;
	// 			int d = radius1 * radius1;
	// 			int e = (int)((float)d * 0.05);
				
	// 			bits[i * w + j] = (s >= d - e && s <= d + e) ? 1 : 0;
	// 		}
	// 	}
		

	// 	draw(bits, w, h);
	// 	usleep(10000);
	// }




	char c = 0;
	// static char string[256] = {0};
	bool running = true, show_debug = false;


	float z_far_plane = 25.0f;
	float screen_distance = 100.0f;

loop:;

	// compute():


	
	// float x = 0, y = 0, z = 0;
	// int H = 5, W = 5;
	

	// for each pixel, trace a ray into the world, and see if it hits a solid block.
	for (int i = 0, h = -h5 / 2; h < h5 / 2; h++,i++) { 
		for (int j = 0, w = -w5 / 2; w < w5 / 2; w++,j++) {


			// printf("---------- on pixel (%d %d) (%d,%d)in screen -----------\n", i,j, h,w);

			// calculate the 3d slope of the line	
			// 	coming from the origin (the players coords)
			// 	going to the pixel?... no... 
			// x x x 1 
			// x x x 1 
			// 0 x x 1 
			// x x x 1 
			// x x x 1

			float dx = (float)w;
			float dy = (float)h;
			float dz = screen_distance;
			// printf("d={%f %f %f} raw\n", dx,dy,dz);
			float s = inversesqrt(dx*dx + dy*dy + dz*dz);
			float d_len = 0.1f;
			dx = dx*s*d_len;
			dy = dy*s*d_len;
			dz = dz*s*d_len;

			// printf("d={%f %f %f} normalized\n", dx,dy,dz);


			// start at the origin.
			float ray_x = (float)px;
			float ray_y = (float)py;
			float ray_z = (float)pz;
			float done_length = 0;

			// printf("ray={%f %f %f} normalized\n", dx,dy,dz);

		this_loop:;
				
			int int_x = 0;
			int int_y = 0;
			int int_z = 0;
			int_x = ((int) floorf(ray_x) + xsize) % xsize;
			int_y = ((int) floorf(ray_y) + ysize) % ysize;
			int_z = ((int) floorf(ray_z) + zsize) % zsize;

			// printf("[dl=%f] trying ray: {%d %d %d}... ", done_length, int_x, int_y, int_z);
			

			if (not (int_x == px and int_y == py and int_z == pz) and *at(int_x, int_y, int_z)) { 

				// printf("HIT! \n");
				// getchar();

				// abort(); // we hit something!
				// draw this pixel white.    (this pixel is    h + h5 / 2   and w + w5 / 2    )


				// bits[(i) * w5 + (j)] = 1;

				goto this_doner;

			} else {
				// printf("miss. \n");
				// abort();
			}
		

			ray_x += dx; // in the direction of motion.
			ray_y += dy; 
			ray_z += dz; 
			done_length += d_len;

			if (done_length < z_far_plane) goto this_loop;


			// printf("total miss,black pixel.\n");
			// getchar();
			bits[(i) * w5 + (j)] = 0;    // the ray didnt hit anything. 
			continue;
		this_doner:
			// printf("HIT pixel. making it white.\n");
			// getchar();
			bits[(i) * w5 + (j)] = 1;    // the ray didnt hit anything. 
			continue;

		}
	}
	

	draw(bits, w5, h5);

	c = 0;
	fcntl(0, F_SETFL, flags | O_NONBLOCK);
	read(0, &c, 1);
	fcntl(0, F_SETFL, flags & ~O_NONBLOCK);

	if (c == 27) running = false;
	if (c == 'z') strcpy(message, "");
	if (c == 'x') show_debug = not show_debug; 
	if (c == 'm') sprintf(message, "message test");

	if (c == 's') { 
		// if (not *at(decr(px, xmax), py, pz)) {
			*at(px,py,pz) = 0;
			if (px) px--; else px = xmax;
			*at(px,py,pz) = 2;
		// }
	} 

	if (c == 't') { 	
		// if (not *at(incr(px, xmax), py, pz)) {
			*at(px,py,pz) = 0;
			if (px < xmax) px++; else px = 0;
			*at(px,py,pz) = 2;
		// }
	} 
	if (c == ' ') { 
		// if (not *at(px, decr(py, ymax), pz)) {
			*at(px,py,pz) = 0;
			if (py) py--; else py = ymax;
			*at(px,py,pz) = 2;
		// }
	} 
	if (c == 'a') { 
		// if (not *at(px, incr(py, ymax), pz)) {
			*at(px,py,pz) = 0;
			if (py < ymax) py++; else py = 0;
			*at(px,py,pz) = 2;
		// }
	} 
	if (c == 'h') { 
		// if (not *at(px, py, decr(pz, zmax))) {
			*at(px,py,pz) = 0;
			if (pz) pz--; else pz = zmax;
			*at(px,py,pz) = 2;
		// }
	}
	if (c == 'r') { 
		// if (not *at(px, py, incr(pz, zmax))) {
			*at(px,py,pz) = 0;
			if (pz < zmax) pz++; else pz = 0; 
			*at(px,py,pz) = 2;
		// }
	}

	if (show_debug) { sprintf(message, "x=%d,y=%d,z=%d", px, py, pz); }
	
	
	usleep(1000);
	if (running) goto loop;
	write(1, "\r\033[K\033[?25h", 10);
	tcsetattr(0, TCSAFLUSH, &terminal);

}







// int radius2 = 30.0 + (int)(20.0 * cos(a));
		// for (int i = 0; i < h; i++) {
		// 	for (int j = 0; j < w; j++) {
		// 		int y = i - h / 2;
		// 		int x = j - w / 2;
		// 		int s = x * x + y * y;
		// 		int d = radius2 * radius2;
		// 		int e = (int)((float)d * 0.05);
				
		// 		bits[i * w + j] |= (s >= d - e && s <= d + e) ? 1 : 0;
		// 	}
		// }



// int length = snprintf(string, 255,
	// 	"\r  %c  %c %c  %c %c  %c %c   %s\033[K", 
	// 	block_string[*at(px,py,pz)],

	// 	block_string[*at(decr(px, xmax),py,pz)],
	// 	block_string[*at(incr(px, xmax),py,pz)],

	// 	block_string[*at(px,decr(py, ymax),pz)],
	// 	block_string[*at(px,incr(py, ymax),pz)],

	// 	block_string[*at(px,py,decr(pz, zmax))],
	// 	block_string[*at(px,py,incr(pz, zmax))],
	// 	message);



// write(1, string, (size_t) length);


