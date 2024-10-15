// 1202410093.235441 dwrr
// a 2d game in the terminal
// mulitplayer sandbox game
// to replace the 3d game.

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <iso646.h>
#include <termios.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <time.h>
#include <math.h>
#include <signal.h>
#include <stdnoreturn.h>

typedef uint64_t nat;

static const nat side_length = 400;

enum blocks {

	air,

	player_facing_up,
	player_facing_down,
	player_facing_left,
	player_facing_right,

	water,
	steam,
	ice,
	snow,

	dirt,
	sand,
	mud,
	clay,
	peat,
	silt,
	loam,
	chalk,

	rock,
	pebble,
	limestone,
	granite,
	gneiss,
	gabbro,
	basalt,
	diorite,

	moss,
	mushroom,
	vine,
	white_flower,
	leaves,
	wood,
	root,
	bark,

	wire,
	cross,
	transistor,
	resistor,
	actuator,

	block_type_count
};






static const char* spelling[block_type_count] = {
	" ",

	"^", // player
	"v",
	"<",
	">",

	"#", // water
	"g", // steam
	"#", // ice
	"#", // snow

	"#", // dirt,
	"'", // sand,
	",", // mud,
	"c", // clay,
	"p", // peat,
	"s", // silt,
	"l", // loam,
	"C", // chalk,

	"o", // rock,
	".", // pebble,
	"D", // limestone,
	"G", // granite,
	"G", // gneiss,
	"G", // gabbro,
	"B", // basalt,
	"D", // diorite,

	"_", // moss,
	"m", // mushroom,
	"|", // vine,
	"w", // white_flower,
	"&", // leaves,
	"$", // wood,
	"r", // root,
	"=", // bark,

	"-", // wire,
	"+", // cross,
	"t", // transistor,
	"=", // resistor,
	"A", // actuator,
};

static int color[block_type_count] = {
	0,

	202, // player
	202,
	202,
	202,

	12,  // water
	15,  // steam
	117, // ice
	15,  // snow

	130, // dirt,
	187, // sand,
	94,  // mud,
	145, // clay,
	145, // peat,
	145, // silt,
	145, // loam,
	15,  // chalk,

	247, // rock,
	247, // pebble,
	180, // limestone,
	247, // granite,
	247, // gneiss,
	247, // gabbro,
	240, // basalt,
	251, // diorite,

	107, // moss,
	9, // mushroom,
	107, // vine,
	15, // white_flower,
	70, // leaves,
	136, // wood,
	130, // root,
	130, // bark,

	66, // wire,
	66, // cross,
	123, // transistor,
	124, // resistor,
	120, // actuator,
};

enum player_directions {
	facing_up, facing_down, facing_left, facing_right,
};

static bool displaying = false;
static nat height = 0;
static nat width = 0;
static nat max_length = 0;
static char* screen = NULL;

static uint32_t facing = facing_up;
static uint32_t hand = 0;
// static nat tool = 0;           todo: implement me!!!
static nat player_x = 0;
static nat player_y = 0;

static uint32_t* space = NULL;

static struct winsize window = {0};
static struct termios terminal = {0};

static void restore_terminal(void) {
	tcsetattr(0, TCSAFLUSH, &terminal);
	write(1, "\033[?25h", 6);
}

static void configure_terminal(void) {
	tcgetattr(0, &terminal);
	atexit(restore_terminal);
	struct termios raw = terminal;
	raw.c_lflag &= (unsigned long) (~(ECHO | ICANON));
	raw.c_cc[VMIN] = 0;
	raw.c_cc[VTIME] = 0;
	tcsetattr(0, TCSAFLUSH, &raw);
	write(1, "\033[?25l", 6);
}

static void window_resize_handler(int _) {if(_)_++;
	ioctl(0, TIOCGWINSZ, &window); 
	height = window.ws_row - 2; 
	width = window.ws_col - 2; 
	max_length = (size_t) (128 * width * height);
	if (not displaying) screen = realloc(screen, max_length);
}

