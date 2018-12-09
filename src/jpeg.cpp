#include <cstdio>
#include <cmath>
#include <cassert>
#include <vector>
#include <algorithm>
#include <OpenCL/cl.h>

#include "jpeg.h"
#include "bmp.h"
#include "yuv.h"
#include "quantable.h"
#include "huffman.h"
#include "block.h"
#include "rle.h"
#include "constants.h"

using namespace std;

int jpeg_width;
int jpeg_height;

vector<vector<Block> > blks_y;
vector<vector<Block> > blks_cb;
vector<vector<Block> > blks_cr;

QuanTable* qtab[16];
Huffman* hdc[16];
Huffman* hac[16];

struct OpenCL {
	cl_context context;
	cl_kernel kernel_fdct;
	cl_command_queue command_queue;
	size_t global_threads[10];
	size_t local_threads[10];
	cl_uint work_dim;
};

OpenCL opencl;

inline double alpha(int x) {
	if (x == 0) {
		return 1.0 / sqrt(2);
	}
	return 1.0;
}

void write_bits(int code, int bitsize, FILE* fp, bool flush = false) {
	static unsigned char x = 0;
	static int idx = 0;

	if (flush) {
		if (idx > 0) {
			x |= ((1 << (8 - idx)) - 1);
			fputc(x, fp);
			x = 0;
			idx = 0;
		}
		return;
	}

	for (int i = 0; i < bitsize; i++) {
		unsigned char c = (code >> (bitsize - i - 1)) & 0x1;
		x |= (c << (8 - idx - 1));
		idx++;

		if (idx == 8) {
			fputc(x, fp);
			if (x == 0xff) fputc(0, fp);
			x = 0;
			idx = 0;
		}
	}
}

void RGB2YCbCr(YUV& yuv, const BMP &bmp) {
	// check init
	assert(yuv.width == bmp.width && yuv.height == bmp.height);
	jpeg_width = yuv.width;
	jpeg_height = yuv.height;

	for (int h = 0; h < bmp.height; h++) {
		for (int w = 0; w < bmp.width; w++) {
			yuv.y[h][w] = 2990 * (int) bmp.data[h][w][0]
				+ 5870 * (int) bmp.data[h][w][1]
				+ 1140 * (int) bmp.data[h][w][2]
				- 1280000;

			yuv.cb[h][w] = -1687 * (int) bmp.data[h][w][0]
				- 3313 * (int) bmp.data[h][w][1]
				+ 5000 * (int) bmp.data[h][w][2];

			yuv.cr[h][w] = 5000 * (int) bmp.data[h][w][0]
				- 4187 * (int) bmp.data[h][w][1]
				- 813 * (int) bmp.data[h][w][2];
		}
	}
}

int category_encode(int val) {
	int cnt = 0;
	for (val = abs(val); val; val >>= 1) {
		cnt++;
	}
	return cnt;
}

void fdct(double f[BLOCK_SIZE][BLOCK_SIZE], const double yuv_data[BLOCK_SIZE][BLOCK_SIZE]) {
	for (int u = 0; u < BLOCK_SIZE; u++) {
		for (int v = 0; v < BLOCK_SIZE; v++) {
			f[u][v] = 0.0;
			for (int x = 0; x < BLOCK_SIZE; x++) {
				for (int y = 0; y < BLOCK_SIZE; y++) {
					double cc = cos((2.0 * x + 1.0) * u * PI / 16.0) * cos((2.0 * y + 1.0) * v * PI / 16.0);
					f[u][v] += yuv_data[x][y] * cc;
				}
			}
			double aa = alpha(u) * alpha(v);
			f[u][v] = f[u][v] * aa / 40000.0;
		}
	}
}

void quantize(int f1[BLOCK_SIZE][BLOCK_SIZE], const double f2[BLOCK_SIZE][BLOCK_SIZE], YUV_ENUM type) {
	for (int u = 0; u < BLOCK_SIZE; u++) {
		for (int v = 0; v < BLOCK_SIZE; v++) {
			f1[u][v] = f2[u][v] / qtab[type]->table[u][v];
		}
	}
}

