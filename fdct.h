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
	double*** data;
	int x;
	int y;
	Block();
	~Block();
};


class FDCT {
public:
	FDCT();
	~FDCT();

	YUV RGB2YCbCr(BMP &bmp);

	// TODO: implement here
	Block** fdct(YUV &yuv);

};

#endif
