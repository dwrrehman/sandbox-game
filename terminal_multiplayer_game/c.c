// a 3d terminal game that uses
// raytracing into a 3d block world, 
// done on the cpu over several frames,
// for its graphics. written on 1202410174.181851 dwrr

#include <signal.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdnoreturn.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <iso646.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/ioctl.h>

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
	wire_cross,
	transistor,
	resistor,
	actuator,

	block_type_count
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

struct vec3 { float x, y, z; };

static const float camera_sensitivity = 0.005f;
static const float pi_over_2 = 1.57079632679f;
static const float camera_accel = 0.09f;

static bool displaying = false;
static nat height = 0;
static nat width = 0;
static nat max_length = 0;
static char* screen = NULL;
static uint32_t* space = NULL;
static struct winsize window = {0};
static struct termios terminal = {0};

static struct vec3 position = {0};
static struct vec3 velocity = {0};
static struct vec3 right = {0};
static struct vec3 up = {0};
static struct vec3 forward = {0};
static struct vec3 straight = {0};
static struct vec3 top = {0};
static float yaw = 0;
static float pitch = 0;




/*

static char* read_file(const char* name) {
	const int file = open(name, O_RDONLY, 0);
	if (file < 0) {  perror("open"); exit(1); } 
	const size_t length = (size_t) lseek(file, 0, SEEK_END);
	char* string = calloc(length + 1, 1);
	lseek(file, 0, SEEK_SET);
	read(file, string, length);
	close(file);
	return string;
}

*/



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
	height = 13;	//window.ws_row - 10; 
	width = 33;	//window.ws_col - 20; 
	max_length = (size_t) (128 * width * height);
	if (not displaying) screen = realloc(screen, max_length);
}

static noreturn void interrupt_handler(int _) { if(_)_++;
	restore_terminal(); 
	exit(0); 
}




static inline float inversesqrt(float y) {
	float x2 = y * 0.5f;
	int32_t i = *(int32_t *)&y;
	i = 0x5f3759df - (i >> 1); 	// glm uses a86 for last three digits.
	y = *(float*) &i;
	return y * (1.5f - x2 * y * y);
}

static inline struct vec3 normalize(struct vec3 v) {
	float s = inversesqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	return (struct vec3) {v.x * s, v.y * s, v.z * s};
}

static inline struct vec3 cross(struct vec3 x, struct vec3 y) {
	return (struct vec3) { x.y * y.z - y.y * x.z, x.z * y.x - y.z * x.x, x.x * y.y - y.x * x.y };
}






static void generate_world(void) {
	
	const nat total = side_length * side_length * side_length;
	space = calloc(total, sizeof(nat));
	space[0] = snow;
	space[2] = snow;
	space[5] = snow;
	space[12] = snow;
}

static void tick(void) {
	
}









static void display(void) {

	displaying = true;
	nat length = 3;
	memcpy(screen, "\033[H", 3);

	const int64_t h = height / 2;
	const int64_t w = width / 2;
	for (int64_t i = -h; i < h; i++) {
		for (int64_t j = -w; j < w; j++) {
			const uint32_t b = dirt;
			length += (nat) snprintf(
				screen + length, 16, "\033[38;5;%dm#\033[0m", color[b]
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
		0, 0LLU, 0LLU
	);
	screen[length++] = 033;
	screen[length++] = '[';
	screen[length++] = 'K';
	screen[length++] = 10;
	write(1, screen, length);
	displaying = false;
}


int main(void) {
	srand((unsigned) time(0));
	window_resize_handler(0);
	struct sigaction action = {.sa_handler = window_resize_handler}; 
	sigaction(SIGWINCH, &action, NULL);
	struct sigaction action2 = {.sa_handler = interrupt_handler}; 
	sigaction(SIGINT, &action2, NULL);
	configure_terminal();


	position = (struct vec3) {1, 5, 4};
	velocity = (struct vec3) {0, 0, 0};
	right =    (struct vec3) {1, 0, 0};
	up =       (struct vec3) {0, 1, 0};
	forward =  (struct vec3) {0, 0, 1};
	straight = cross(right, up);
	top = cross(forward, right);
	yaw = 0.0f;
	pitch = 0.0f;

	char c = 0;
	generate_world();
loop:	tick();
	display();
	usleep(20000);
	ssize_t n = read(0, &c, 1);
	if (n == 0) goto loop;
	if (n < 0) { perror("read"); exit(1); }
	if (c == 'q') goto done;

	if (c == 'n' or c == 'e' or c == 'o' or c == 'u') {

		float xrel = 0, yrel = 0;
		if (c == 'n') { xrel = -1; }
		if (c == 'o') { xrel = 1; }
		if (c == 'u') { yrel = -1; }
		if (c == 'e') { yrel = 1; }
		
		const float dx = (float) xrel;
    		const float dy = (float) yrel;
		yaw += camera_sensitivity * dx;
		pitch -= camera_sensitivity * dy;
		if (pitch > pi_over_2) pitch = pi_over_2 - 0.0001f;
		else if (pitch < -pi_over_2) pitch = -pi_over_2 + 0.0001f;
		forward.x = -sinf(yaw) * cosf(pitch);
		forward.y = -sinf(pitch);
		forward.z = -cosf(yaw) * cosf(pitch);
		forward = normalize(forward);
		right.x = -cosf(yaw);
		right.y = 0.0;
		right.z = sinf(yaw);
		right = normalize(right);
		straight = cross(right, up);
		top = cross(forward, right);
	}

	else if (c == ' ') {
		velocity.x -= camera_accel * up.x;
		velocity.y -= camera_accel * up.y;
		velocity.z -= camera_accel * up.z;
	}

	else if (c == 'a') { 
		velocity.x += camera_accel * up.x;
		velocity.y += camera_accel * up.y;
		velocity.z += camera_accel * up.z;
	}

	else if (c == 'r') { 
		velocity.x += camera_accel * straight.x;
		velocity.y += camera_accel * straight.y;
		velocity.z += camera_accel * straight.z;
	}

	else if (c == 'h') { 
		velocity.x -= camera_accel * straight.x;
		velocity.y -= camera_accel * straight.y;
		velocity.z -= camera_accel * straight.z;
	}

	else if (c == 's') {
		velocity.x -= camera_accel * right.x;
		velocity.y -= camera_accel * right.y;
		velocity.z -= camera_accel * right.z;
	}
	
	else if (c == 't') {
		velocity.x += camera_accel * right.x;
		velocity.y += camera_accel * right.y;
		velocity.z += camera_accel * right.z;
	}

	else {}
	goto loop;
done:; 
	puts("done");
}





































































