void zigzag(int zz[BLOCK_SIZE * BLOCK_SIZE], const int f[BLOCK_SIZE][BLOCK_SIZE]) {
	int dx[2] = {-1, 1}, dy[2] = {1, -1};
	int dir = 0;
	int idx = 0;
	bool flag = false;
	int i = 0, j = 0;

	while (1) {
		zz[idx++] = f[i][j];
		if (i == BLOCK_SIZE - 1 && j == BLOCK_SIZE - 1) {
			break;
		}

		if (idx < 36) {
			if (i == 0 && !flag) {
				j += 1;
				dir = (dir + 1) % 2;
				flag = true;
			} else if (j == 0 && !flag) {
				i += 1;
				dir = (dir + 1) % 2;
				flag = true;
			} else {
				i += dx[dir];
				j += dy[dir];
				flag = false;
			}
		} else {
			if (i == BLOCK_SIZE - 1 && !flag) {
				j += 1;
				dir = (dir + 1) % 2;
				flag = true;
			} else if (j == BLOCK_SIZE - 1 && !flag) {
				i += 1;
				dir = (dir + 1) % 2;
				flag = true;
			} else {
				i += dx[dir];
				j += dy[dir];
				flag = false;
			}
		}
	}
}

void rle(vector<RLE>& rle_list, const int zz[BLOCK_SIZE * BLOCK_SIZE]) {
	int cnt_zero = 0;
	int eob = 0;
	for (int i = 1; i < BLOCK_SIZE * BLOCK_SIZE && (int)rle_list.size() < BLOCK_SIZE * BLOCK_SIZE - 1; i++) {
		if (zz[i] == 0 && cnt_zero < 15) {
			cnt_zero++;
		} else {
			int code = zz[i];
			int size = category_encode(code);
			//printf("%d %d %d\n", cnt_zero, size, code);
			rle_list.push_back(RLE(cnt_zero, size, code));
			cnt_zero = 0;
			if (size != 0) {
				eob = (int)rle_list.size();
			}
		}
	}
	//printf("%d %d\n", eob, (int)rle_list.size());
	if (zz[BLOCK_SIZE * BLOCK_SIZE - 1] == 0) {
		assert(eob <= rle_list.size());
		if (eob == (int)rle_list.size()) {
			rle_list.push_back(RLE(0, 0, 0));
		} else {
			rle_list[eob].run_len = 0;
			rle_list[eob].code_size = 0;
			rle_list[eob].code_data = 0;
			rle_list.resize(eob + 1);
		}
	}
}

void go_transform_block(Block& blk, YUV_ENUM type) {
	double f[BLOCK_SIZE][BLOCK_SIZE];
	cl_float f2[BLOCK_SIZE * BLOCK_SIZE];

	// opencl fdct
	cl_int status;

	cl_mem buffer_f = clCreateBuffer(opencl.context, CL_MEM_WRITE_ONLY | CL_MEM_USE_HOST_PTR, BLOCK_SIZE * BLOCK_SIZE * sizeof(cl_float), f2, &status);
	assert(status == CL_SUCCESS);

	cl_mem buffer_tmp_buf = clCreateBuffer(opencl.context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, BLOCK_SIZE * BLOCK_SIZE * sizeof(cl_float), blk.tmp_buf, &status);
	assert(status == CL_SUCCESS);

	status = clSetKernelArg(opencl.kernel_fdct, 0, sizeof(cl_mem), (void*)&buffer_f);
	assert(status == CL_SUCCESS);

	status = clSetKernelArg(opencl.kernel_fdct, 1, sizeof(cl_mem), (void*)&buffer_tmp_buf);
	assert(status == CL_SUCCESS);

	status = clEnqueueNDRangeKernel(opencl.command_queue, opencl.kernel_fdct, opencl.work_dim, NULL, opencl.global_threads, opencl.local_threads, 0, NULL, NULL);
	assert(status == CL_SUCCESS);

	clEnqueueReadBuffer(opencl.command_queue, buffer_f, CL_TRUE, 0, BLOCK_SIZE * BLOCK_SIZE * sizeof(cl_float), f2, 0, NULL, NULL);

	for (int i = 0; i < BLOCK_SIZE; i++) {
		for (int j = 0; j < BLOCK_SIZE; j++) {
			f[i][j] = f2[i * BLOCK_SIZE + j];
		}
	}

	clReleaseMemObject(buffer_f);
	clReleaseMemObject(buffer_tmp_buf);

	// TODO: opencl quantize
	quantize(blk.data, f, type);

	int zz[BLOCK_SIZE * BLOCK_SIZE];
	zigzag(zz, blk.data);

	/*
	for (int i = 0; i < BLOCK_SIZE; i++) {
		for (int j = 0; j < BLOCK_SIZE; j++) {
			printf("%d ", blk.data[i][j]);
		}
		printf("\n");
	}

	printf("DC: %d\n", zz[0]);
	assert(zz[0] == blk.data[0][0]);
	for (int i = 1; i < BLOCK_SIZE * BLOCK_SIZE; i++) printf("%d ", zz[i]);
	printf("\n");
	*/

	// AC RLE
	rle(blk.rle_list, zz);

	/*
	for (int i = 0; i < (int)blk.rle_list.size(); i++) printf("(%d,%d,%d)", blk.rle_list[i].run_len, blk.rle_list[i].code_size, blk.rle_list[i].code_data);
	printf("\n");
	*/
}

