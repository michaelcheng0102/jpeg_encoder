#include <cstdio>
#include <cstdlib>
#include "jpeg.h"

using namespace std;

int main(int argc, char *argv[]) {
	jpeg_init(atoi(argv[3]));
	convert_bmp_to_jpg(argv[1], argv[2]);
	return 0;
}
