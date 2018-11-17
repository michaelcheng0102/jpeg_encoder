#include <cassert>
#include <utility>

#include "huffman.h"
#include "constants.h"

using namespace std;

Huffman::Huffman() {
}

Huffman::Huffman(const unsigned char t[MAX_HUFFMAN_CODE_LEN + 256]) {
	for (int i = 0; i < MAX_HUFFMAN_CODE_LEN + 256; i++) table[i] = t[i];

	int symbol;
	int code = 0;
	unsigned char hufsize[256];
	int hufcode[256];
	int tabsize;

	int k = 0;
	for (int i = 0; i < MAX_HUFFMAN_CODE_LEN; i++) {
		for (int j = 0; j < table[i]; j++) {
			hufsize[k] = i + 1;
			hufcode[k] = code;
			code++;
			k++;
		}
		code <<= 1;
	}
	tabsize = k;
	assert(tabsize < 256);

	for (int i = 0; i < tabsize; i++) {
		symbol = table[MAX_HUFFMAN_CODE_LEN + i];
		assert(symbol < 256);
		codelist[symbol].depth = hufsize[i];
		codelist[symbol].code = hufcode[i];
	}
}

Huffman::~Huffman() {
}

pair<int, int> Huffman::encode(int data) {
	int code = codelist[data].code;
	int len = codelist[data].depth;
	assert(data < 256);

	return make_pair(code, len);
}
