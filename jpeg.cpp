#include <stdio.h>
#include <cmath>
#include <cassert>
#include <vector>
#include <algorithm>
#include "jpeg.h"
#include "bmp.h"
#include "quantable.h"
#include "huffman.h"
#include "constants.h"

using namespace std;

inline double alpha(int x) {
	if (x == 0) {
		return 1.0 / sqrt(2);
	}
	return 1.0;
}

JPEG::JPEG() {
	for (int i = 0; i < 16; i++) {
		qtab[i] = NULL;
		hdc[i] = NULL;
		hac[i] = NULL;
	}

	qtab[0] = new QuanTable(STD_QUAN_TABLE);

	hdc[YUV_ENUM::YUV_Y] = new Huffman(STD_HUFTAB_LUMIN_DC);
	hdc[YUV_ENUM::YUV_C] = new Huffman(STD_HUFTAB_CHROM_DC);

	hac[YUV_ENUM::YUV_Y] = new Huffman(STD_HUFTAB_LUMIN_AC);
	hac[YUV_ENUM::YUV_C] = new Huffman(STD_HUFTAB_CHROM_AC);
}

JPEG::~JPEG() {
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

void JPEG::category_encode(int& code, int& size) {
	unsigned absc = abs(code);
	unsigned mask = (1 << 15);
	int i = 15;
	if (absc == 0) {
		size = 0;
		return;
	}
	while (i && !(absc & mask)) {
		mask >>= 1;
		i--;
	}
	size = i + 1;
	if (code < 0) {
		code = (1 << size) - absc - 1;
	}
}

void JPEG::fdct(double f[BLOCK_SIZE][BLOCK_SIZE], const int* const* yuv_data, int st_x, int st_y) {
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
	int dx[2] = {-1, 1}, dy[2] = {1, -1};
	int dir = 0;
	int idx = 0;
	bool flag = false;
	int i = 0, j = 0;

	while (1) {
		zz[idx++] = f[i][j];
		if (i == BLOCK_SIZE && j == BLOCK_SIZE) {
			break;
		}

		if (idx < 36) {
			if (i == 0 && !flag) {
				j += 1;
				dir = (dir + 1) % 2;
				flag = true;
			} else if (j == 0 && !flag) {
				i += 1;
				dir = (dir + 1) % 2;
				flag = true;
			} else {
				i += dx[dir];
				j += dy[dir];
				flag = false;
			}
		} else {
			if (i == BLOCK_SIZE && !flag) {
				j += 1;
				dir = (dir + 1) % 2;
				flag = true;
			} else if (j == BLOCK_SIZE && !flag) {
				i += 1;
				dir = (dir + 1) % 2;
				flag = true;
			} else {
				i += dx[dir];
				j += dy[dir];
				flag = false;
			}
		}
	}
}

int JPEG::rle(RLE rle_list[BLOCK_SIZE * BLOCK_SIZE], int& eob, const int zz[BLOCK_SIZE * BLOCK_SIZE]) {
	int idx = 0;
	int cnt_zero = 0;
	eob = 0;
	for (int i = 0; i < BLOCK_SIZE * BLOCK_SIZE; i++) {
		if (zz[i] == 0 && cnt_zero < 15) {
			cnt_zero++;
		} else {
			int code = zz[i], size = 0;
			category_encode(code, size);
			rle_list[idx].run_len = cnt_zero;
			rle_list[idx].code_size = size;
			rle_list[idx].code_data = code;
			cnt_zero = 0;
			idx++;
			if (size != 0) eob = idx;
		}
	}
	return idx;
}

void JPEG::go_encode_block(Block& blk, int& dc, const int* const* yuv_data, int st_x, int st_y, YUV_ENUM type) {
	double f[BLOCK_SIZE][BLOCK_SIZE];

	fdct(f, yuv_data, st_x, st_y);
	quantize(blk.data, f);

	int zz[BLOCK_SIZE * BLOCK_SIZE];
	zigzag(zz, blk.data);

	// DC
	int diff = zz[0] - dc, size = 0;
	pair<unsigned int, int> e;

	dc = zz[0];
	category_encode(diff, size);
	e = hdc[type]->encode(size);
	blk.write_bit(e.first, e.second);
	blk.write_bit(diff, size);

	// AC
	RLE rle_list[BLOCK_SIZE * BLOCK_SIZE];
	int eob = 0;
	int rle_idx = rle(rle_list, eob, zz);
	if (zz[BLOCK_SIZE - 1] == 0) { // eob
		rle_list[eob].run_len = 0;
		rle_list[eob].code_size = 0;
		rle_list[eob].code_data = 0;
		rle_idx = eob + 1;
	}
	// TODO: huffman AC
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

	int dc[3] = {0};
	for (int i = 0; i < b_width; i++) {
		for (int j = 0; j < b_height; j++) {
			blks_y[i][j].x = i;
			blks_y[i][j].y = j;
			blks_cb[i][j].x = i;
			blks_cb[i][j].y = j;
			blks_cr[i][j].x = i;
			blks_cr[i][j].y = j;

			// Y
			go_encode_block(blks_y[i][j], dc[0], yuv.y, i * BLOCK_SIZE, j * BLOCK_SIZE, YUV_ENUM::YUV_Y);

			// Cb
			go_encode_block(blks_cb[i][j], dc[1], yuv.cb, i * BLOCK_SIZE, j * BLOCK_SIZE, YUV_ENUM::YUV_C);

			// Cr
			go_encode_block(blks_cb[i][j], dc[2], yuv.cr, i * BLOCK_SIZE, j * BLOCK_SIZE, YUV_ENUM::YUV_C);
		}
	}
}

void JPEG::convert_bmp_to_jpg(const char* input_path, const char* output_path) {
	BMP bmp;
	if (!bmp.read(input_path)) {
		return;
	}

	YUV yuv(bmp.width, bmp.height);

	RGB2YCbCr(yuv, bmp);
	encode(yuv);

	// TODO: write to output_path
}

Block::Block() {
	buf_bit_idx = 8;
}

Block::~Block() {
}

void Block::write_bit(int n, int bitsize) {
	int write_bit = 0;
	while (write_bit < bitsize) {
		if (buf_bit_idx == 8) {
			buffer.push_back((unsigned char)0);
			buf_bit_idx = 0;
		}

		int now_write = min(bitsize - write_bit, 8 - buf_bit_idx);
		int shift = bitsize - write_bit - now_write;
		int shift2 = 8 - now_write - buf_bit_idx;
		int bitmask = (1 << now_write) - 1;

		unsigned char x = (unsigned char)((bitmask & (n >> shift)) << shift2);
		buffer[buffer.size() - 1] = buffer[buffer.size() - 1] | x;

		buf_bit_idx += now_write;
		write_bit += now_write;
	}
}