void encode(YUV &yuv) {
	int b_width = jpeg_width / BLOCK_SIZE;
	int b_height = jpeg_height / BLOCK_SIZE;

	blks_y.resize(b_height);
	for (int i = 0; i < b_height; i++) {
		blks_y[i].resize(b_width);
	}

	blks_cb.resize(b_height);
	for (int i = 0; i < b_height; i++) {
		blks_cb[i].resize(b_width);
	}

	blks_cr.resize(b_height);
	for (int i = 0; i < b_height; i++) {
		blks_cr[i].resize(b_width);
	}

	// can parallel
	for (int i = 0; i < b_height; i++) {
		for (int j = 0; j < b_width; j++) {
			int st_x = i * BLOCK_SIZE;
			int st_y = j * BLOCK_SIZE;

			// Y
			for (int x = 0; x < BLOCK_SIZE; x++) {
				for (int y = 0; y < BLOCK_SIZE; y++) {
					blks_y[i][j].tmp_buf[x * BLOCK_SIZE + y] = yuv.y[st_x + x][st_y + y];
				}
			}
			//printf("type=%d st_x=%d st_y=%d\n", YUV_ENUM::YUV_Y, st_x, st_y);
			go_transform_block(blks_y[i][j], YUV_ENUM::YUV_Y);

			if (i % 2 != 0 || j % 2 != 0) continue;

			// Cb
			for (int x = 0; x < BLOCK_SIZE; x++) {
				for (int y = 0; y < BLOCK_SIZE; y++) {
					int xx = st_x + x * 2;
					int yy = st_y + y * 2;
					blks_cb[i][j].tmp_buf[x * BLOCK_SIZE + y] = (yuv.cb[xx][yy] + yuv.cb[xx][yy + 1] + yuv.cb[xx + 1][yy] + yuv.cb[xx + 1][yy + 1]) / 4.0;
				}
			}
			//printf("\ntype=%d st_x=%d st_y=%d\n", YUV_ENUM::YUV_C, st_x, st_y);
			go_transform_block(blks_cb[i][j], YUV_ENUM::YUV_C);

			// Cr
			for (int x = 0; x < BLOCK_SIZE; x++) {
				for (int y = 0; y < BLOCK_SIZE; y++) {
					int xx = st_x + x * 2;
					int yy = st_y + y * 2;
					blks_cr[i][j].tmp_buf[x * BLOCK_SIZE + y] = (yuv.cr[xx][yy] + yuv.cr[xx][yy + 1] + yuv.cr[xx + 1][yy] + yuv.cr[xx + 1][yy + 1]) / 4.0;
				}
			}
			//printf("type=%d st_x=%d st_y=%d\n", YUV_ENUM::YUV_C, st_x, st_y);
			go_transform_block(blks_cr[i][j], YUV_ENUM::YUV_C);
		}
		printf("%d/%d\n", i, b_height);
	}
}

/*
string to_bin(int val, int len) {
	string ret = "";
	for (int i = 0; i < len; i++) {
		string tmp = (val & 1) ? "1" : "0";
		ret = tmp + ret;
		val >>= 1;
	}

	return ret;
}
*/

