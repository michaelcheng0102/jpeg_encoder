#include <stdio.h>
#include <cmath>
#include <cassert>
#include "jpeg.h"
#include "bmp.h"
#include "constants.h"

using namespace std;

inline double alpha(int x) {
	if (x == 0) {
		return 1.0 / sqrt(2);
	}
	return 1.0;
}

JPEG::JPEG() {
}

JPEG::~JPEG() {
}

YUV::YUV() {
	width = 0;
	height = 0;
	y = NULL;
	cb = NULL;
	cr = NULL;
}

YUV::YUV(int w, int h) {
	width = w;
	height = h;

	y = new int*[width];
	for (int i = 0; i < width; i++) {
		y[i] = new int[height];
	}

	cb = new int*[width];
	for (int i = 0; i < width; i++) {
		cb[i] = new int[height];
	}

	cr = new int*[width];
	for (int i = 0; i < width; i++) {
		cr[i] = new int[height];
	}
}

YUV::~YUV() {
	for (int i = 0; i < width; i++) {
		delete []y[i];
	}
	delete []y;

	for (int i = 0; i < width; i++) {
		delete []cb[i];
	}
	delete []cb;

	for (int i = 0; i < width; i++) {
		delete []cr[i];
	}
	delete []cr;
}

void JPEG::RGB2YCbCr(YUV& yuv, const BMP &bmp) {
	// check init
	assert(yuv.width == bmp.width && yuv.height == bmp.height);

	for (int w = 0; w < bmp.width; w++) {
		for (int h = 0; h < bmp.height; h++) {
			yuv.y[w][h] = 2990 * (int) bmp.data[w][h][0]
				+ 5870 * (int) bmp.data[w][h][1]
				+ 1140 * (int) bmp.data[w][h][2]
				- 1280000;

			yuv.cb[w][h] = -1687 * (int) bmp.data[w][h][0]
				- 3313 * (int) bmp.data[w][h][1]
				+ 5000 * (int) bmp.data[w][h][2];

			yuv.cr[w][h] = 5000 * (int) bmp.data[w][h][0]
				- 4187 * (int) bmp.data[w][h][1]
				- 813 * (int) bmp.data[w][h][2];
		}
	}
}

void JPEG::fdct(double f[BLOCK_SIZE][BLOCK_SIZE], int** yuv_data, int st_x, int st_y) {
	for (int u = 0; u < BLOCK_SIZE; u++) {
		for (int v = 0; v < BLOCK_SIZE; v++) {
			f[u][v] = 0.0;
			for (int x = 0; x < BLOCK_SIZE; x++) {
				for (int y = 0; y < BLOCK_SIZE; y++) {
					double cc = cos((2.0 * x + 1.0) * u * PI / 16.0) * cos((2.0 * y + 1.0) * v * PI / 16.0);
					f[u][v] += yuv_data[st_x + x][st_y + y] * cc;
				}
			}
			double aa = alpha(u) * alpha(v);
			f[u][v] = f[u][v] * aa / 40000.0;
		}
	}
}

void JPEG::quantize(int f1[BLOCK_SIZE][BLOCK_SIZE], const double f2[BLOCK_SIZE][BLOCK_SIZE]) {
	static const int Qmatrix[BLOCK_SIZE][BLOCK_SIZE] = {
		{16, 11, 10, 16, 24, 40, 51, 61},
		{12, 12, 14, 19, 26, 58, 60, 55},
		{14, 13, 16, 24, 40, 57, 69, 56},
		{14, 17, 22, 29, 51, 87, 80, 62},
		{18, 22, 37, 56, 68, 109, 103, 77},
		{24, 35, 55, 64, 81, 104, 113, 92},
		{49, 64, 78, 87, 103, 121, 120, 101},
		{72, 92, 95, 98, 112, 100, 103, 99}
	};

	for (int u = 0; u < BLOCK_SIZE; u++) {
		for (int v = 0; v < BLOCK_SIZE; v++) {
			f1[u][v] = f2[u][v] / Qmatrix[u][v];
		}
	}
}

void JPEG::zigzag(int zz[BLOCK_SIZE * BLOCK_SIZE], const int f[BLOCK_SIZE][BLOCK_SIZE]) {
}

int JPEG::go_encode_block(Block& blk, int** yuv_data, int st_x, int st_y) {
	double f[BLOCK_SIZE][BLOCK_SIZE];

	fdct(f, yuv_data, st_x, st_y);
	quantize(blk.data, f);

	//int new_dc = blk.data[0][0];
	//blk.data[0][0] -= old_dc;

	int zz[BLOCK_SIZE * BLOCK_SIZE];
	zigzag(zz, blk.data);

	// FIXME to new_dc
	return 0;
}

void JPEG::encode(YUV &yuv) {
	int b_width = yuv.width / BLOCK_SIZE;
	int b_height = yuv.height / BLOCK_SIZE;

	Block **blks_y = new Block*[b_width];
	for (int i = 0; i < b_width; i++) {
		blks_y[i] = new Block[b_height];
	}

	Block **blks_cb = new Block*[b_width];
	for (int i = 0; i < b_width; i++) {
		blks_cb[i] = new Block[b_height];
	}

	Block **blks_cr = new Block*[b_width];
	for (int i = 0; i < b_width; i++) {
		blks_cr[i] = new Block[b_height];
	}

	for (int i = 0; i < b_width; i++) {
		for (int j = 0; j < b_height; j++) {
			blks_y[i][j].x = i;
			blks_y[i][j].y = j;
			blks_cb[i][j].x = i;
			blks_cb[i][j].y = j;
			blks_cr[i][j].x = i;
			blks_cr[i][j].y = j;

			// Y
			go_encode_block(blks_y[i][j], yuv.y, i * BLOCK_SIZE, j * BLOCK_SIZE);

			// Cb
			go_encode_block(blks_cb[i][j], yuv.cb, i * BLOCK_SIZE, j * BLOCK_SIZE);

			// Cr
			go_encode_block(blks_cb[i][j], yuv.cr, i * BLOCK_SIZE, j * BLOCK_SIZE);
		}
	}
}

void JPEG::convert_bmp_to_jpg(const BMP& bmp) {
	YUV yuv(bmp.width, bmp.height);

	RGB2YCbCr(yuv, bmp);
	encode(yuv);
}

Block::Block() {
}

Block::~Block() {
}
