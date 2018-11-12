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
