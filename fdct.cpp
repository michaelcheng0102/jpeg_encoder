#include <stdio.h>
#include "fdct.h"
#include "bmp.h"

using namespace std;

FDCT::FDCT() {}

FDCT::~FDCT() {

}

YUV::YUV() {
	width = 0;
	height = 0;
	data = NULL;
}

YUV::YUV(int w, int h) {
	width = w;
	height = h;
	data = new int**[width];
	for (int i = 0; i < width; i++) {
		data[i] = new int*[height];
		for (int j = 0; j < height; j++) {
			data[i][j] = new int[3];
		}
	}
}

YUV::~YUV() {
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			delete []data[i][j];
		}
		delete []data[i];
	}
	delete []data;
}

YUV FDCT::RGB2YCbCr(BMP &bmp) {


	// initialization
	YUV b(bmp.width, bmp.height);
	for (int w = 0; w < bmp.width; w++) {
		for (int h = 0; h < bmp.height; h++) {
			b.data[w][h][0] = 2990 * (int) bmp.data[w][h][0]
				+ 5870 * (int) bmp.data[w][h][1]
				+ 1140 * (int) bmp.data[w][h][2]
				- 1280000;

			b.data[w][h][1] = -1687 * (int) bmp.data[w][h][0]
				- 3313 * (int) bmp.data[w][h][1]
				+ 5000 * (int) bmp.data[w][h][2];

			b.data[w][h][2] = 5000 * (int) bmp.data[w][h][0]
				- 4187 * (int) bmp.data[w][h][1]
				- 813 * (int) bmp.data[w][h][2];
		}
	}
	return b;
}

Block::Block() {
	data = new int**[BLOCK_SIZE];
	for (int i = 0; i < BLOCK_SIZE; i++) {
		data[i] = new int*[BLOCK_SIZE];
		for (int j = 0; j < BLOCK_SIZE; j++) {
			data[i][j] = new int[3];
		}
	}
}

Block::~Block() {
	for (int i = 0; i < BLOCK_SIZE; i++) {
		for (int j = 0; j < BLOCK_SIZE; j++) {
			delete []data[i][j];
		}
		delete []data[i];
	}
	delete []data;
}

