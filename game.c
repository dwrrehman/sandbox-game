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

/// --------------------------- core data types -----------------------------


typedef size_t nat;
typedef unsigned int uint;
typedef uint8_t byte;

typedef ssize_t integer;


/// ---------------------- constants / parameters: -----------------------


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

enum gamemodes {
	creative_mode,
	survival_mode,
};

enum rogue_states {
	wandering_state,
	searching_state,
};

static const byte default_gamemode = survival_mode;
static const byte default_inventory_size = 8;

static const byte max_rogue_health = 16;
static const byte max_player_health = 12;
static const byte initial_player_health = 10;

static const nat rogue_spawn_attempts = 100;
static const nat player_spawn_attempts = 1000;

static const nat astar_cost_limit = 100;
static const nat rogue_dormant_radius = 50;

static const nat rogue_spawn_modulus = 128;
static const nat maximum_rogue_count = 1;

static const char* visualize_player = " ¶";
static const char* visualize_rogue = " R";
static const char* visualize_rogue_path = " ☐";

static const char* visualize_block[total_block_count] =             {"  ", "██", "██", "██", "██", }; 	// ☐☐
static const nat visualize_block_color[total_block_count] =         {0,     136,  64,  240,  250,  };

static const char* visualize_item[total_item_count] =               {" ",     "d",   "G",  "s",  ":", };
static const nat visualize_item_color[total_item_count] =           {249,     136,  64,  240,  250,  };


/// ---------------------- structures -----------------------


struct slot {
	byte item;
	byte count;
};

struct point {
	nat x;
	nat y;
};

struct node {
	struct node* parent;
	struct point point;
	nat f;
	nat g;
	nat h;
};


struct point_list {
	struct point* list;
	nat count;
};


struct node_list {
	struct node* list;
	nat count;
};

struct player {
	struct slot* inventory;
	struct point location;
	uint32_t _reserved0;
	byte health;
	byte selected;
	byte gamemode;
	byte _reserved1;
};

struct rogue {
	struct slot* inventory;

	struct point_list path;
	struct point location;
	struct point target;

	nat lifetime;
	uint32_t reserved0;

	byte health;
	byte strength;
	byte target_valid;
	byte state;
};

struct universe {

	byte* state;
	nat count;

	nat side;

	nat inventory_size;

	struct player* players; 	// only 1 element, for now.
	nat player_count;		// todo: support multi-player, make this server-client based.

	struct rogue* rogues;
	nat rogue_count;
};


/// ---------------------- globals -----------------------


static nat window_rows = 0;
static nat window_columns = 0;

static char* screen = NULL;
static char message[4096] = {0};

static struct universe universe = {0};




/// ---------------------- helpers -----------------------


#define  pathfinding_failure  (struct point_list) {0};

static inline nat min(nat a, nat b) {
	return a < b ? a : b;
}

static inline bool points_equal(struct point a, struct point b) {
	return a.x == b.x and a.y == b.y;
}


static inline void push_node(struct node_list* list, struct node node) {
	list->list = realloc(list->list, sizeof(struct node) * (list->count + 1));
	list->list[list->count++] = node;
}

static inline void push_point(struct point_list* list, struct point point) {
	list->list = realloc(list->list, sizeof(struct point) * (list->count + 1));
	list->list[list->count++] = point;	
}

static inline bool node_is_not_in_list(struct node_list list, struct node element) {
	for (nat i = 0; i < list.count; i++) {
		if (points_equal(list.list[i].point, element.point)) return false;
	}
	return true;
}

static inline bool point_is_not_in_list(struct point_list list, struct point element) {
	for (nat i = 0; i < list.count; i++) {
		if (points_equal(list.list[i], element)) return false;
	}
	return true;
}

// static inline nat at_x_y(nat x, nat y) {
// 	return universe.side * x + y;
// }

static inline nat at_point(struct point p) {
	return universe.side * p.x + p.y;
}

static inline nat at_player(integer x_offset, integer y_offset) {
	const nat S = universe.side;
	return  
		S * ((universe.players[0].location.x + (nat)(x_offset + (integer)S)) % S)
		+ (universe.players[0].location.y + (nat)(y_offset + (integer)S)) % S;
}

static inline size_t at_position(nat origin_x, nat origin_y, integer x_offset, integer y_offset) {
	const nat S = universe.side;
	return  
		S * ((origin_x + (nat)(x_offset + (integer)S)) % S)
		+ (origin_y + (nat)(y_offset + (integer)S)) % S;
}













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

