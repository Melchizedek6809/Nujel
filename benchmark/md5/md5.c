/* Heavily modified version of https://github.com/pod32g/MD5 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <byteswap.h>

struct md5_result {
	uint32_t h[4];
};

static inline uint32_t leftRotate(uint32_t x, uint32_t c){
	return (((x) << (c)) | ((x) >> (32 - (c))));
}

struct md5_result md5(const void *initial_msg, size_t initial_len) {
	const uint32_t r[] = {
		7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
		5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
		4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
		6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};

	const uint32_t k[] = {
		0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
		0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
		0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
		0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
		0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
		0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
		0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
		0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
		0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
		0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
		0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
		0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
		0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
		0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
		0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
		0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

	struct md5_result result = {{
		0x67452301,
		0xefcdab89,
		0x98badcfe,
		0x10325476}};

	const int new_len = ((((initial_len + 8) >> 6) + 1) << 6) - 8;

	uint8_t *msg = calloc(new_len + 64, 1);
	memcpy(msg, initial_msg, initial_len);
	msg[initial_len] = 0x80;

	const uint32_t bits_len = 8*initial_len;
	memcpy(msg + new_len, &bits_len, 4);

	for(int offset=0; offset<new_len; offset += (512/8)) {
		const uint32_t *w = (uint32_t *) (msg + offset);
		uint32_t a = result.h[0];
		uint32_t b = result.h[1];
		uint32_t c = result.h[2];
		uint32_t d = result.h[3];

		for(int i = 0; i<64; i++) {
			uint32_t f, g;
			switch(i >> 4){
			default:
			case 0:
				f = (b & c) | ((~b) & d);
				g = i;
				break;
			case 1:
				f = (d & b) | ((~d) & c);
				g = (5*i + 1) & 0xF;
				break;
			case 2:
				f = b ^ c ^ d;
				g = (3*i + 5) & 0xF;
				break;
			case 3:
				f = c ^ (b | (~d));
				g = (7*i) & 0xF;
				break;
			}
			const uint32_t temp = d;
			d = c;
			c = b;
			b = b + leftRotate((a + f + k[i] + w[g]), r[i]);
			a = temp;
		}
		result.h[0] += a;
		result.h[1] += b;
		result.h[2] += c;
		result.h[3] += d;
	}
	free(msg);
	return result;
}

void *loadFile(const char *filename, size_t *len){
	FILE *fp;
	size_t filelen,readlen,read;
	uint8_t *buf = NULL;

	fp = fopen(filename,"rb");
	if(fp == NULL){return NULL;}

	fseek(fp,0,SEEK_END);
	filelen = ftell(fp);
	fseek(fp,0,SEEK_SET);

	buf = malloc(filelen);
	if(buf == NULL){return NULL;}

	readlen = 0;
	while(readlen < filelen){
		read = fread(buf+readlen,1,filelen-readlen,fp);
		if(read == 0){
			free(buf);
			return NULL;
		}
		readlen += read;
	}
	fclose(fp);

	*len = filelen;
	return buf;
}

void md5sum(const char *filename) {
	size_t len;
	const char *msg = loadFile(filename, &len);
	struct md5_result result = md5(msg, len);
	printf("%8.8x%8.8x%8.8x%8.8x  %s\n",
		__bswap_32(result.h[0]),
		__bswap_32(result.h[1]),
		__bswap_32(result.h[2]),
		__bswap_32(result.h[3]),
		filename);
}

int main(int argc, char **argv) {
	/*
	if (argc < 2) {
		printf("usage: %s [FILE]...\n", argv[0]);
		return 1;
	}

	for(int i=1;i<argc;i++){
		md5sum(argv[i]);
	}
	*/
	md5sum("test-files/r5rs.pdf");
	return 0;
}
