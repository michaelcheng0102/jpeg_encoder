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


class FDCT {

public:
	FDCT();
	~FDCT();

	YUV RGB2YCbCr(BMP &bmp);
	void fdct(YUV &b);

};

#endif
