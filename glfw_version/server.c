// server for block based sandbox game.
// written by dwrr, started on 202406156.001556:

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct nat3 { nat x, y, z; };
typedef uint64_t nat;

static const char* player_name[128] = {0};
struct nat3 player_name[128] = {0};
	

int main(void) {
	
	const int s = 200;
	const int space_count = s * s * s;
	int8_t* space = calloc(space_count, 1);

	for (int x = 0; x < s; x++) {
		for (int z = 0; z < s; z++) {

			const float f = perlin2d(x, z, 0.01f, 20);
			const int H = (int) (f * 50);
			//printf("H = %u, n = %f\n", H, f);

			const int divide = H / 2;
			for (int y = 0; y < H; y++) {
				if (y >= divide) space[s * s * x + s * y + z] = dirt_block;
				if (y < divide) space[s * s * x + s * y + z] = stone_block + (rand() % 2) * (rand() % 2);

			}
			space[s * s * x + s * H + z] = grass_block;
		}
	}

	space[s * s * 50 + s * 50 + 4] = air_block;
	space[s * s * 50 + s * 50 + 6] = grass_block;
	space[s * s * 50 + s * 50 + 8] = dirt_block;
	space[s * s * 50 + s * 50 + 10] = stone_block;
	space[s * s * 50 + s * 50 + 12] = granite_block;
	space[s * s * 50 + s * 50 + 14] = wood_block;
	space[s * s * 50 + s * 50 + 16] = leaves_block;
	space[s * s * 50 + s * 50 + 18] = water_block;
	space[s * s * 50 + s * 50 + 20] = moss_block;
	space[s * s * 50 + s * 50 + 22] = iron_ore_block;
	space[s * s * 50 + s * 50 + 24] = off_cell_block;
	space[s * s * 50 + s * 50 + 26] = on_cell_block;
	space[s * s * 50 + s * 50 + 28] = off_path_block;
	space[s * s * 50 + s * 50 + 30] = on_path_block;
	space[s * s * 50 + s * 50 + 32] = glass_block;


	position.x = 20;
	position.y = 25;
	position.z = 20;
	space[s * s * 20 + s * 25 + 20] = on_cell_block;

	



}


