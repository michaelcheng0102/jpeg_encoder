#include <cstdio>
#include "yuv.h"

using namespace std;

YUV::YUV() {
	width = 0;
	height = 0;
	y = NULL;
	cb = NULL;
	cr = NULL;
}

YUV::YUV(int h, int w) {
	height = h;
	width = w;

	y = new int*[height];
	for (int i = 0; i < height; i++) {
		y[i] = new int[width];
	}

	cb = new int*[height];
	for (int i = 0; i < height; i++) {
		cb[i] = new int[width];
	}

	cr = new int*[height];
	for (int i = 0; i < height; i++) {
		cr[i] = new int[width];
	}
}

YUV::~YUV() {
	for (int i = 0; i < height; i++) {
		delete []y[i];
	}
	delete []y;

	for (int i = 0; i < height; i++) {
		delete []cb[i];
	}
	delete []cb;

	for (int i = 0; i < height; i++) {
		delete []cr[i];
	}
	delete []cr;
}
