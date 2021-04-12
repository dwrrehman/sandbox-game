// a 2D, minecraft-like, top-down sandbox game, 
//   made by daniel warren riaz rehman, 
//     on 2104121.124522 

// ----------bugs:-----------

// - found a bug where the line above you is shifted to amke your player seem on the left.... hmm... 
// 

// ------------- features: -----------------


//  get a frames per second counter, and display it.

// add more blocks!    
// add water! 
// add sand! 
// add granite!   
// add pebbles,
// add plants! flowers!
// add trees somehow! ?....

// ----------- done features: -------------------
// 
// 	 save/serialize and also load/open binary world file!      .state file   


// tb:

//TODO: move the player block to the proper location.
// player block is going to be 255, i think. rogues are going to be 254, i think.
// i think i want a possible attach to be like... shooting at you! 
// and then simulate a projectile flying through the air. i also want it to be 
// like... direcional: > v < ^ for the various direcional projectiles.
// but, also i want melee combat with the rogues, to be neccessary.
// and i want their health and strength to vary, and correlate.

// also i want to add wiring! somehow. im not sure how... 
// i think i need wire, an then a junction, and then a cross-over (nonjunction).
// and then i also need an inverter, i think, or a transister, maybe.
// hmm... 

// -------------- controls: --------------------

// - tab to save and quit.
// - delete to just quit, without saving.

// w a s d   or  W A S D  to move.

// I J K L    to place the currently selected item in your inventory.
// i j l k    to break blocks, and put them in your inventory

// < >    to change which inventory item is selected.

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

typedef size_t nat;
typedef ssize_t integer;
typedef uint8_t byte;
typedef unsigned int uint;

// generation parameters;
static const nat default_inventory_size = 8;

static const nat max_rogue_health = 16;

static const nat max_player_health = 12;
static const nat initial_player_health = 10;

static const nat rogue_spawn_attempts = 100;
static const nat player_spawn_attempts = 1000;

enum blocks {
	air_block = 0,
	dirt_block = 1,
	grass_block = 2,
	stone_block = 3,
	glass_block = 4,

	total_block_count,
};

enum items {
	no_item = 0,
	dirt_item = 1,
	grass_item = 2,
	stone_item = 3,
	glass_item = 4, 

	total_item_count,
};


static const char* visualize_player = " ¶";
static const char* visualize_rogue = " R";

static const char* visualize_block[total_block_count] =             {"  ", "██", "██", "██", "██", }; 	// ☐☐
static const nat visualize_block_color[total_block_count] =         {0,     136,  64,  240,  250,  };

static const char* visualize_item[total_item_count] =               {" ",     "d",   "G",  "s",  ":", };
static const nat visualize_item_color[total_item_count] =           {249,     136,  64,  240,  250,  };

struct slot {
	byte item;
	byte count;
};

struct player {
	struct slot* inventory;
	nat x;
	nat y;
	nat health;
	nat selected;
};

struct universe {
	byte* state;
	nat count;
	nat side; // side length of the sqaure world.

	struct player* players; // only one element, for now.
	// todo: support multi-player, make this server-client based.
	struct player* rogues;

	nat player_count; // = 1, for now.
	nat rogue_count;
	nat inventory_size;
};

static nat window_rows = 0;
static nat window_columns = 0;
static char* screen = NULL;

static char message[4096] = {0};

static struct universe universe = {0};

static inline struct termios configure_terminal() {
	struct termios terminal = {0};
	tcgetattr(0, &terminal);
	struct termios raw = terminal;
	raw.c_oflag &= ~( (unsigned long)OPOST );
	raw.c_iflag &= ~( (unsigned long)BRKINT 
			| (unsigned long)ICRNL 
			| (unsigned long)INPCK 
			| (unsigned long)IXON );	
	raw.c_lflag &= ~( (unsigned long)ECHO 
			| (unsigned long)ICANON 
			| (unsigned long)IEXTEN );

	raw.c_cc[VMIN] = 0;
  	raw.c_cc[VTIME] = 1;

	tcsetattr(0, TCSAFLUSH, &raw);
	return terminal;
}