void write_huffman(int key, int val, int len, Huffman* table, YUV_ENUM type, FILE* fp, bool ff) {
	pair<int, int> e = table->encode(key);
	write_bits(e.first, e.second, fp);

	if (--len < 0) {
		return;
	}

	write_bits(val > 0 ? 1 : 0, 1, fp);

	val = val > 0 ? val - (1 << len) : val + (3 << len) - 1;
	write_bits(val, len, fp);
}

void go_encode_block_to_file(Block& blk, int& dc, YUV_ENUM type, FILE* fp) {
	// DC
	int diff = blk.data[0][0] - dc, size;

	dc = blk.data[0][0];
	size = category_encode(diff);
	//printf("DC(%d. size=%d)\n", diff, size);
	write_huffman(size, diff, size, hdc[type], type, fp);

	// AC
	for (int i = 0; i < (int)blk.rle_list.size(); i++) {
		//printf("AC %d (%d,%d,%d):\n", i, blk.rle_list[i].code_data, blk.rle_list[i].code_size, blk.rle_list[i].run_len);
		write_huffman((blk.rle_list[i].run_len << 4) | (blk.rle_list[i].code_size << 0), blk.rle_list[i].code_data, blk.rle_list[i].code_size, hac[type], type, fp, false);
	}
}

