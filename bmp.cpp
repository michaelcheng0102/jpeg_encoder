#include <cstdio>
#include "bmp.h"

using namespace std;

BMP::BMP() {
}

BMP::~BMP() {
	for (int i = 0; i < width; ++i) {
		for (int j = 0; j < height; ++j) {
			delete []data[i][j];
		}
		delete []data[i];
	}
	delete []data;
}

bool BMP::read(const char* path) {
	FILE *fp = fopen(path, "rb");
	if (fp == NULL) {
		fprintf(stderr, "Open BMP file error\n");
		return false;
	}

	unsigned char header[64];
	fread(header, sizeof(unsigned char), 54, fp);

	width = width_ori = (int) header[18];
	height = height_ori = (int) header[22];

	int mod = 8;
	if (width % mod != 0) {
		width += (mod - width % mod);
	}

	if (height % mod != 0) {
		height += (mod - height % mod);
	}

	data = new unsigned char**[width];
	for (int i = 0; i < width_ori; ++i) {
		data[i] = new unsigned char*[height];
		for (int j = 0; j < height_ori; ++j) {
			data[i][j] = new unsigned char[3];
			if (fread(data[i][j], sizeof(unsigned char), 3, fp) != 3) {
				fprintf(stderr, "Read BMP bytes error\n");
				fclose(fp);
				return false;
			}

			unsigned char tmp = data[i][j][0];
			data[i][j][0] = data[i][j][1];
			data[i][j][1] = data[i][j][2];
			data[i][j][2] = tmp;
		}
	}

	// fill extra row, column
	for (int i = width_ori; i < width; i++) {
		for (int j = 0; j < height; ++j) {
			data[i][j][0] = data[i - 1][j][0];
			data[i][j][1] = data[i - 1][j][1];
			data[i][j][2] = data[i - 1][j][2];
		}
	}

	for (int j = height_ori; j < height; j++) {
		for (int i = 0; i < width; ++i) {
			data[i][j][0] = data[i][j - 1][0];
			data[i][j][1] = data[i][j - 1][1];
			data[i][j][2] = data[i][j - 1][2];
		}
	}

	fclose(fp);
	return true;
}
