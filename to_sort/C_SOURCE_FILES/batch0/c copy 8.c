// 2D - a top down block game
// This is a top down minecraft like block game that i want to make my own 
// laws of physics and universe in, so that i can have a world to play in. 

// made by dwrr, revived project on 2212191.201907



// original description:
// this is a minecraft like game that i am writing for fun. not sure why. 



/*
	todo:

		-	make it use top down generation

		-	make the rendering use line of sight, so that you cant see through walls. 

		-	make more blocks 
	
		-	add dynamic behavior to the game. 

		-	add another mob/character

		-	add ability to save game!

		-	add rendering opt to print less color sequences (for row-contigous same blocks)...?  hmm.. nahh... 

			






*/







#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <iso646.h>

#include <ctype.h>
#include <math.h>
#include <time.h>
#include <sys/time.h> 

typedef int32_t block;
typedef ssize_t nat;





// ----------------------- constants --------------------------

static const nat xsize = 300, ysize = 200;         // the virtual world size.    make bigger for a bigger world. 
static const nat height = 20, width = 20;         // the screen size. make bigger to display more blocks at once. 

static const nat seed = 42;

static const nat max_spawn_attempts = 10000;        // spawn attempt parameter-    delete me.





static const nat maximum_inventory_radius = 10;     // remove this...!?!!?  







enum block_type {

	air,
	bedrock,

	player,
	player1,
	player2,
	player3,

	inventory,
	inventory1,
	inventory2,
	inventory3,

	dirt,
	stone,
	grass,

	arrow,
	box,

};


static const char* block_face[] = {
	"   ", 					// air 
	"\033[38;5;237m[#]\033[0m", 		// bedrock

	"\033[1;38;5;222m ᐱ \033[0m", 		// player
	"\033[1;38;5;222m ᐯ \033[0m", 		// player
	"\033[1;38;5;222m ᐸ \033[0m", 		// player
	"\033[1;38;5;222m ᐳ \033[0m", 		// player

	"\033[1;38;5;80m ⍓ \033[0m", 		// inv
	"\033[1;38;5;80m ⍌ \033[0m", 		// inv
	"\033[1;38;5;80m ⍃ \033[0m", 		// inv
	"\033[1;38;5;80m ⍄ \033[0m", 		// inv
	
	
	"\033[38;5;94m[&]\033[0m", 		// dirt
	"\033[38;5;250m[⠿]\033[0m", 		// stone
	"\033[38;5;28m._,\033[0m", 		// grass

	"\033[38;5;250m ⤅ \033[0m", 		// arrow
	"\033[38;5;250m ⍔ \033[0m", 		// box
};

// normal = "\033[38;5;67m" 
// bold = "\033[1;38;5;42m"
// reset = "\033[0m"




// ----------------------- globals -------------------------------


static char* screen = NULL;
static char message[1024] = {0};



static block* world = NULL;

static nat 
	px = 0, py = 0, pd = 0, 
	invx = 0, invy = 0, invd = 0,   invhomex = 0,  invhomey = 0,

	inside_inv = 0, display_debug = 1,

	tick = 0;


static inline struct termios configure_terminal() {
	struct termios save = {0};
	tcgetattr(0, &save);
	struct termios raw = save;
	raw.c_oflag &= ~( (unsigned long)OPOST );
	raw.c_iflag &= ~( (unsigned long)BRKINT 
			| (unsigned long)ICRNL 
			| (unsigned long)INPCK 
			| (unsigned long)IXON );	
	raw.c_lflag &= ~( (unsigned long)ECHO 
			| (unsigned long)ICANON 
			| (unsigned long)IEXTEN );
	tcsetattr(0, TCSAFLUSH, &raw);
	return save;
}


static block* at0(nat x, nat y) { return world + x + xsize * y; }

static block* atp() { return at0(px, py); }

static block* ati() { return at0(invx, invy); }

// static block* atih() { return at0(invhomex, invhomey); }

static block* at(nat x, nat y, nat xoff, nat yoff) {
	const nat x_ = ((x + xsize + xoff) % xsize);
	const nat y_ = ((y + ysize + yoff) % ysize);
	return at0(x_,y_);
}

