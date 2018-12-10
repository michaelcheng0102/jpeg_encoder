#define sqrt2 1.41421
#define PI 3.14159
#define BLOCK_SIZE 8

__kernel void fdct(__global double* f, __global double* yuv_data, __global int* width) {
	int wid = *width;
	int idx = get_global_id(0);

	int st_x = idx / wid;
	int st_y = idx % wid;
	int u = 0;
	int v = 0;
	while (st_x % BLOCK_SIZE != 0) {
		st_x--;
		u++;
	}
	while (st_y % BLOCK_SIZE != 0) {
		st_y--;
		v++;
	}

	int x, y;
	double tmp = 0.0;
	for (x = 0; x < BLOCK_SIZE; x++) {
		for (y = 0; y < BLOCK_SIZE; y++) {
			float t1 = (2.0 * x + 1.0) * u * PI / 16.0;
			float t2 = (2.0 * y + 1.0) * v * PI / 16.0;
			int xx = st_x + x;
			int yy = st_y + y;
			tmp = tmp + yuv_data[xx * wid + yy] * cos(t1) * cos(t2);
		}
	}
	double a1 = (u == 0) ? (1.0 / sqrt2) : 1.0;
	double a2 = (v == 0) ? (1.0 / sqrt2) : 1.0;
	f[idx] = tmp * a1 * a2 / 40000.0;
}