static inline void display() {
	nat length = 3; 
	memcpy(screen, "\033[H", 3);

	const integer height = (integer)window_rows / 2;     
	const integer width = (integer)window_columns / 4;   // divide by 2 from each block being two chars, and 
		   				    // also another for the fact that we go from -height to +height.
	for (integer x = -height + 1; x < height; x++) {
		for (integer y = -width; y < width; y++) {

			nat ap = at_player(x,y);
			byte block = universe.state[ap];

			bool at_rogue = false, at_other_player = false, in_rogue_path = false;

			for (nat i = 0; i < universe.rogue_count; i++) {
				if (ap == at_point(universe.rogues[i].location)) {
					at_rogue = true;
					break;
				}
				for (nat p = 0; p < universe.rogues[i].path.count; p++) {
					if (ap == at_point(universe.rogues[i].path.list[p])) {
						in_rogue_path = true;
						goto double_break;
					}
				}
			}
			
			for (nat i = 1; i < universe.player_count; i++) {
				if (ap == at_point(universe.players[i].location)) {
					at_other_player = true;
					break;
				}
			}

			double_break:

			if (not x and not y) {
				length += (nat)sprintf(screen + length, "\033[38;5;%lum%s\033[m", 69L, visualize_player);

			} else if (at_rogue) {			
				length += (nat)sprintf(screen + length, "\033[38;5;%lum%s\033[m", 9L, visualize_rogue);

			} else if (at_other_player) {
				length += (nat)sprintf(screen + length, "\033[38;5;%lum%s\033[m", 235L, visualize_player);

			} else if (in_rogue_path) {
				length += (nat)sprintf(screen + length, "\033[38;5;%lum%s\033[m", 231L, visualize_rogue_path);
			
			
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

	nat _y = (nat)sprintf(screen + length + status_length, " (%lu, %lu)     \033[38;5;%lum", player.location.x, player.location.y, 9L);
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

	if (universe.rogue_count >= maximum_rogue_count) return;

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

			struct rogue new = {
				.inventory = calloc(sizeof(struct slot), universe.inventory_size),
				.path = (struct point_list) {0},

				.location = (struct point){.x = x, .y = y},
				.target = (struct point){0},

				.lifetime = 256,

				.health = (byte) rand() % max_rogue_health,
				.strength = (byte) rand() % max_rogue_health,
				.target_valid = false,

				.state = 0,
			};

			universe.rogues = realloc(universe.rogues, sizeof(struct rogue) * (universe.rogue_count + 1));
			universe.rogues[universe.rogue_count++] = new;
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

			struct player new = {
				.inventory = calloc(sizeof(struct slot), universe.inventory_size),
				.location = (struct point){.x = x, .y = y},
				.health = initial_player_health,
				.selected = 0,
				.gamemode = default_gamemode,
			};

			universe.players = realloc(universe.players, sizeof(struct player) * (universe.player_count + 1));
			universe.players[universe.player_count++] = new;
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
	spawn_rogue();
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
	universe.rogues = malloc(rogue_count * sizeof(struct rogue));

	for (nat i = 0; i < player_count; i++) {

		uint x = 0, y = 0;
		byte h = 0, s = 0;

		fread(&x, sizeof(uint), 1, file);
		fread(&y, sizeof(uint), 1, file);

		fread(&h, sizeof(byte), 1, file);
		fread(&s, sizeof(byte), 1, file);
		
		struct player player = {
			.inventory = malloc(universe.inventory_size * sizeof(struct slot)),
			.location = (struct point) {.x = x, .y = y}, .health = h, .selected = s,
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

		uint x = 0, y = 0;     ///TODO: 
		byte h = 0, s = 0;

		fread(&x, sizeof(uint), 1, file);
		fread(&y, sizeof(uint), 1, file);

		fread(&h, sizeof(byte), 1, file);
		fread(&s, sizeof(byte), 1, file);
	

		struct rogue rogue = {
			.inventory = calloc(sizeof(struct slot), universe.inventory_size),
			.path = (struct point_list) {0},

			.location = (struct point){.x = x, .y = y},
			.target = (struct point){0},

			.lifetime = 256,

			.health = (byte) rand() % max_rogue_health,
			.strength = (byte) rand() % max_rogue_health,
			.target_valid = false,

			.state = 0,
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
			x = (uint) universe.players[i].location.x, 
			y = (uint) universe.players[i].location.y, 
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
			x = (uint) universe.rogues[i].location.x, 
			y = (uint) universe.rogues[i].location.y;

		byte
			h = (byte) universe.rogues[i].health, 
			s = (byte) universe.rogues[i].strength;

		fwrite(&x, sizeof(uint), 1, file);
		fwrite(&y, sizeof(uint), 1, file);
		fwrite(&h, sizeof(byte), 1, file);
		fwrite(&s, sizeof(byte), 1, file);

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
				universe.state[at] = air_block;
				return;

			} else if (universe.players[0].inventory[i].item == no_item) {
				universe.players[0].inventory[i].item = block;
				universe.players[0].inventory[i].count = 1;
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












// ------------------------ pathfinding -------------------------------


// static inline struct node* deep_copy(struct node this) {
// 	struct node* copy = malloc(sizeof(struct node));
// 	*copy = this;
// 	if (this.parent) copy->parent = deep_copy(*this.parent);
// 	return copy;
// }

static inline struct node_list get_valid_path_neighbors(struct node origin, nat g) {

	const nat S = universe.side;

	struct node* successors = malloc(9 * sizeof(struct node));
	nat successors_count = 0;

	for (integer i = -1; i <= 1; i++) {
		for (integer j = -1; j <= 1; j++) {

			// struct node* copy = deep_copy(origin);

			struct node* copy = malloc(sizeof(struct node));
			*copy = origin;

			struct node p = {
				.parent = copy,
				.point = (struct point){(origin.point.x + (nat)(i + (integer)S)) % S, 
							(origin.point.y + (nat)(j + (integer)S)) % S},
				.f = 0, .g = g + 1, .h = 0,
			};

			if (((i and not j) or (not i and j)) and not universe.state[at_point(p.point)]) {
				successors[successors_count++] = p;
			}
		}
	}

	return (struct node_list) {.list = successors, .count = successors_count};
}



static inline nat distance(struct point a, struct point b) { 
	const nat S = universe.side;
	return (nat)
	(min (
		(nat)labs((integer)a.x - (integer)b.x), 
		S - (nat)labs((integer)b.x - (integer)a.x)
	) + min (
		(nat)labs((integer)a.y - (integer)b.y), 
		S - (nat)labs((integer)b.y - (integer)a.y)
	));
}

static inline struct point_list construct_path(struct node current_initial) {

	struct point_list path = {0};
	struct point_list stack = {0};
	struct node current = current_initial;

	while (current.parent) {
		push_point(&stack, current.point);
		current = *current.parent;
	}

	for (nat i = stack.count; i--;) {
		push_point(&path, stack.list[i]);
	}

	return path;
}

static inline int compare_nodes(const void* a_raw, const void* b_raw) {
	const struct node* a_node = a_raw;
	const struct node* b_node = b_raw;
	return (int)b_node->f - (int)a_node->f;
}

static inline struct point_list astar(struct point start, struct point goal) {

	struct node_list open = {0};
	struct point_list closed = {0};

	const nat h = distance(start, goal);

	push_node(&open, (struct node) {
		.parent = NULL, 
		.point = start, 
		.f = h, 
		.g = 0, 
		.h = h
	});

	while (open.count) {
		qsort(open.list, open.count, sizeof(struct node), compare_nodes);
		struct node current = open.list[--open.count];

		// check to see if we cost too much.
		if (current.g > astar_cost_limit) return pathfinding_failure;

		// check to see if we reached the goal.
		struct node_list goal_neighbors = get_valid_path_neighbors((struct node) {.point = goal}, 0);

		for (nat i = 0; i < goal_neighbors.count; i++) {
			if (points_equal(current.point, goal_neighbors.list[i].point)) return construct_path(current);
		}

		push_point(&closed, current.point);

		struct node_list neighbors = get_valid_path_neighbors(current, current.g);

		for (nat i = 0; i < neighbors.count; i++) {

			struct node neighbor = neighbors.list[i];
		
			neighbor.g = current.g + 1;
    			neighbor.h = distance(neighbor.point, goal);
    			neighbor.f = neighbor.g + neighbor.h;

			if (point_is_not_in_list(closed, neighbor.point) and 
			    node_is_not_in_list(open, neighbor)) 
				push_node(&open, neighbor);

			else {
				struct point_list points = {0};
				for (nat n = 0; n < open.count; n++) {
					if (open.list[n].g > neighbor.g) 
						push_point(&points, open.list[n].point);
				}

				if (not point_is_not_in_list(points, neighbor.point)) {

					for (nat n = 0; n < open.count; n++) {
						if (open.list[n].g > neighbor.g and 
						    points_equal(open.list[n].point, neighbor.point)) {
							open.list[n] = neighbor;
							break;
						}
					}	
				}
			}
		}
	}
	return pathfinding_failure;
}





static inline void find_targets() {

	for (nat r = 0; r < universe.rogue_count; r++) {

		for (nat p = 0; p < universe.player_count; p++) {
			if (universe.players[p].gamemode == survival_mode) {
				universe.rogues[r].target = universe.players[p].location;
				universe.rogues[r].state = searching_state;
				universe.rogues[r].target_valid = true;
			}
		}
	}
}

static inline void move_rogues() {

	for (nat r = 0; r < universe.rogue_count; r++) {

		struct rogue rogue = universe.rogues[r];
		bool still = false;
		bool visible = distance(rogue.location, rogue.target) < rogue_dormant_radius;

		if (rogue.target_valid and rogue.path.count and visible) 
			universe.rogues[r].location = rogue.path.list[0];

		else if (not rogue.path.count) {
			universe.rogues[r].target_valid = false;
			still = true;
		}

		find_targets();

		if (visible) universe.rogues[r].path = astar(rogue.location, rogue.target);
		else still = true;

		// if (still and rogue.lifetime > 0) universe.rogues[r].lifetime--;
		// if (rogue.lifetime == 0) at(rogue.location) = 0;
	}
}

static inline void compute() {

	

	// for (size_t i = 0; i < universe.count; i++) {
	// 	if (rand() % 128 == 0 and universe.state[i]) {
	// 		universe.state[i]--;
	// 	}
	// }

	if ((nat) rand() % rogue_spawn_modulus == 0) {
		spawn_rogue();
	}

	move_rogues();
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
			if (not universe.state[at_player(-1,0)]) {
				universe.players[0].location.x = (universe.players[0].location.x + universe.side - 1) % universe.side;
			}
			
		} else if (c == 's' or c == 'S') {
			if (not universe.state[at_player(1,0)]) {
				universe.players[0].location.x = (universe.players[0].location.x + 1) % universe.side;
			}

		} else if (c == 'a' or c == 'A') {
			if (not universe.state[at_player(0,-1)]) {
				universe.players[0].location.y = (universe.players[0].location.y + universe.side - 1) % universe.side;
			}

		} else if (c == 'd' or c == 'D') {
			if (not universe.state[at_player(0,1)]) {
				universe.players[0].location.y = (universe.players[0].location.y + 1) % universe.side;
			}
		} 

		else if (c == 'i') break_block(at_player(-1, 0));
		else if (c == 'k') break_block(at_player(1, 0));
		else if (c == 'j') break_block(at_player(0, -1));
		else if (c == 'l') break_block(at_player(0, 1));

		else if (c == 'I') place_block(at_player(-1, 0));
		else if (c == 'K') place_block(at_player(1, 0));
		else if (c == 'J') place_block(at_player(0, -1));
		else if (c == 'L') place_block(at_player(0, 1));

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
		usleep(1000);
	}

	write(1, "\033[?1049l\033[?1000l", 16);	  // goto main screen, then keep mouse disabled.
	write(1, "\033[?25h", 6); // show cursor again.
	tcsetattr(0, TCSAFLUSH, &terminal);

}





// static inline std::vector<point> get_path_neighbors(const point& me) {
//     std::vector<point> successors = {};
//     for (int i = -1; i <= 1; i++) {
//         for (int j = -1; j <= 1; j++) {
            
//             point p {
//                 (me.x + i + game.size) % game.size,
//                 (me.y + j + game.size) % game.size
//             };
            
//             if (((i and not j) or (not i and j))) {
//                 successors.push_back(p);
//             }
//         }
//     }
//     return successors;
// }












		// if (std::find(points.begin(), points.end(), neighbor.point) != points.end()) {
  //                   std::vector<point> points = {};
  //                   std::vector<nat> indicies = {};
  //                   nat i = 0;
  //                   for (auto& node : open) {
  //                       if (node.g > neighbor.g and node.point == neighbor.point) {
  //                           points.push_back(node.point);
  //                           indicies.push_back(i);
  //                       }
  //                       i++;
  //                   }
  //                   open[indicies.front()] = neighbor;
  //               }



