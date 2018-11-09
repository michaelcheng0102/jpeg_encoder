#ifndef __FDCT_H__
#define __FDCT_H__
#include "bmp.h"



class Block {

public:
	Block();
	Block(int width, int height);
	~Block();

	int*** data;
	int width;
	int height;
};


class FDCT {

public:
	FDCT();
	~FDCT();

	Block RGB2YCbCr(BMP &bmp);

};

#endif
