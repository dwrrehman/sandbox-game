// a 3d terminal game that uses
// raytracing into a 3d block world, 
// done on the cpu over several frames,
// for its graphics. written on 1202410174.181851 dwrr





/*




		curent state:


			we need to trace paths    and leave behind teh path as well, not just move forwards the head of the ray. 

				this allows us to contniuouslyyyyy (essentially)  send a continuous stream of photons using that full path
								thus keeping the pixel on.  that the ray hit. 



					we do want to use ray marching though, to step forwards as we are currently doing as well, though, becasue that allows us to get object collision good.   we need to be using the dda algorithm for this though lol. to do it right, and efficiently. 


								(doesnt matter if its a bit slow, just make it precise lol.)

											(heck we don't even needddd dda lol)



					we also want to i think, associate a ray with a bounced ray. ie, they associate via  tree like data structure ithink. or like a linked list, kinda. one ray links to another in the ray list. 


							if it came about via a bounce. this allows us to track dependancies, when the path is suddenly blocked, at some earlier point in the ray trajectory. 



					


					also, we need to keeping around all these traced paths, becuase then we will be able to keep the ray count static, after a sufficient amount of computation lol. ie, everything has already been traced basically. 

							this needs to happen. this is what allows us to have the insane performance we want. 


								only this. 




						

			ohhh, but note, we need to have the retina still updated though, when move or move our head,  we need to update the retina ie, looping through the rays and finding which ones hit the retina, now that its moved lol. 


					hmmm this is expensive, as well. crap.. hmmmm









		yeah we have to solve that. crappp hmmmmm







*/





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

static const nat side_length = 100;

enum blocks {

	air,
	light,
	player,

	unused_block1,
	unused_block2,

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
struct color { float r, b, g; };

struct ray {
	struct color color;
	struct vec3 position;
	struct vec3 direction;
	float intensity;
	nat bounces;
};

static const float camera_sensitivity = 0.05f;
static const float pi_over_2 = 1.57079632679f;
static const float camera_accel = 0.01f;

static bool displaying = false;

static nat height = 0;
static nat width = 0;
static nat max_length = 0;

static uint32_t* retina = NULL;
static char* screen = NULL;

static uint32_t* space = NULL;
static struct ray* rays = NULL;
static nat ray_count = 0;

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
	if (displaying) return;
	ioctl(0, TIOCGWINSZ, &window); 
	height = 30;	//window.ws_row - 10; 
	width = 80;	//window.ws_col - 20; 
	max_length = (size_t) (128 * width * height);
	retina = realloc(retina, width * height * sizeof(uint32_t));
	screen = realloc(screen, max_length);
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
	const nat s = side_length;
	space = calloc(total, sizeof(nat));

	space[s * s * 4 + s * 3 + 2] = snow;
	space[s * s * 2 + s * 2 + 2] = snow;
	space[s * s * 2 + s * 2 + 5] = snow;
	space[s * s * 3 + s * 3 + 3] = snow;
	space[s * s * 5 + s * 5 + 5] = light;
	space[s * s * 5 + s * 7 + 8] = light;
}

static void tick(void) {
	
	const nat s = side_length;

	position.x += velocity.x;
	position.y += velocity.y;
	position.z += velocity.z;
	velocity.x *= 0.96f;
	velocity.y *= 0.96f;
	velocity.z *= 0.96f;

	position.x = fmodf(position.x + (float)s, (float)s);
	position.y = fmodf(position.y + (float)s, (float)s);
	position.z = fmodf(position.z + (float)s, (float)s);
}

