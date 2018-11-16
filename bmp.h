#ifndef __BMP_H__
#define __BMP_H__

class BMP {
public:
	unsigned char*** data;

	int height_ori;
	int width_ori;

	int height;
	int width;

	BMP();
	~BMP();
	bool read(const char* path);
};

#endif