static inline void adjust_window_size() {
	struct winsize window = {0};
	ioctl(1, TIOCGWINSZ, &window);

	if (window.ws_row == 0 or window.ws_col == 0) { window.ws_row = 20; window.ws_col = 40; }

	if (window.ws_row != window_rows or window.ws_col != window_columns) {
		window_rows = window.ws_row;
		window_columns = window.ws_col;
		screen = realloc(screen, sizeof(char) * (size_t) (4 * window_rows * (window_columns * 4 + 16)));
	}
}

static inline nat at(integer x_offset, integer y_offset) {
	const nat S = universe.side;
	return  
		S * ((universe.players[0].x + (nat)(x_offset + (integer)S)) % S)
		+ (universe.players[0].y + (nat)(y_offset + (integer)S)) % S;
}

static inline size_t at_position(nat origin_x, nat origin_y, integer x_offset, integer y_offset) {
	const nat S = universe.side;
	return  
		S * ((origin_x + (nat)(x_offset + (integer)S)) % S)
		+ (origin_y + (nat)(y_offset + (integer)S)) % S;
}

static inline void display() {
	nat length = 3; 
	memcpy(screen, "\033[H", 3);

	const integer height = (integer)window_rows / 2;     
	const integer width = (integer)window_columns / 4;   // divide by 2 from each block being two chars, and 
		   				    // also another for the fact that we go from -height to +height.
	for (integer x = -height + 1; x < height; x++) {
		for (integer y = -width; y < width; y++) {

			byte block = universe.state[at(x, y)];

			bool at_rogue = false, at_player = false;

			for (nat i = 0; i < universe.rogue_count; i++) {
				if (at(x,y) == at_position(universe.rogues[i].x,universe.rogues[i].y, 0, 0)) {
					at_rogue = true;
					break;
				}
			}
			
			for (nat i = 1; i < universe.player_count; i++) {
				if (at(x,y) == at_position(universe.players[i].x,universe.players[i].y, 0, 0)) {
					at_player = true;
					break;
				}
			}

			if (not x and not y) {
				length += (nat)sprintf(screen + length, "\033[38;5;%lum%s\033[m", 69L, visualize_player);

			} else if (at_rogue) {			
				length += (nat)sprintf(screen + length, "\033[38;5;%lum%s\033[m", 9L, visualize_rogue);

			} else if (at_player) {
				length += (nat)sprintf(screen + length, "\033[38;5;%lum%s\033[m", 235L, visualize_player);
			
			} else if (block == air_block) {
				length += (nat)sprintf(screen + length, "  ");
			} else {
				length += (nat)sprintf(screen + length, "\033[38;5;%lum%s\033[m", 
							visualize_block_color[block], visualize_block[block]); 
			}
		}
		screen[length++] = '\033'; 		// (print new line)
		screen[length++] = '[';
		screen[length++] = 'K';
		screen[length++] = '\r';
		screen[length++] = '\n';
	}

	const struct player player = universe.players[0];
	nat status_length = 0, status_visual_length = 0;

	nat _y = (nat)sprintf(screen + length + status_length, " (%lu, %lu)     \033[38;5;%lum", player.x, player.y, 9L);
	status_length += _y;
	status_visual_length += _y;

	for (nat i = 0; i < max_player_health; i++) {
		status_length += (nat)sprintf(screen + length + status_length, i < player.health ? "♥ " : "♡ ");
		status_visual_length += 2;
	}
	status_length += (nat)sprintf(screen + length + status_length, "\033[m  ");
	status_visual_length += 3;

	for (nat i = 0; i < universe.inventory_size; i++) {

		nat _l = (nat)sprintf(screen + length + status_length, i == player.selected ? "(" : " ");
		status_length += _l;
		status_visual_length += _l;

		if (player.inventory[i].count) {
			nat _k = (nat)sprintf(screen + length + status_length, "\033[38;5;%lum%s\033[m(%2d)", 

				visualize_item_color[player.inventory[i].item],
				visualize_item[player.inventory[i].item], 
				player.inventory[i].count);

			status_length += _k;
			status_visual_length += _k;
		} else {
			nat _k = (nat)sprintf(screen + length + status_length, "\033[38;5;%lum%s\033[m", 

				visualize_item_color[player.inventory[i].item],
				visualize_item[player.inventory[i].item]);

			status_length += _k;
			status_visual_length += _k;
		}
	
		nat _ll = (nat)sprintf(screen + length + status_length, i == player.selected ? ")" : " ");
		status_length += _ll;
		status_visual_length += _ll;
	}

	nat _h = (nat)sprintf(screen + length + status_length, "%s", message);
	status_length += _h;
	status_visual_length += _h;

	length += status_length;

	for (nat i = status_visual_length; i < window_columns; i++)
		screen[length++] = ' ';

	screen[length++] = '\033';
	screen[length++] = '[';
	screen[length++] = 'm';

	write(1, screen, (size_t) length);
}