static void display() {
	int length = 9; 
	memcpy(screen, "\033[?25l\033[H", 9);

	length += sprintf(screen + length, "\033[38;5;238m┌");
	for (int x = 0; x < 3 * width; x++) length += sprintf(screen + length, "─");
	length += sprintf(screen + length, "┐\033[0m\033[K\r\n");

	for (int h = -height/2; h < height/2; h++) {
		length += sprintf(screen + length, "\033[38;5;238m│\033[0m");
		for (int w = -width/2; w < width/2; w++) {

			nat originx = px, originy = py;
			if (inside_inv) { originx = invx; originy = invy; } 

			block b = *at(originx,originy,w,h);

			if (b == player) b += pd;
			if (b == inventory) b += invd;

			length += sprintf(screen + length, "%s", block_face[b]);
		}
		length += sprintf(screen + length, "\033[38;5;238m│\033[0m\033[K\r\n");
	}

	length += sprintf(screen + length, "\033[38;5;238m└");
	for (int x = 0; x < 3 * width; x++) length += sprintf(screen + length, "─");
	length += sprintf(screen + length, "┘\033[0m\033[K\r\n");


	if (display_debug) 
		length += sprintf(screen + length, 
			"[%zd, %zd; %zd] %s \033[K\r\n", 
				px, py, pd, message);


	write(1, screen, (size_t) length);
}

static void compute() {
	tick++;
	if (tick != 100) return;
	tick = 0;

	// code here
}


static void spawn_player() {

	nat x = 0, y = 0;

	for (nat i = 0; i < max_spawn_attempts; i++) {
		for (nat yk = -1; yk <= 1; yk++) {
			for (nat xk = -1; xk <= 1; xk++) {
				if (*at(x,y,xk,yk)) goto try_next;
			}
		}
		goto done;
		try_next: ;
		x = rand() % xsize;
		y = rand() % ysize;
	}
done:
	px = x; py = y; pd = 0;
	*at0(x,y) = player;
}




static void spawn_inventory() {

	nat inv_rad = maximum_inventory_radius;

	nat x = 0, y = 0;

	while (inv_rad > 2) {

		for (nat i = 0; i < max_spawn_attempts; i++) {

			for (nat yk = -inv_rad; yk <= inv_rad; yk++) {
				for (nat xk = -inv_rad; xk <= inv_rad; xk++) {
					if (*at(x,y,xk,yk)) goto try_next;
				}
			}
			goto done;
			try_next:
			x = rand() % xsize;
			y = rand() % ysize;
		}
		inv_rad--;
	}

done:
	invhomex = x; invhomey = y;
	invx = x; invy = y; invd = 0;
	*at0(x,y) = box;
}


static void move_player_up() {
	if (*at(px,py,0,-1)) return;
	*atp() = air;
	if (py) py--; else py = ysize - 1;
	*atp() = player;
}
static void move_player_down() {
	if (*at(px,py,0,1)) return;
	*atp() = air;
	py++; if (py == ysize) py = 0;
	*atp() = player;
}
static void move_player_left() {
	if (*at(px,py,-1,0)) return;
	*atp() = air;
	if (px) px--; else px = xsize - 1;
	*atp() = player;
}
static void move_player_right() {
	if (*at(px,py,1,0)) return;
	*atp() = air;
	px++; if (px == xsize) px = 0;
	*atp() = player;
}

static nat absolute(nat x) {
	if (x < 0) return -x;
	return x;
}

static void move_inv_up() {
	if (absolute((invy - 1) - invhomey) > maximum_inventory_radius) return;
	if (*at(invx,invy,0,-1)) return;
	*ati() = air;
	if (invy) invy--; else invy = ysize - 1;
	*ati() = inventory;
}
static void move_inv_down() {
	if (absolute((invy + 1) - invhomey) > maximum_inventory_radius) return;
	if (*at(invx,invy,0,1)) return;
	*ati() = air;
	invy++; if (invy == ysize) invy = 0;
	*ati() = inventory;
}
static void move_inv_left() {
	if (absolute((invx - 1) - invhomex) > maximum_inventory_radius) return;
	if (*at(invx,invy,-1,0)) return;
	*ati() = air;
	if (invx) invx--; else invx = xsize - 1;
	*ati() = inventory;
}
static void move_inv_right() {
	if (absolute((invx + 1) - invhomex) > maximum_inventory_radius) return;
	if (*at(invx,invy,1,0)) return;
	*ati() = air;
	invx++; if (invx == xsize) invx = 0;
	*ati() = inventory;
}


