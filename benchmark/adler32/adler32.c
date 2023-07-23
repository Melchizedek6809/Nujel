#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

uint32_t adler32(const void *msg, size_t len) {
	uint32_t a = 1;
	uint32_t b = 0;
	const uint8_t *p = msg;
	for (int i=0;i<len;i++){
		a = (a + p[i]) % 65521;
		b = (a + b) % 65521;
	}
	return a | (b << 16);
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

void adler32sum(const char *filename) {
	size_t len;
	const char *msg = loadFile(filename, &len);
	const uint32_t result = adler32(msg, len);
	printf("%8.8X  %s\n", result, filename);
}

int main(int argc, char **argv) {
	/*
	if (argc < 2) {
		printf("usage: %s [FILE]...\n", argv[0]);
		return 1;
	}

	for(int i=1;i<argc;i++){
		adler32sum(argv[i]);
	}
	*/
	adler32sum("test-files/r5rs.pdf");
	return 0;
}
