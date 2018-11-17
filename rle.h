#ifndef __RLE_H__
#define __RLE_H__

class RLE {
public:
	int run_len;
	int code_size;
	int code_data;
	RLE();
	RLE(int r, int c1, int c2);
	~RLE();
};

#endif
