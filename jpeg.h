#ifndef __JPEG_H__
#define __JPEG_H__

#include "bmp.h"
#include "constants.h"


class YUV {
public:
	YUV();
	YUV(int w, int h);
	~YUV();

	int** y;
	int** cb;
	int** cr;

	int width;
	int height;
};


class Block {
public:
	int data[BLOCK_SIZE][BLOCK_SIZE];
	int type;
	int x;
	int y;
	Block();
	~Block();
};


class JPEG {
private:
	void RGB2YCbCr(YUV& yuv, const BMP& bmp);

	void fdct(double f[BLOCK_SIZE][BLOCK_SIZE], int** yuv_data, int st_x, int st_y);
	void quantize(int f1[BLOCK_SIZE][BLOCK_SIZE], const double f2[BLOCK_SIZE][BLOCK_SIZE]);
	void zigzag(int zz[BLOCK_SIZE * BLOCK_SIZE], const int f[BLOCK_SIZE][BLOCK_SIZE]);

	int go_encode_block(Block& blk, int** yuv_data, int st_x, int st_y);

	void encode(YUV &yuv);

public:
	JPEG();
	~JPEG();

	void convert_bmp_to_jpg(const BMP& bmp);
};

#endif
