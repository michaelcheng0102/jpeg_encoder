#ifndef __FDCT_H__
#define __FDCT_H__
#include "bmp.h"



class YUV {
public:
	YUV();
	YUV(int width, int height);
	~YUV();

	int*** data;
	int width;
	int height;
};


class Block {
public:
	static const int BLOCK_SIZE = 8;
	int*** data;
	int x;
	int y;
	Block();
	~Block();
};


class FDCT {
public:
	FDCT();
	~FDCT();
	const int Qmatrix[8][8] = {
		{16, 11, 10, 16, 24, 40, 51, 61},
	   	{12, 12, 14, 19, 26, 58, 60, 55},
		{14, 13, 16, 24, 40, 57, 69, 56},
		{14, 17, 22, 29, 51, 87, 80, 62},
		{18, 22, 37, 56, 68, 109, 103, 77},
		{24, 35, 55, 64, 81, 104, 113, 92},
		{49, 64, 78, 87, 103, 121, 120, 101},
		{72, 92, 95, 98, 112, 100, 103, 99}
	};

	YUV RGB2YCbCr(BMP &bmp);

	// TODO: implement here
	Block** fdct(YUV &yuv);

};


#endif