void write_to_file(const char* output) {
	FILE* fp = fopen(output, "wb");
	int len;


	// SOI
	fputc(0xff, fp);
	fputc(0xd8, fp);
#if DEBUG
	printf("%02x %02x\n", 0xff, 0xd8);
#endif


	// Quant
	fputc(0xff, fp);
	fputc(0xdb, fp);
	len = 2;
	for (int i = 0; i < 16; i++) {
		if (qtab[i] != NULL) {
			len += (1 + BLOCK_SIZE * BLOCK_SIZE);
		}
	}
	fputc(len >> 8, fp);
	fputc(len >> 0, fp);
#if DEBUG
	printf("%02x %02x %02x %02x", 0xff, 0xdb, len >> 8, len >> 0);
#endif

	int zz[BLOCK_SIZE * BLOCK_SIZE];
	for (int i = 0; i < 16; i++) {
		if (qtab[i] == NULL) {
			continue;
		}
		fputc(i, fp);
#if DEBUG
		printf(" %02x", i);
#endif
		zigzag(zz, qtab[i]->table);
		for (int j = 0; j < BLOCK_SIZE * BLOCK_SIZE; j++) {
			fputc(zz[j], fp);
#if DEBUG
			printf(" %02x", zz[j]);
#endif
		}
	}
#if DEBUG
	printf("\n");
#endif


	// SOF0
	const unsigned char comp_num = 3;
	const unsigned char sample_factor_v1 = 2;
	const unsigned char sample_factor_h1 = 2;
	const unsigned char sample_factor_v2 = 1;
	const unsigned char sample_factor_h2 = 1;
	len = 2 + 1 + 2 + 2 + 1 + 3 * comp_num;
	fputc(0xff, fp);
	fputc(0xc0, fp);
	fputc(len >> 8, fp);
	fputc(len >> 0, fp);
	fputc(8, fp);
	fputc(jpeg_height >> 8, fp);
	fputc(jpeg_height >> 0, fp);
	fputc(jpeg_width >> 8, fp);
	fputc(jpeg_width >> 0, fp);
	fputc(comp_num, fp);

	fputc(1, fp);
	fputc((sample_factor_h1 << 4) | (sample_factor_v1 << 0), fp);
	fputc(YUV_ENUM::YUV_Y, fp);

	fputc(2, fp);
	fputc((sample_factor_h2 << 4) | (sample_factor_v2 << 0), fp);
	fputc(YUV_ENUM::YUV_C, fp);

	fputc(3, fp);
	fputc((sample_factor_h2 << 4) | (sample_factor_v2 << 0), fp);
	fputc(YUV_ENUM::YUV_C, fp);

#if DEBUG
	printf("%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x", 0xff, 0xc0, len >> 8, len >> 0, 8, jpeg_height >> 8, jpeg_height >> 0, jpeg_width >> 8, jpeg_width >> 0, comp_num);
	printf(" %02x %02x %02x", 1, (sample_factor_h1 << 4) | (sample_factor_v1 << 0), YUV_ENUM::YUV_Y);
	printf(" %02x %02x %02x", 2, (sample_factor_h2 << 4) | (sample_factor_v2 << 0), YUV_ENUM::YUV_C);
	printf(" %02x %02x %02x", 3, (sample_factor_h2 << 4) | (sample_factor_v2 << 0), YUV_ENUM::YUV_C);
	printf("\n");
#endif

	// Huffman table AC
	for (int i = 0; i < 16; i++) {
		if (hac[i] == NULL) continue;
		fputc(0xff, fp);
		fputc(0xc4, fp);

		len = 2 + 1 + 16;
		for (int j = 0; j < MAX_HUFFMAN_CODE_LEN; j++) {
			len += hac[i]->table[j];
		}
		fprintf(stderr, "%d\n", len);

		fputc(len >> 8, fp);
		fputc(len >> 0, fp);
		fputc(0x10 | i, fp); // flag_ac (4 bit) | index (4 bit)
		fwrite(hac[i]->table, 1, len - 3, fp);
#if DEBUG
		printf("%02x %02x %02x %02x %02x", 0xff, 0xc4, len >> 8, len >> 0, 0x10 | i);
		for (int j = 0; j < len - 3; j++) {
			printf(" %02x", hac[i]->table[j]);
		}
		printf("\n");
#endif
	}


	// Huffman table DC
	for (int i = 0; i < 16; i++) {
		if (hdc[i] == NULL) continue;
		fputc(0xff, fp);
		fputc(0xc4, fp);

		len = 2 + 1 + 16;
		for (int j = 0; j < MAX_HUFFMAN_CODE_LEN; j++) {
			len += hdc[i]->table[j];
		}
		fprintf(stderr, "%d\n", len);

		fputc(len >> 8, fp);
		fputc(len >> 0, fp);
		fputc(0x00 | i, fp); // flag_ac (4 bit) | index (4 bit)
		fwrite(hdc[i]->table, 1, len - 3, fp);
#if DEBUG
		printf("%02x %02x %02x %02x %02x", 0xff, 0xc4, len >> 8, len >> 0, 0x00 | i);
		for (int j = 0; j < len - 3; j++) {
			printf(" %02x", hdc[i]->table[j]);
		}
		printf("\n");
#endif
	}


	// SOS
	len = 2 + 1 + 2 * comp_num + 3;
	fputc(0xff, fp);
	fputc(0xda, fp);
	fputc(len >> 8, fp);
	fputc(len >> 0, fp);
	fputc(comp_num, fp);

	fputc(1, fp);
	fputc((YUV_ENUM::YUV_Y << 4) | (YUV_ENUM::YUV_Y << 0), fp);

	fputc(2, fp);
	fputc((YUV_ENUM::YUV_C << 4) | (YUV_ENUM::YUV_C << 0), fp);

	fputc(3, fp);
	fputc((YUV_ENUM::YUV_C << 4) | (YUV_ENUM::YUV_C << 0), fp);

	fputc(0x00, fp);
	fputc(0x3f, fp);
	fputc(0x00, fp);

#if DEBUG
	printf("%02x %02x %02x %02x %02x", 0xff, 0xda, len >> 8, len >> 0, comp_num);
	printf(" %02x %02x", 1, (YUV_ENUM::YUV_Y << 4) | (YUV_ENUM::YUV_Y << 0));
	printf(" %02x %02x", 2, (YUV_ENUM::YUV_C << 4) | (YUV_ENUM::YUV_C << 0));
	printf(" %02x %02x\n", 3, (YUV_ENUM::YUV_C << 4) | (YUV_ENUM::YUV_C << 0));
	printf("%02x %02x %02x\n", 0, 0x3f, 0);
#endif


	// Data
	int b_height = jpeg_height / BLOCK_SIZE;
	int b_width = jpeg_width / BLOCK_SIZE;
	// no parallel
	int dc[3] = {0};
	for (int i = 0; i < b_height; i += 2) {
		for (int j = 0; j < b_width; j += 2) {
			go_encode_block_to_file(blks_y[i][j], dc[0], YUV_ENUM::YUV_Y, fp);
			go_encode_block_to_file(blks_y[i][j + 1], dc[0], YUV_ENUM::YUV_Y, fp);
			go_encode_block_to_file(blks_y[i + 1][j], dc[0], YUV_ENUM::YUV_Y, fp);
			go_encode_block_to_file(blks_y[i + 1][j + 1], dc[0], YUV_ENUM::YUV_Y, fp);

			go_encode_block_to_file(blks_cb[i][j], dc[1], YUV_ENUM::YUV_C, fp);

			go_encode_block_to_file(blks_cr[i][j], dc[2], YUV_ENUM::YUV_C, fp);
		}
	}
	write_bits(0, 8, fp, true);


	// EOI
	fputc(0xff, fp);
	fputc(0xd9, fp);

	fclose(fp);
}

