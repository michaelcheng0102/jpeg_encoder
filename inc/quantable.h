#ifndef __QUANTABLE_H__
#define __QUANTABLE_H__

#include "constants.h"

class QuanTable {
public:
	int table[BLOCK_SIZE][BLOCK_SIZE];

	QuanTable();
	QuanTable(const int t[BLOCK_SIZE][BLOCK_SIZE]);
	~QuanTable();
};

#endif
