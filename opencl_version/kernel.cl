// a program to do raycasting in our voxel-based sandbox game. 

__kernel void compute_pixel(
	__constant float* input, 
	__global unsigned int* output, 
	__global unsigned char* space
) {
	const unsigned int id = get_global_id(0);
	const float w = input[0];
	const float h = input[1];
	const float sf = input[2];
	const unsigned int wi = (unsigned int) w;
	const unsigned int hi = (unsigned int) h;
	const unsigned int s = (unsigned int) sf;
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
	const float st_y = -fov + 2 * fov * yr;
	dir += right * st_x;
	dir += top * st_y;
	dir = normalize(dir);

	float3 S = (float3) (
		sqrt(1 + (dir.y/dir.x)*(dir.y/dir.x) + (dir.z/dir.x)*(dir.z/dir.x)),
		sqrt((dir.x/dir.y)*(dir.x/dir.y) + 1 + (dir.z/dir.y)*(dir.z/dir.y)),
		sqrt((dir.x/dir.z)*(dir.x/dir.z) + (dir.y/dir.z)*(dir.y/dir.z) + 1)
	);

	float3 start = position;
	int3 m = (int3) (start.x, start.y, start.z);

	float3 length;
	int3 unit;

	if (dir.x < 0) {
		unit.x = -1;
		length.x = (start.x - (float)(m.x)) * S.x;
	} else {
		unit.x = 1;
		length.x = ((float)(m.x + 1) - start.x) * S.x;
	}

	if (dir.y < 0) {
		unit.y = -1;
		length.y = (start.y - (float)(m.y)) * S.y;
	} else {
		unit.y = 1;
		length.y = ((float)(m.y + 1) - start.y) * S.y;
	}

	if (dir.z < 0) {
		unit.z = -1;
		length.z = (start.z - (float)(m.z)) * S.z;
	} else {
		unit.z = 1;
		length.z = ((float)(m.z + 1) - start.z) * S.z;
	}

	unsigned short a = 0xFF;
	unsigned short r = 0x00;
	unsigned short b = 0x00;
	unsigned short g = 0x00;

	for (unsigned int n = 0; n < 800; n++) {
		
		const unsigned char block = space[s * s * m.z + s * m.y + m.x];

		if (block) {
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
		}
		
		if (length.x <= length.y && length.x <= length.z) {
			m.x = (m.x + unit.x + s) % s;
			length.x += S.x;

		} else if (length.y <= length.x && length.y <= length.z) {
			m.y = (m.y + unit.y + s) % s;
			length.y += S.y;

		} else {
			m.z = (m.z + unit.z + s) % s;
			length.z += S.z;
		}
	}

	if (r >= 0xFF) r = 0xFF;
	if (g >= 0xFF) g = 0xFF;
	if (b >= 0xFF) b = 0xFF;

	const unsigned int color = 0xFF000000 | (r << 16) | (g << 8) | b;

	output[id] = color;

}


