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

	width = (int) header[18];
	height = (int) header[22];

	data = new unsigned char**[width];
	for (int i = 0; i < width; ++i) {
		data[i] = new unsigned char*[height];
		for (int j = 0; j < height; ++j) {
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

	fclose(fp);
	return true;
}
