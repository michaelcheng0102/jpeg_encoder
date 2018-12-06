#include <omp.h>
#include <cstdio>
#include <cstdlib>
#include "jpeg.h"

using namespace std;

int main(int argc, char *argv[]) {
	printf("Thread %d\n", omp_get_thread_num ());
	JPEG jpeg;
	jpeg.convert_bmp_to_jpg(argv[1], argv[2]);
	return 0;
}