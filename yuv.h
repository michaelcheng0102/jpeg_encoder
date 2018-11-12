#ifndef __YUV_H__
#define __YUV_H__


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

#endif