static noreturn void interrupt_handler(int _) { if(_)_++;
	restore_terminal(); 
	exit(0); 
}

static uint32_t at_player(int64_t x_off, int64_t y_off) {
	const int64_t s = side_length;
	int64_t x = (int64_t) player_x + x_off;
	int64_t y = (int64_t) player_y + y_off;
	x = (x + 10 * s) % s;
	y = (y + 10 * s) % s;
	return space[y * s + x];
}

static uint32_t* get_at_player(int64_t x_off, int64_t y_off) {
	const int64_t s = side_length;
	int64_t x = (int64_t) player_x + x_off;
	int64_t y = (int64_t) player_y + y_off;
	x = (x + s) % s;
	y = (y + s) % s;
	return space + y * s + x;
}


static void display(void) {

	displaying = true;
	nat length = 3;
	memcpy(screen, "\033[H", 3);

	const int64_t h = height / 2;
	const int64_t w = width / 2;
	for (int64_t i = -h; i < h; i++) {
		for (int64_t j = -w; j < w; j++) {
			const uint32_t b = at_player(j, i);
			length += (nat) snprintf(
				screen + length, 16, 
				"\033[38;5;%dm%s\033[0m", 
				color[b], spelling[b]
			);
		}
		screen[length++] = 033;
		screen[length++] = '[';
		screen[length++] = 'K';
		screen[length++] = 10;
	}

	length += (nat) snprintf(
		screen + length, 128, 
		"\033[0mdebug: facing %u {x=%llu y=%llu} ", 
		facing, player_x, player_y
	);
	screen[length++] = 033;
	screen[length++] = '[';
	screen[length++] = 'K';
	screen[length++] = 10;
	write(1, screen, length);
	displaying = false;
}

static void init(void) {
	const nat total = side_length * side_length;
	space = calloc(total, sizeof(uint32_t));
	for (nat i = 0; i < total; i++) {
		space[i] = ((uint32_t) !(rand() % 16)) * dirt;
	}

	for (uint32_t i = 0; i < block_type_count; i++) 
		space[i] = i;
}

static void tick(void) {
	//const nat total = side_length * side_length;
	//const nat w = (nat) (rand()) % total;
	//space[w] = ((uint32_t) !(rand() % 16)) * 8;
}

static void interact(int64_t x, int64_t y, uint32_t direction) {

	if (not hand) { facing = direction; hand = at_player(x, y); } 
	else if (facing == direction or not at_player(x, y)) {	
		if (facing == facing_up) (*get_at_player(0, -1)) = air;
		if (facing == facing_down) (*get_at_player(0, 1)) = air;
		if (facing == facing_left) (*get_at_player(-1, 0)) = air;
		if (facing == facing_right) (*get_at_player(1, 0)) = air;
		(*get_at_player(x, y)) = hand; 
		hand = 0; facing = direction;
	}
	(*get_at_player(0, 0)) = 1 + facing;
}

static void move_player(int64_t x, int64_t y, uint32_t direction) {
	if (hand) {
		if (facing == direction and not at_player(2 * x, 2 * y)) {
			if (facing == facing_up) (*get_at_player(0, -1)) = air;
			if (facing == facing_down) (*get_at_player(0, 1)) = air;
			if (facing == facing_left) (*get_at_player(-1, 0)) = air;
			if (facing == facing_right) (*get_at_player(1, 0)) = air;
			(*get_at_player(0, 0)) = air;
			player_x = (player_x + side_length + (nat)x) % side_length;
			player_y = (player_y + side_length + (nat)y) % side_length;
			(*get_at_player(0, 0)) = 1 + direction;
			(*get_at_player(x, y)) = hand;
			facing = direction;

		} else if (facing != direction and not at_player(x, y)) {
			if (facing == facing_up) (*get_at_player(0, -1)) = air;
			if (facing == facing_down) (*get_at_player(0, 1)) = air;
			if (facing == facing_left) (*get_at_player(-1, 0)) = air;
			if (facing == facing_right) (*get_at_player(1, 0)) = air;
			(*get_at_player(0, 0)) = 1 + direction;
			(*get_at_player(x, y)) = hand;
			facing = direction;
		}
	} else if (not at_player(x, y)) {
		(*get_at_player(0, 0)) = air; 
		player_x = (player_x + side_length + (nat)x) % side_length;
		player_y = (player_y + side_length + (nat)y) % side_length;
		(*get_at_player(0, 0)) = 1 + direction; 
		facing = direction;
	}
}

