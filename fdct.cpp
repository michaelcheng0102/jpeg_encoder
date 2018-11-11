#include <stdio.h>
#include <cmath>
#include "fdct.h"
#include "bmp.h"

using namespace std;

const double PI = 2.0 * acos(0);

inline double alpha(int x) {
    if (x == 0) {
        return 1.0 / sqrt(2);
    }
    return 1.0;
}

FDCT::FDCT() {
}

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

Block** FDCT::fdct(YUV &yuv) {
    int b_width = yuv.width / 8;
    int b_height = yuv.height / 8;

    Block **blks = new Block*[b_width];
    for (int i = 0; i < b_width; i++) {
        blks[i] = new Block[b_height];
    }

    for (int i = 0; i < b_width; i++) {
        for (int j = 0; j < b_height; j++) {
            blks[i][j].x = i;
            blks[i][j].y = j;
            for (int u = 0; u < Block::BLOCK_SIZE; u++) {
                for (int v = 0; v < Block::BLOCK_SIZE; v++) {
                    blks[i][j].data[u][v][0] = 0;
                    blks[i][j].data[u][v][1] = 0;
                    blks[i][j].data[u][v][2] = 0;
                    for (int x = 0; x < Block::BLOCK_SIZE; x++) {
                        for (int y = 0; y < Block::BLOCK_SIZE; y++) {
                            double cc = cos((2.0 * x + 1.0) * u * PI / 16.0) * cos((2.0 * y + 1.0) * v * PI / 16.0);
                            blks[i][j].data[u][v][0] += yuv.data[i * Block::BLOCK_SIZE + x][j * Block::BLOCK_SIZE + y][0] * cc;
                            blks[i][j].data[u][v][1] += yuv.data[i * Block::BLOCK_SIZE + x][j * Block::BLOCK_SIZE + y][1] * cc;
                            blks[i][j].data[u][v][2] += yuv.data[i * Block::BLOCK_SIZE + x][j * Block::BLOCK_SIZE + y][2] * cc;
                        }
                    }
                    double aa = alpha(u) * alpha(v) / 4.0;
                    blks[i][j].data[u][v][0] *= aa;
                    blks[i][j].data[u][v][1] *= aa;
                    blks[i][j].data[u][v][2] *= aa;
                }
            }
        }
    }

    return blks;
}

Block::Block() {
	data = new double**[BLOCK_SIZE];
	for (int i = 0; i < BLOCK_SIZE; i++) {
		data[i] = new double*[BLOCK_SIZE];
		for (int j = 0; j < BLOCK_SIZE; j++) {
			data[i][j] = new double[3];
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

