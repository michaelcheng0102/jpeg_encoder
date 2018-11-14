#include <cstring>
#include <utility>
#include "huffman.h"
#include "constants.h"

using namespace std;

Huffman::Huffman() {
}

Huffman::Huffman(const unsigned char t[MAX_HUFFMAN_CODE_LEN + 256]) {
	memcpy(table, t, MAX_HUFFMAN_CODE_LEN + 256);

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

	for (int i = 0; i < tabsize; i++) {
		symbol = table[MAX_HUFFMAN_CODE_LEN + i];
		codelist[symbol].depth = hufsize[i];
		codelist[symbol].code = hufcode[i];
	}
}

Huffman::~Huffman() {
}

pair<unsigned int, int> Huffman::encode(int data) {
	unsigned int code = codelist[data].code;
	int len = codelist[data].depth;

	return make_pair(code, len);
}
