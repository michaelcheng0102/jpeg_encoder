#ifndef __JPEG_H__
#define __JPEG_H__

#include "bmp.h"
#include "yuv.h"
#include "constants.h"


class Block {
public:
	int data[BLOCK_SIZE][BLOCK_SIZE];
	int type;
	int x;
	int y;
	Block();
	~Block();
};

class RLE {
public:
	int run_len;
	int code_size;
	int code_data;
};


class JPEG {
private:
	void RGB2YCbCr(YUV& yuv, const BMP& bmp);

	void category_encode(int& code, int& size);

	void fdct(double f[BLOCK_SIZE][BLOCK_SIZE], const int* const* yuv_data, int st_x, int st_y);
	void quantize(int f1[BLOCK_SIZE][BLOCK_SIZE], const double f2[BLOCK_SIZE][BLOCK_SIZE]);
	void zigzag(int zz[BLOCK_SIZE * BLOCK_SIZE], const int f[BLOCK_SIZE][BLOCK_SIZE]);
	int rle(RLE rle_list[BLOCK_SIZE * BLOCK_SIZE], int& eob, const int zz[BLOCK_SIZE * BLOCK_SIZE]);

	void go_encode_block(Block& blk, int& dc, const int* const* yuv_data, int st_x, int st_y);

	void encode(YUV &yuv);

public:
	JPEG();
	~JPEG();

	void convert_bmp_to_jpg(const char* input_path, const char* output_path);
};

#endif
