#ifndef __BLOCK_H__
#define __BLOCK_H__

#include <vector>

#include "rle.h"
#include "constants.h"

class Block {
public:
	int data[BLOCK_SIZE][BLOCK_SIZE];
	std::vector<RLE> rle_list;

	Block();
	~Block();
};

#endif