static inline void generate_universe() {
	for (nat i = 0; i < universe.count; i++) {
		universe.state[i] = (rand() % total_block_count) * (rand() % 2);
	}
}

static inline void spawn_rogue() {
	nat x = 0, y = 0;
	for (nat i = 0; i < rogue_spawn_attempts; i++) {
		x = (nat)rand() % universe.side;
		y = (nat)rand() % universe.side;
		if (
			universe.state[at_position(x, y, 1, 1)] or 
			universe.state[at_position(x, y, 1, -1)] or 
			universe.state[at_position(x, y, -1, 1)] or 
			universe.state[at_position(x, y, -1, -1)] or 

			universe.state[at_position(x, y, 0, -1)] or 
			universe.state[at_position(x, y, -1, 0)] or 
			universe.state[at_position(x, y, 1, 0)] or 
			universe.state[at_position(x, y, 0, 1)] or 

			universe.state[at_position(x, y, 0, 0)] 
		) continue; else {
			universe.rogues = realloc(universe.rogues, sizeof(struct player) * (universe.rogue_count + 1));
			universe.rogues[universe.rogue_count].x = x;
			universe.rogues[universe.rogue_count].y = y;
			universe.rogues[universe.rogue_count].health = (nat) rand() % max_rogue_health;
			universe.rogues[universe.rogue_count].inventory = calloc(sizeof(struct slot), universe.inventory_size);
			universe.rogue_count++;
			return;
		}
	}
	strcpy(message, "debug: rogue spawn attempt failed");
}

static inline void spawn_player() {
	nat x = 0, y = 0;
	for (nat i = 0; i < player_spawn_attempts; i++) {
		x = (nat)rand() % universe.side;
		y = (nat)rand() % universe.side;
		if (
			universe.state[at_position(x, y, 1, 1)] or 
			universe.state[at_position(x, y, 1, -1)] or 
			universe.state[at_position(x, y, -1, 1)] or 
			universe.state[at_position(x, y, -1, -1)] or 

			universe.state[at_position(x, y, 0, -1)] or 
			universe.state[at_position(x, y, -1, 0)] or 
			universe.state[at_position(x, y, 1, 0)] or 
			universe.state[at_position(x, y, 0, 1)] or 

			universe.state[at_position(x, y, 0, 0)] 
		) continue; else {
			universe.players = realloc(universe.players, sizeof(struct player) * (universe.player_count + 1));
			universe.players[universe.player_count].x = x;
			universe.players[universe.player_count].y = y;
			universe.players[universe.player_count].health = initial_player_health;
			universe.players[universe.player_count].inventory = calloc(sizeof(struct slot), universe.inventory_size);
			universe.player_count++;
			return;
		}
	}
	printf("could not spawn player after %lu attempts, aborting...\n", player_spawn_attempts);
	abort();
}

static inline void create_universe(nat size) {
	universe.side = size;
	universe.count = universe.side * universe.side;
	universe.inventory_size = default_inventory_size;

	universe.state = malloc(universe.count);
	generate_universe();
	
	universe.rogue_count = 0;
	universe.player_count = 0;
	universe.players = NULL;
	universe.rogues = NULL;
	
	spawn_player();
}

