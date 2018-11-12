#ifndef __JPEG_H__
#define __JPEG_H__

#include "bmp.h"


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
	static const int BLOCK_SIZE = 8;
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

	void fdct(double f[8][8], int** yuv_data, int st_x, int st_y);
	void quantize(int f1[8][8], const double f2[8][8]);
	void zigzag(int zz[64], const int f[8][8]);

	int go_encode_block(Block& blk, int** yuv_data, int st_x, int st_y);

	void encode(YUV &yuv);

public:
	JPEG();
	~JPEG();

	void convert_bmp_to_jpg(const BMP& bmp);
};

#endif