static bool put_block_in_inventory(block g){
		
	nat xoff = 1, yoff = 0;

	if (invd == 0) {// up
		xoff = 0; yoff = -1;
	} else if (invd == 1) {// down
		xoff = 0; yoff = 1;
	} else if (invd == 2) {// left
		xoff = -1; yoff = 0;
	} else if (invd == 3) {// right
		xoff = 1; yoff = 0;
	}
	
	if (invd == 0) move_inv_down();
	if (invd == 1) move_inv_up();
	if (invd == 2) move_inv_right();
	if (invd == 3) move_inv_left();

	block* b = at(invx, invy, xoff, yoff);
	if (*b) return false;
	*b = g;
	return true;
}

static block get_block_from_inventory() {
	
	nat xoff = 1, yoff = 0;

	if (invd == 0) {// up
		xoff = 0; yoff = -1;
	} else if (invd == 1) {// down
		xoff = 0; yoff = 1;
	} else if (invd == 2) {// left
		xoff = -1; yoff = 0;
	} else if (invd == 3) {// right
		xoff = 1; yoff = 0;
	}

	block* b = at(invx, invy, xoff, yoff);
	block w = *b;
	if (*b) {
		*b = air;
		if (invd == 0) move_inv_up();
		if (invd == 1) move_inv_down();
		if (invd == 2) move_inv_left();
		if (invd == 3) move_inv_right();
	}
	return w;
}

static void break_block() {
	nat xoff = 1, yoff = 0;

	if (pd == 0) {// up
		xoff = 0; yoff = -1;
	} else if (pd == 1) {// down
		xoff = 0; yoff = 1;
	} else if (pd == 2) {// left
		xoff = -1; yoff = 0;
	} else if (pd == 3) {// right
		xoff = 1; yoff = 0;
	}

	block* this = at(px, py, xoff, yoff);

	if (not *this) return;
	if (put_block_in_inventory(*this))
		*this = air;
}

static void place_block() {

	// get pointed to block from inventory


	nat xoff = 1, yoff = 0;

	if (pd == 0) {// up
		xoff = 0; yoff = -1;
	} else if (pd == 1) {// down
		xoff = 0; yoff = 1;
	} else if (pd == 2) {// left
		xoff = -1; yoff = 0;
	} else if (pd == 3) {// right
		xoff = 1; yoff = 0;
	}

	block* this = at(px, py, xoff, yoff);
	if (*this) return; 

	block new = get_block_from_inventory();
	if (new) *this = new; 
}

static void generate_world() {

	for (nat y = 0; y < ysize; y++) {
		for (nat x = 0; x < xsize; x++) {
			const nat p = (rand() % 2) * (rand() % 2) * (rand() % 2);
			*at0(x,y) = p ? dirt : air;
		}
	}

	for (nat y = 0; y < ysize; y++) {
		for (nat x = 0; x < xsize; x++) {
			const nat p = (rand() % 2) * (rand() % 2) * (rand() % 2) * (rand() % 2) * (rand() % 2);
			if (p) *at0(x,y) = stone;
		}
	}

	for (nat y = 0; y < ysize; y++) {
		for (nat x = 0; x < xsize; x++) {
			const nat p = (rand() % 2) * (rand() % 2) * (rand() % 2) * (rand() % 2) * (rand() % 2);
			if (p) *at0(x,y) = grass;
		}
	}

}



static void load_game() {}

static void save_game() {}



