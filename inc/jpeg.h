#ifndef __JPEG_H__
#define __JPEG_H__

#include <vector>

#include "bmp.h"
#include "yuv.h"
#include "quantable.h"
#include "huffman.h"
#include "block.h"
#include "rle.h"
#include "constants.h"

void RGB2YCbCr(YUV& yuv, const BMP& bmp);

int category_encode(int val);

void zigzag(int zz[BLOCK_SIZE * BLOCK_SIZE], const int f[BLOCK_SIZE][BLOCK_SIZE]);
void rle(std::vector<RLE>& rle_list, const int zz[BLOCK_SIZE * BLOCK_SIZE]);

void go_transform_block(Block& blk, YUV_ENUM type);
void encode(YUV &yuv);

void write_huffman(int key, int val, int len, Huffman* table, YUV_ENUM type, FILE* fp, bool ff = true);
void go_encode_block_to_file(Block& blk, int& dc, YUV_ENUM type, FILE* fp);
void write_to_file(const char* output_path);

void jpeg_init(int thread_num);
void convert_bmp_to_jpg(const char* input_path, const char* output_path);

#endif