static inline void load_universe(const char* filename) {

	FILE* file = fopen(filename, "r");

	if (not file) { 
		perror("open"); 
		exit(3); 
	}

	uint side = 0, player_count = 0, rogue_count = 0, inv_size = 0;
	fread(&side, sizeof(uint), 1, file);
	fread(&player_count, sizeof(uint), 1, file);
	fread(&rogue_count, sizeof(uint), 1, file);
	fread(&inv_size, sizeof(uint), 1, file);
	
	universe.side = side;
	universe.count = universe.side * universe.side;
	universe.state = malloc(universe.count);

	universe.player_count = player_count;
	universe.rogue_count = rogue_count;
	universe.inventory_size = inv_size;

	universe.players = malloc(player_count * sizeof(struct player));
	universe.rogues = malloc(rogue_count * sizeof(struct player));

	for (nat i = 0; i < player_count; i++) {
		uint x = 0, y = 0, h = 0, s = 0;
		fread(&x, sizeof(uint), 1, file);
		fread(&y, sizeof(uint), 1, file);
		fread(&h, sizeof(uint), 1, file);
		fread(&s, sizeof(uint), 1, file);
		
		struct player player = {
			.inventory = malloc(universe.inventory_size * sizeof(struct slot)),
			.x = x, .y = y, .health = h, .selected = s,
		};

		for (nat j = 0; j < universe.inventory_size; j++) {
			byte item = 0, count = 0;
			fread(&item, sizeof(byte), 1, file);
			fread(&count, sizeof(byte), 1, file);
			player.inventory[j].item = item;
			player.inventory[j].count = count;
		}
		universe.players[i] = player;
	}

	for (nat i = 0; i < rogue_count; i++) {
		uint x = 0, y = 0, h = 0, s = 0;
		fread(&x, sizeof(uint), 1, file);
		fread(&y, sizeof(uint), 1, file);
		fread(&h, sizeof(uint), 1, file);
		fread(&s, sizeof(uint), 1, file);
		
		struct player rogue = {
			.inventory = malloc(universe.inventory_size * sizeof(struct slot)),
			.x = x, .y = y, .health = h, .selected = s,
		};

		for (nat j = 0; j < universe.inventory_size; j++) {
			byte item = 0, count = 0;

			fread(&item, sizeof(byte), 1, file);
			fread(&count, sizeof(byte), 1, file);
			
			rogue.inventory[j].item = item;
			rogue.inventory[j].count = count;
		}
		universe.rogues[i] = rogue;
	}
	fread(universe.state, sizeof(byte), universe.count, file);
	fclose(file);
}

static inline void save_universe(const char* destination) {

	FILE* file = fopen(destination, "w");

	if (not file) { 
		perror("open"); 
		exit(3); 
	}

	uint 
		side = (uint) universe.side, 
		player_count = (uint) universe.player_count, 
		rogue_count = (uint) universe.rogue_count, 
		inv_size = (uint) universe.inventory_size;

	fwrite(&side, sizeof(uint), 1, file);
	fwrite(&player_count, sizeof(uint), 1, file);
	fwrite(&rogue_count, sizeof(uint), 1, file);
	fwrite(&inv_size, sizeof(uint), 1, file);

	for (nat i = 0; i < player_count; i++) {

		uint 
			x = (uint) universe.players[i].x, 
			y = (uint) universe.players[i].y, 
			h = (uint) universe.players[i].health, 
			s = (uint) universe.players[i].selected;

		fwrite (&x, sizeof(uint), 1, file);
		fwrite(&y, sizeof(uint), 1, file);
		fwrite(&h, sizeof(uint), 1, file);
		fwrite(&s, sizeof(uint), 1, file);

		for (nat j = 0; j < universe.inventory_size; j++) {

			byte 
				item = universe.players[i].inventory[j].item, 
				count = universe.players[i].inventory[j].count;

			fwrite(&item, sizeof(byte), 1, file);
			fwrite(&count, sizeof(byte), 1, file);
		}
	}

	for (nat i = 0; i < rogue_count; i++) {

		uint 
			x = (uint) universe.rogues[i].x, 
			y = (uint) universe.rogues[i].y, 
			h = (uint) universe.rogues[i].health, 
			s = (uint) universe.rogues[i].selected;

		fwrite (&x, sizeof(uint), 1, file);
		fwrite(&y, sizeof(uint), 1, file);
		fwrite(&h, sizeof(uint), 1, file);
		fwrite(&s, sizeof(uint), 1, file);

		for (nat j = 0; j < universe.inventory_size; j++) {

			byte 
				item = universe.rogues[i].inventory[j].item, 
				count = universe.rogues[i].inventory[j].count;

			fwrite(&item, sizeof(byte), 1, file);
			fwrite(&count, sizeof(byte), 1, file);
		}
	}

	fwrite(universe.state, sizeof(byte), universe.count, file);

	fclose(file);
}


