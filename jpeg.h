#ifndef __JPEG_H__
#define __JPEG_H__

#include <vector>

#include "bmp.h"
#include "yuv.h"
#include "constants.h"
#include "quantable.h"
#include "huffman.h"

class Block {
public:
	int data[BLOCK_SIZE][BLOCK_SIZE];
	int type;

	int buf_bit_idx;
	std::vector<unsigned char> buffer;

	Block();
	~Block();

	void write_bit(int n, int bitsize);
};

class RLE {
public:
	int run_len;
	int code_size;
	int code_data;
};

class JPEG {
private:
	int width;
	int height;

	std::vector<std::vector<Block> > blks_y;
	std::vector<std::vector<Block> > blks_cb;
	std::vector<std::vector<Block> > blks_cr;

	QuanTable* qtab[16];
	Huffman* hdc[16];
	Huffman* hac[16];

	void RGB2YCbCr(YUV& yuv, const BMP& bmp);

	void category_encode(int& code, int& size);

	void fdct(double f[BLOCK_SIZE][BLOCK_SIZE], const int* const* yuv_data, int st_x, int st_y);
	void quantize(int f1[BLOCK_SIZE][BLOCK_SIZE], const double f2[BLOCK_SIZE][BLOCK_SIZE], YUV_ENUM type);
	void zigzag(int zz[BLOCK_SIZE * BLOCK_SIZE], const int f[BLOCK_SIZE][BLOCK_SIZE]);
	int rle(RLE rle_list[BLOCK_SIZE * BLOCK_SIZE], int& eob, const int zz[BLOCK_SIZE * BLOCK_SIZE]);

	void go_encode_block(Block& blk, int& dc, const int* const* yuv_data, int st_x, int st_y, YUV_ENUM type);

	void encode(YUV &yuv);
	void write_to_file(const char* output_path);

public:
	JPEG();
	~JPEG();

	void convert_bmp_to_jpg(const char* input_path, const char* output_path);
};

#endif