int main() { /// /*int argc, const char** argv*/

	// base on argc and argv:


	load_game();

	// -or- 


	unsigned s = seed;
	if (not seed) s = (unsigned) time(0);
	srand(s);


	world = calloc((size_t) xsize * ysize, sizeof(block));
	screen = calloc((size_t)(height * width * 256), sizeof(char));

	generate_world();   // new world


	spawn_player();
	spawn_inventory();





	struct termios terminal = configure_terminal();
	write(1, "\033[?1049h\033[?1000h", 16);

	const int flags = fcntl(0, F_GETFL, 0);
	char c = 0;

	

	int running = 1;

loop:
	compute();

	c = 0;
	fcntl(0, F_SETFL, flags | O_NONBLOCK);
	read(0, &c, 1);
	fcntl(0, F_SETFL, flags & ~O_NONBLOCK);


	

	if (c == 'Q') running = 0;


if (not inside_inv) {

	if (c == 'r') move_player_up();
	if (c == 'h') move_player_down();
	if (c == 's') move_player_left();
	if (c == 't') move_player_right();
	
	if (c == 'k') { break_block(); }
	if (c == 'l') { place_block(); }

	if (c == 'u') { pd = 0; }
	if (c == 'e') { pd = 1; }
	if (c == 'n') { pd = 2; }
	if (c == 'o') { pd = 3; }
	
	if (c == 'd') {  } // pick block?... 
	if (c == ' ') {  }

	// add grab block functionality for player too!

} else {

	if (c == 'r') move_inv_up();
	if (c == 'h') move_inv_down();
	if (c == 's') move_inv_left();
	if (c == 't') move_inv_right();
	
	//if (c == 'k') { grab_block(); }
	//if (c == 'l') { put_block(); }

	if (c == 'u') { invd = 0; }
	if (c == 'e') { invd = 1; }
	if (c == 'n') { invd = 2; }
	if (c == 'o') { invd = 3; }

}

	if (c == '0') display_debug = not display_debug;
	if (c == ' ') inside_inv = not inside_inv;


	display();
	usleep(2000);        // "tick speed"              todo: make this tweakable. 

	if (running) goto loop;

	save_game();

	write(1, "\033[1;1H\033[?25h", 12);
	write(1, "\033[?1049l\033[?1000l", 16);	
	tcsetattr(0, TCSAFLUSH, &terminal);

	
}
























// ------------------------------------------ old documentation -----------------------------------------




/*

	okay, so i actually realized where i left off in making this game-    i actually left off at the character itself!
	
		ie, i dont want to implement movement like any other game- i want to make the player actually walk around using muscles, and stuff like that, basically... it will be quite hard.. 


		the idea is that i have a brain block, which the camera is always focused on, and nerve blocks which transmit signals to the muscle blocks, which actually move things, or rather, excert a force on other blocks, i guess. 



			its rather tricky, because i dont know what the right formulation is, in order to get forces, and stuff.. 


					i think its as simple, as like.. f = ma,   where a is accereration, ie, change in velocity. and m is mass, or ie, some proportionality constant under which the object changes, i think?


						hmm... i also know that i want to model binding force some how, how dirt kind of like binds to itself...



				i dont really know what the right way to do this is... 
























old code:









// a dirt flat world.
	for (int y = ysize; y-- > (ysize-10);) {
		for (int x = 0; x < xsize; x++) {
			*at(x,y,0,0) = dirt;
		}
	}


	// a top layer of grass.
	for (int x = 0; x < xsize; x++) {
		*at(x,(ysize - 10),0,0) = rand() % 2 ? grass : air;
	}


	// some falling dirt, for fun.
	for (int x = 0; x < xsize; x++) {
		*at(x,(ysize - 22),0,0) = (rand() % 2) * (rand() % 2) ? dirt : air;
	}


	// bottom layer of bedrock, that isnt affected by gravity.
	for (int x = 0; x < xsize; x++) {
		*at(x,ysize - 1,0,0) = bedrock;
	}




// do gravity

	for (int y = ysize; y--;) {
		for (int x = 0; x < xsize; x++) {

			int8_t* block = at0(x,y);
			int8_t* below = at(x,y,0,1);

			if (*below == air) {
				if (*block == player) {
					// *block = air;
					// *below = player;
					// py++; if (py == ysize) py = 0;

				} else if (*block != bedrock) { // every block besides bedrock has gravity.
					// *below = *block;
					// *block = air;
				}
			}				
		}
	}






for (nat i = 0; i < max_spawn_attempts; i++) {
		if (not *at(invhomex,invhomey,x,y)) goto done;
		x = rand() % (2 * maximum_inventory_radius) - maximum_inventory_radius;
		y = rand() % (2 * maximum_inventory_radius) - maximum_inventory_radius;
	}
*/

