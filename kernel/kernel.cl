__kernel void fdct(__global double* f, __global double* yuv_data) {
	double sqrt2 = 1.41421;
	double PI = 3.14159;
	int BLOCK_SIZE = 8;
	double tmp = 0.0;

	int idx = get_global_id(0);
	int u = idx / BLOCK_SIZE;
	int v = idx % BLOCK_SIZE;
	int x, y;

	for (x = 0; x < BLOCK_SIZE; x++) {
		for (y = 0; y < BLOCK_SIZE; y++) {
			float t1 = (2.0 * x + 1.0) * u * PI / 16.0;
			float t2 = (2.0 * y + 1.0) * v * PI / 16.0;
			tmp = tmp + yuv_data[x * BLOCK_SIZE + y] * cos(t1) * cos(t2);
		}
	}
	double a1 = (u == 0) ? (1.0 / sqrt2) : 1.0;
	double a2 = (v == 0) ? (1.0 / sqrt2) : 1.0;
	f[idx] = tmp * a1 * a2 / 40000.0;
}
