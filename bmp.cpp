#include <cstdio>
#include "bmp.h"
#include "constants.h"

using namespace std;

BMP::BMP() {
}

BMP::~BMP() {
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
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

	height = height_ori = (int) header[22] + (int)header[23] * 256 + (int)header[24] * 256 * 256;
	width = width_ori = (int) header[18] + (int)header[19] * 256 + (int)header[20] * 256 * 256;
	fprintf(stderr, "%d %d\n", width, height);

	int mod = BLOCK_SIZE * 2;
	if (height % mod != 0) {
		height += (mod - height % mod);
	}

	if (width % mod != 0) {
		width += (mod - width % mod);
	}

	data = new unsigned char**[height];
	for (int i = 0; i < height; i++) {
		data[i] = new unsigned char*[width];
		for (int j = 0; j < width; j++) {
			data[i][j] = new unsigned char[3];
		}
	}

	for (int i = 0; i < height_ori; i++) {
		for (int j = 0; j < width_ori; j++) {
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

	// fill extra row
	for (int i = height_ori; i < height; i++) {
		for (int j = 0; j < width; j++) {
			data[i][j][0] = data[i - 1][j][0];
			data[i][j][1] = data[i - 1][j][1];
			data[i][j][2] = data[i - 1][j][2];
		}
	}

	// fill extra column
	for (int j = width_ori; j < width; j++) {
		for (int i = 0; i < height; i++) {
			data[i][j][0] = data[i][j - 1][0];
			data[i][j][1] = data[i][j - 1][1];
			data[i][j][2] = data[i][j - 1][2];
		}
	}

	fprintf(stderr, "%d %d\n", width, height);

	fclose(fp);
	return true;
}
