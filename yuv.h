#ifndef __YUV_H__
#define __YUV_H__

enum YUV_ENUM {
	YUV_Y = 0,
	YUV_C
};

class YUV {
public:
	YUV();
	YUV(int h, int w);
	~YUV();

	int** y;
	int** cb;
	int** cr;

	int height;
	int width;
};

#endif
