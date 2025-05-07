// a program to do raycasting in our voxel-based sandbox game. 
// rewrite to make it use sparse voxel arrays, on 1202409091.182516

__kernel void compute_pixel(
	__constant float* input, 
	__global unsigned int* output, 
	__global unsigned long* space
) {
	const unsigned int id = get_global_id(0);
	const float w = input[0];
	const float h = input[1];
	const unsigned int wi = (unsigned int) w;
	const unsigned int hi = (unsigned int) h;
	const float x = (float)(id % wi);
	const float y = (float)(id / wi);
	const float xr = x / w;
	const float yr = y / h;
	float3 position = (float3) (input[3], input[4], input[5]);
	float3 right = (float3) (input[6], input[7], input[8]);
	float3 top = (float3) (input[9], input[10], input[11]);
	float3 dir = (float3) (input[12], input[13], input[14]);
	const float fov = 0.8;
	const float st_x = -fov + 2 * fov * xr;
	const float st_y = -fov + 2 * fov * yr;   // todo: make this use sperical coordinate ray rotation for finding ray direction. don't inch along by one unit on each axis, because the amount you need to change on each axis is nonlinear, if you want to make the tips of all rays equally spaced to form a sphere, wihch is what we want to get the right visual effects on the peripherals. fix this to use true sphere coordinates.

	dir += right * st_x;
	dir += top * st_y;
	dir = fast_normalize(dir);

	unsigned short a = 0xFF;
	unsigned short r = 0x00;
	unsigned short b = 0x00;
	unsigned short g = 0x00;
	const unsigned int unit_count = (int) (space[0] * 4);

	for (unsigned int n = 0; n < unit_count; n += 4) {
		const unsigned long block_c = space[n + 1];
		const float block_x = space[n + 2];
		const float block_y = space[n + 3];
		const float block_z = space[n + 4];
		float3 center = (float3) (block_x, block_y, block_z);
		float dist = fast_distance(position, center);
		float mydot = dot(dir, position - center);
		if (mydot * mydot >= (dist * dist - 1) && mydot < 0) {
			if (block_c == 0) { r = 0xFF; g = 0x00; b = 0xFF; }
			if (block_c == 1) { r = 0x00; g = 0xFF; b = 0xFF; }
			else { r = 0xFF; g = 0xFF; b = 0xFF; }

			break;
		}
	}
	//if (r >= 0xFF) r = 0xFF;
	//if (g >= 0xFF) g = 0xFF;
	//if (b >= 0xFF) b = 0xFF;
	//const unsigned int color = 0xFF000000 | (r << 16) | (g << 8) | b;
	output[id] = 0xFF000000 | (r << 16) | (g << 8) | b;
}





/*

	we should have each thread mark a particular block as being   marked for moving up.. and then the cpu moves them up bettween frames?


			orrrrr rather, maybe the cpu should just do this calculation once per 100 frames ish...   ie, the cpu reorders the unit buffer    
				whenever the  blocks change!




					yeah i think the block reordering in the space  needs to be  totallyyyyy done       cpu side only. that makes sense.  

						it reallyyyyyyy only needs to happen when the blocks change, right?  so yeah. idk.    whichhh technicallyy will happen a lot... buttt idk... yeah its possible lol hm. interesting 





*/









/*





		     if (block == 1) { r += 0x1F; g += 0x1F; b += 0x5F; }
		else if (block == 2) { r += 0xcc; g += 0x00; b += 0x00; break; }
		else if (block == 3) { r += 0x00; g += 0xcc; b += 0x00; break; }
		else if (block == 4) { r += 0x00; g += 0x00; b += 0xcc; break; }
		else if (block == 5) { r += 0xcc; g += 0xcc; b += 0x00; break; }
		else if (block == 6) { r += 0x00; g += 0xcc; b += 0xcc; break; }
		else if (block == 7) { r += 0xcc; g += 0x00; b += 0xcc; break; }
		else if (block == 8) { r += 0x0c; g += 0x0c; b += 0xcc; break; }
		else if (block == 9) { r += 0x0c; g += 0xcc; b += 0x0c; break; }
		else                 { r += 0xFF; g += 0xFF; b += 0xFF; break; }
		if (r >= 0xFF) r = 0xFF;
		if (g >= 0xFF) g = 0xFF;
		if (b >= 0xFF) b = 0xFF;



*/