int main(void) {
	srand((unsigned) time(0));
	window_resize_handler(0);
	struct sigaction action = {.sa_handler = window_resize_handler}; 
	sigaction(SIGWINCH, &action, NULL);
	struct sigaction action2 = {.sa_handler = interrupt_handler}; 
	sigaction(SIGINT, &action2, NULL);
	configure_terminal();
	
	char c = 0;

	init();
loop:	tick();
	display();
	usleep(20000);

	ssize_t n = read(0, &c, 1);
	if (n == 0) goto loop;
	if (n < 0) { perror("read"); exit(1); }
	if (c == 'q') goto done;

	else if (c == 's') move_player(-1, 0, facing_left);
	else if (c == 't') move_player(1, 0, facing_right);
	else if (c == 'r') move_player(0, -1, facing_up);
	else if (c == 'h') move_player(0, 1, facing_down);

	else if (c == 'n') interact(-1, 0, facing_left);
	else if (c == 'o') interact(1, 0, facing_right);
	else if (c == 'u') interact(0, -1, facing_up);
	else if (c == 'e') interact(0, 1, facing_down);

	else {}
	goto loop;
done:
	printf("exiting..\n");
}

/*

copya do  ./run

copyb insert clang -g -O0 c.c -Weverything -Wno-declaration-after-statement -o run -ferror-limit=1 -fsanitize=address,undefined 








*/












/*




		const bool can_move = 
		(
			facing == direction and not at_player(2 * x, 2 * y) 
		)


		
		or (
			facing != direction and 
			not at_player(x, y) and not at_player(2 * x, 2 * y)
		);
		


		facing = facing_down;
		if (not at_player(0, 1) and (not hand or not at_player(0, 2)) {
			player_y = (player_y + side_length + 1) % side_length;
			(*get_at_player(0, 0)) = player + facing; 
			(*get_at_player(0, 1)) = hand; 
		}

	



		facing = facing_left;
		if (not at_player(-1, 0) and (not hand or not at_player(-2, 0)) {
			player_x = (player_x + side_length - 1) % side_length;
			(*get_at_player(0, 0)) = player + facing; 
			(*get_at_player(-1, 0)) = hand; 
		}
 
		facing = facing_up;
		if (not at_player(0, -1) and (not hand or not at_player(0, -2)) {
			player_y = (player_y + side_length - 1) % side_length;
			(*get_at_player(0, 0)) = player + facing; 
			(*get_at_player(0, -1)) = hand; 
		}


		if ((not at_player(1, 0)) and (not hand or not at_player(2, 0)) {
			
			
			if (facing == facing_right) {
				(*get_at_player(1, 0)) = air;

			
			(*get_at_player(1, 0)) = hand;
			player_x = (player_x + side_length + 1) % side_length;
			(*get_at_player(0, 0)) = player + facing_right; 
			(*get_at_player(1, 0)) = hand; 
		}
		facing = facing_right;






























if (not i and not j) {
				const char player_symbol[4] = {'^', '<', '>', 'v'};
				length += (nat) snprintf(
					screen + length, 16, 
					"\033[38;5;%dm%c\033[0m", 
					202, player_symbol[facing]
				);
			} else 


*/







//static void ____clear_screen(void) { 
	//printf("\033[H\033[2J");
//}
