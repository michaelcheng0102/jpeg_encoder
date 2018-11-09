#ifndef __BMP_H__
#define __BMP_H__

class BMP {
public:
	unsigned char*** data;
	int width;
	int height;

	BMP();
	~BMP();
	bool read(const char* path);
};

#endif
