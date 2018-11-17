#ifndef __HUFFMAN_H__
#define __HUFFMAN_H__

#include "constants.h"

class HuffmanCodeItem {
public:
	int depth;
	int code;
};

class Huffman {
public:
	unsigned char table[MAX_HUFFMAN_CODE_LEN + 256];
	int first[MAX_HUFFMAN_CODE_LEN];
	int index[MAX_HUFFMAN_CODE_LEN];

	HuffmanCodeItem codelist[256];

	Huffman();
	Huffman(const unsigned char t[MAX_HUFFMAN_CODE_LEN + 256]);
	~Huffman();

	std::pair<int, int> encode(int data);
};

#endif