static void ray_trace(void) {

	const nat s = side_length;

	for (nat x = 0; x < s; x++) {
		for (nat y = 0; y < s; y++) {
			for (nat z = 0; z < s; z++) {
				
				const uint32_t b = space[s * s * x + s * y + z];
	
				if (b != light or ray_count >= 10000) continue;
				for (nat i = 0; i < 200; i++) {
			
				const float rx = (float) (rand() % 100000);
				const float dx = rx / 100000.0f;

				const float ry = (float) (rand() % 100000);
				const float dy = ry / 100000.0f;

				const float rz = (float) (rand() % 100000);
				const float dz = rz / 100000.0f;

				struct ray new_ray = {
					.color = {1.0, 1.0, 1.0},
					.position = {x, y, z},
					.direction = {dx - 0.5f, dy - 0.5f, dz - 0.5f},
					.intensity = 1.0,
					.bounces = 0,
				};

				/*printf("generated ray  ###{pos={%lf,%lf,%lf},dir={%lf,%lf,%lf},color={%lf,%lf,%lf},intensity=%lf,bounces=%llu}\033[K\n", 

					(double) new_ray.position.x, 
					(double) new_ray.position.y, 
					(double) new_ray.position.z, 
					(double) new_ray.direction.x, 
					(double) new_ray.direction.y, 
					(double) new_ray.direction.z, 
					(double) new_ray.color.r, 
					(double) new_ray.color.g, 
					(double) new_ray.color.g, 
					(double) new_ray.intensity,
					new_ray.bounces
				);*/
				//fflush(stdout);
				//sleep(1);

				rays = realloc(rays, sizeof(struct ray) * (ray_count + 1));
				rays[ray_count++] = new_ray;
				//printf("generated rays: %llu\n", ray_count);
				//usleep(10000);


				}
			}
		}
	}



	const float factor = 0.4f;

	for (nat i = 0; i < ray_count; i++) {
		
		rays[i].position.x += rays[i].direction.x * factor;
		rays[i].position.y += rays[i].direction.y * factor;
		rays[i].position.z += rays[i].direction.z * factor;

		if (rays[i].position.x < 0) rays[i].position.x = 0;
		if (rays[i].position.y < 0) rays[i].position.y = 0;
		if (rays[i].position.z < 0) rays[i].position.z = 0;

		if (rays[i].position.x > (float)s) rays[i].position.x = (float)s;
		if (rays[i].position.y > (float)s) rays[i].position.y = (float)s;
		if (rays[i].position.z > (float)s) rays[i].position.z = (float)s;

		const float fx = rays[i].position.x;
		const float fy = rays[i].position.y;
		const float fz = rays[i].position.z;
		nat x = (nat) fx;
		nat y = (nat) fy;
		nat z = (nat) fz;
		x += s; y += s; z += s;
		x %= s; y %= s; z %= s;
		const uint32_t b = space[s * s * x + s * y + z];
		if (b <= light) continue;
		const float dx = fx - (float) x;
		const float dy = fy - (float) y;
		const float dz = fz - (float) z;
		const bool cx = dx < 0.5f;
		const bool cy = dy < 0.5f;
		const bool cz = dz < 0.5f;

		if (not cx and not cy and not cz) { }//puts("0 0 0"); fflush(stdout); }
		if (not cx and not cy and     cz) { }//puts("0 0 1"); fflush(stdout); }
		if (not cx and     cy and not cz) { }//puts("0 1 0"); fflush(stdout); }
		if (not cx and     cy and     cz) { }//puts("0 1 1"); fflush(stdout); }
		if (    cx and not cy and not cz) { }//puts("1 0 0"); fflush(stdout); }
		if (    cx and not cy and     cz) { }//puts("1 0 1"); fflush(stdout); }
		if (    cx and     cy and not cz) { }//puts("1 1 0"); fflush(stdout); }
		if (    cx and     cy and     cz) { }//puts("1 1 1"); fflush(stdout); }
		// rays[i].bounces++; // eventually lol 
	}


	
	for (nat i = 0; i < ray_count; i++) {

		bool outside = false;

		if (rays[i].position.x <= 0.0f) outside = true;
		if (rays[i].position.y <= 0.0f) outside = true;
		if (rays[i].position.z <= 0.0f) outside = true;

		if (rays[i].position.x >= (float) s) outside = true;
		if (rays[i].position.y >= (float) s) outside = true;
		if (rays[i].position.z >= (float) s) outside = true;

		if (rays[i].bounces > 3 or outside) {
			//printf("deleting a ray...");
			//fflush(stdout);
			rays[i] = rays[ray_count - 1];
			ray_count--;
		}
	}

	memset(retina, 0, 4 * height * width);
	for (nat h = 0; h < height; h++) {
		for (nat w = 0; w < width; w++) {
			struct vec3 pixel = position;
			const float hf = (float) h;
			const float heightf = (float) height;
			const float heightf2 = (float) height / 2.0f;
			const float wf = (float) w;
			const float widthf = (float) width;
			const float widthf2 = (float) width / 2.0f;
			pixel.x += top.x * (heightf2 - hf) / heightf;
			pixel.y += top.y * (heightf2 - hf) / heightf;
			pixel.z += top.z * (heightf2 - hf) / heightf;
			pixel.x += right.x * (widthf2 - wf) / widthf;
			pixel.y += right.y * (widthf2 - wf) / widthf;
			pixel.z += right.z * (widthf2 - wf) / widthf;

			for (nat i = 0; i < ray_count; i++) {
				if (  	fabsf(rays[i].position.x - pixel.x) < 0.01f and 
					fabsf(rays[i].position.y - pixel.y) < 0.01f and 
					fabsf(rays[i].position.z - pixel.z) < 0.01f and

					rays[i].direction.x * forward.x + 
					rays[i].direction.y * forward.y + 
					rays[i].direction.z * forward.z 
					< 0
				) {

					retina[width * h + w] = 1;

						// printf("deleting a ray...");
						//fflush(stdout);
						rays[i] = rays[ray_count - 1];
						ray_count--;
					

				}
			}
		}
	}
}


