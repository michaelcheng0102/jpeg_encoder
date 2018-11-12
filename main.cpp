#include <cstdio>
#include <cstdlib>
#include "jpeg.h"
#include "bmp.h"

using namespace std;

int main(int argc, char *argv[]) {
	// read bmp
	BMP bmp;
	bmp.read(argv[1]);

	JPEG jpeg;
	jpeg.convert_bmp_to_jpg(bmp);
	// jpeg.write_to_file(argv[2]);
	return 0;
}