static inline void break_block(nat at) {

	byte block = universe.state[at];

	if (block != air_block) {
	
		for (nat i = 0; i < universe.inventory_size; i++) {
			if (universe.players[0].inventory[i].item == block) {
				universe.players[0].inventory[i].count++;
				universe.players[0].selected = i;
				universe.state[at] = air_block;
				return;

			} else if (universe.players[0].inventory[i].item == no_item) {
				universe.players[0].inventory[i].item = block;
				universe.players[0].inventory[i].count = 1;
				universe.players[0].selected = i;
				universe.state[at] = air_block;
				return;
			}
		}		
	} 
}

static inline void place_block(nat at) {
	if (universe.players[0].inventory[universe.players[0].selected].count 
		and universe.state[at] == air_block) {
		universe.state[at] = universe.players[0].inventory[universe.players[0].selected].item;
		universe.players[0].inventory[universe.players[0].selected].count--;
	}
}

static inline void pathfind_rogue() {
	
}

static inline void compute() {

	const nat rogue_spawn_modulus = 512;

	// for (size_t i = 0; i < universe.count; i++) {
	// 	if (rand() % 128 == 0 and universe.state[i]) {
	// 		universe.state[i]--;
	// 	}
	// }

	if ((nat) rand() % rogue_spawn_modulus == 0) {
		spawn_rogue();
	}

	pathfind_rogue();
}


int main(int argc, const char** argv) {

	srand((unsigned)time(0));

	if (argc <= 1) usage: exit(puts( "usage: \n"
			"\t./universe create <new_filename: string> <size: nat>\n"
			"\t./universe load <existing_filename: string>\n"
			));
	
	else if (not strcmp(argv[1], "create")) create_universe((nat)atol(argv[3]));
	else if (not strcmp(argv[1], "load")) load_universe(argv[2]);
	else goto usage;

	struct termios terminal = configure_terminal();
	write(1, "\033[?1049h\033[?1000l", 16); // use alternate screen, then disable mouse
	write(1, "\033[?25l", 6); // hide cursor.
	adjust_window_size();

	char c = 0;
	bool quit = false;
	while (not quit) {

		compute();
		adjust_window_size();
		display();
		ssize_t n = read(0, &c, 1);

		if (n == 0) c = 0;

		if (c == 9) {
			save_universe(argv[2]);
			quit = true; 			// tab to save and then quit.

		} else if (c == 127) quit = true;  	// delete to quit without saving.
		
		else if (c == 'w' or c == 'W') {
			if (not universe.state[at(-1,0)]) {
				universe.players[0].x = (universe.players[0].x + universe.side - 1) % universe.side;
			}
			

		} else if (c == 's' or c == 'S') {
			if (not universe.state[at(1,0)]) {
				universe.players[0].x = (universe.players[0].x + 1) % universe.side;
			}

		} else if (c == 'a' or c == 'A') {
			if (not universe.state[at(0,-1)]) {
				universe.players[0].y = (universe.players[0].y + universe.side - 1) % universe.side;
			}

		} else if (c == 'd' or c == 'D') {
			if (not universe.state[at(0,1)]) {
				universe.players[0].y = (universe.players[0].y + 1) % universe.side;
			}
		} 

		else if (c == 'i') break_block(at(-1, 0));
		else if (c == 'k') break_block(at(1, 0));
		else if (c == 'j') break_block(at(0, -1));
		else if (c == 'l') break_block(at(0, 1));

		else if (c == 'I') place_block(at(-1, 0));
		else if (c == 'K') place_block(at(1, 0));
		else if (c == 'J') place_block(at(0, -1));
		else if (c == 'L') place_block(at(0, 1));

		else if (c == '<') {
			if (universe.players[0].selected) universe.players[0].selected--;
		} else if (c == '>') {
			if (universe.players[0].selected < universe.inventory_size - 1) universe.players[0].selected++;
		} else if (c == ';') {
			sprintf(message, "%lu rogues, %lu players side=%lu:count=%lu:inv=%lu  ", 
				universe.rogue_count, universe.player_count, 
				universe.side, universe.count, universe.inventory_size); 
		}

		if (not universe.players[0].inventory[universe.players[0].selected].count) 
			universe.players[0].inventory[universe.players[0].selected].item = no_item;
		usleep(10000);
	}

	write(1, "\033[?1049l\033[?1000l", 16);	  // goto main screen, then keep mouse disabled.
	write(1, "\033[?25h", 6); // show cursor again.
	tcsetattr(0, TCSAFLUSH, &terminal);

}


