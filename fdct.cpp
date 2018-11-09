#include <stdio.h>
#include "fdct.h"
#include "bmp.h"

using namespace std;

FDCT::FDCT() {}

FDCT::~FDCT() {

}

Block::Block() {
	width = 0;
	height = 0;
	data = NULL;
}

Block::Block(int w, int h) {
	width = w;
	height = h;
	data = new double**[width];
	for (int i = 0; i < width; i++) {
		data[w] = new double*[height];
		for (int h = 0; h < height; h++) {
			data[w][h] = new double[3];
		}
	}
}

Block::~Block() {
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			delete []data[i][j];
		}
		delete []data[i];
	}
	delete []data;
}

Block FDCT::RGB2YCbCr(BMP &bmp) {


	// initialization
	Block b(bmp.width, bmp.height);
	for (int w = 0; w < bmp.width; w++) {
		for (int h = 0; h < bmp.height; h++) {
			b.data[w][h][0] = 0.229 * (double) bmp.data[w][h][0]
				+ 0.587 * (double) bmp.data[w][h][1]
				+ 0.114 * (double) bmp.data[w][h][2];

			b.data[w][h][1] = -0.1687 * (double) bmp.data[w][h][0]
				- 0.3313 * (double) bmp.data[w][h][1]
				+ 0.5 * (double) bmp.data[w][h][2]
				+ 128;

			b.data[w][h][2] = 0.5 * (double) bmp.data[w][h][0]
				- 0.4187 * (double) bmp.data[w][h][1]
				- 0.0813 * (double) bmp.data[w][h][2]
				+ 128;

		}
	}
	return b;
}