void jpeg_init(int thread_num) {
	for (int i = 0; i < 16; i++) {
		qtab[i] = NULL;
		hdc[i] = NULL;
		hac[i] = NULL;
	}

	qtab[YUV_ENUM::YUV_Y] = new QuanTable(STD_QUAN_TABLE_LUMIN);
	qtab[YUV_ENUM::YUV_C] = new QuanTable(STD_QUAN_TABLE_CHROM);

	hdc[YUV_ENUM::YUV_Y] = new Huffman(STD_HUFTAB_LUMIN_DC);
	hdc[YUV_ENUM::YUV_C] = new Huffman(STD_HUFTAB_CHROM_DC);

	hac[YUV_ENUM::YUV_Y] = new Huffman(STD_HUFTAB_LUMIN_AC);
	hac[YUV_ENUM::YUV_C] = new Huffman(STD_HUFTAB_CHROM_AC);

	// opencl init
	cl_int status;

	// platform
	cl_platform_id platform_id;
	cl_uint platform_id_got;
	status = clGetPlatformIDs(1, &platform_id, &platform_id_got);
	assert(status == CL_SUCCESS && platform_id_got == 1);
	printf("Get platform: %d\n", platform_id_got);

	// device
	cl_device_id GPU[5];
	cl_uint GPU_id_got;
	status = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 5, GPU, &GPU_id_got);
	assert(status == CL_SUCCESS);
	printf("Get GPU: %d\n", GPU_id_got);

	// context
	opencl.context = clCreateContext(NULL, GPU_id_got, GPU, NULL, NULL, &status);
	assert(status == CL_SUCCESS);

	// command queue
	opencl.command_queue = clCreateCommandQueue(opencl.context, GPU[0], 0, &status);
	assert(status == CL_SUCCESS);

	// kernel source
	FILE* kernel_fp = fopen("kernel/kernel.cl", "r");
	assert(kernel_fp != NULL);

	char* kernelBuffer = new char[65536];
	const char* constKernelSource = kernelBuffer;
	size_t kernelLength = fread(kernelBuffer, 1, 65536, kernel_fp);
	cl_program program = clCreateProgramWithSource(opencl.context, 1, &constKernelSource, &kernelLength, &status);
	assert(status == CL_SUCCESS);

	// build program
	status = clBuildProgram(program, GPU_id_got, GPU, NULL, NULL, NULL);
	static char *buffer[65536];
	status = clGetProgramBuildInfo(program, GPU[0], CL_PROGRAM_BUILD_LOG, 65536, buffer, NULL);
	assert(status == CL_SUCCESS);

	// create kernel fdct
	opencl.kernel_fdct = clCreateKernel(program, "fdct", &status);
	assert(status == CL_SUCCESS);

	opencl.work_dim = 1;
	opencl.global_threads[0] = (size_t)(BLOCK_SIZE * BLOCK_SIZE);
	opencl.local_threads[0] = (size_t)1;

	fclose(kernel_fp);
	delete []kernelBuffer;
}

void convert_bmp_to_jpg(const char* input_path, const char* output_path) {
	BMP bmp;
	if (!bmp.read(input_path)) {
		return;
	}

	YUV yuv(bmp.height, bmp.width);

	RGB2YCbCr(yuv, bmp);
	encode(yuv);

	write_to_file(output_path);
}