static void display(void) {

	displaying = true;
	nat length = 3;
	memcpy(screen, "\033[H", 3);

	for (nat h = 0; h < height; h++) {
		for (nat w = 0; w < width; w++) {

			uint32_t b = retina[width * h + w];
			if (b) b = snow;
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
		screen + length, 2048, 
		"\033[0mdebug: pos(%lf %lf %lf} vel(%lf %lf %lf} str(%lf %lf %lf} \033[K\n  for(%lf %lf %lf} top(%lf %lf %lf} right(%lf %lf %lf}", 
		(double) position.x,
		(double) position.y,
		(double) position.z,

		(double) velocity.x,
		(double) velocity.y,
		(double) velocity.z,

		(double) straight.x,
		(double) straight.y,
		(double) straight.z,

		(double) forward.x,
		(double) forward.y,
		(double) forward.z,

		(double) top.x,
		(double) top.y,
		(double) top.z,

		(double) right.x,
		(double) right.y,
		(double) right.z
	);

	screen[length++] = 033;
	screen[length++] = '[';
	screen[length++] = 'K';
	screen[length++] = 10;

	printf("\033[H\033[2J");
	fflush(stdout);

	write(1, screen, length);

	return;



	printf("rays: %llu: {\033[K\n", ray_count);
	fflush(stdout);

	for (nat i = 0; i < ray_count; i++) {

		printf("ray{pos={%lf,%lf,%lf},dir={%lf,%lf,%lf},color={%lf,%lf,%lf},intensity=%lf,bounces=%llu}\033[K\n", 

			(double) rays[i].position.x, 
			(double) rays[i].position.y, 
			(double) rays[i].position.z, 

			(double) rays[i].direction.x, 
			(double) rays[i].direction.y, 
			(double) rays[i].direction.z, 

			(double) rays[i].color.r, 
			(double) rays[i].color.g, 
			(double) rays[i].color.g, 

			(double) rays[i].intensity,

			rays[i].bounces
		);
		fflush(stdout);
	}
	puts("}\033[K");
	fflush(stdout);
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
	ray_trace();
	display();
	usleep(10000);
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





































































































