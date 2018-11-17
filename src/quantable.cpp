#include "constants.h"
#include "quantable.h"

QuanTable::QuanTable() {
}

QuanTable::QuanTable(const int t[BLOCK_SIZE][BLOCK_SIZE]) {
	for (int i = 0; i < BLOCK_SIZE; i++) {
		for (int j = 0; j < BLOCK_SIZE; j++) {
			table[i][j] = t[i][j];
		}
	}
}

QuanTable::~QuanTable() {
}
