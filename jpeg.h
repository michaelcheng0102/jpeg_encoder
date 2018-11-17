#ifndef __JPEG_H__
#define __JPEG_H__

#include <vector>

#include "bmp.h"
#include "yuv.h"
#include "constants.h"
#include "quantable.h"
#include "huffman.h"

class RLE {
public:
	int run_len;
	int code_size;
	int code_data;
	RLE() { }
	RLE(int r, int c1, int c2): run_len(r), code_size(c1), code_data(c2) { }
};

class Block {
public:
	int data[BLOCK_SIZE][BLOCK_SIZE];
	int type;
	std::vector<RLE> rle_list;

	Block();
	~Block();
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

	int category_encode(int val);

	void fdct(double f[BLOCK_SIZE][BLOCK_SIZE], const double yuv_data[BLOCK_SIZE][BLOCK_SIZE]);
	void quantize(int f1[BLOCK_SIZE][BLOCK_SIZE], const double f2[BLOCK_SIZE][BLOCK_SIZE], YUV_ENUM type);
	void zigzag(int zz[BLOCK_SIZE * BLOCK_SIZE], const int f[BLOCK_SIZE][BLOCK_SIZE]);
	void rle(std::vector<RLE>& rle_list, const int zz[BLOCK_SIZE * BLOCK_SIZE]);

	void go_transform_block(Block& blk, const double yuv_data[BLOCK_SIZE][BLOCK_SIZE], YUV_ENUM type);

	void write_huffman(int key, int val, int len, Huffman* table, YUV_ENUM type, FILE* fp, bool ff = true);
	void go_encode_block_to_file(Block& blk, int& dc, YUV_ENUM type, FILE* fp);

	void encode(YUV &yuv);
	void write_to_file(const char* output_path);

public:
	JPEG();
	~JPEG();

	void convert_bmp_to_jpg(const char* input_path, const char* output_path);
};

#endif
