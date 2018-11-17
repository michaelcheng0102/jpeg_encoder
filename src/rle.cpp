#include <cstdio>

#include "rle.h"

using namespace std;

RLE::RLE() {
}

RLE::RLE(int r, int c1, int c2): run_len(r), code_size(c1), code_data(c2) {
}

RLE::~RLE() {
}
