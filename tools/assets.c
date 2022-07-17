/* Nujel - Copyright (C) 2020-2021 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE
 */
#include <stdio.h>

char *fileSlug(char *path){
	static char ret[1024];
	char c,*r,*start;
	r = ret;
	start = NULL;
	while((c = *path++)){
		if((c == '/') || (c == '.') || (c < 32)){
			if(start == NULL){start = r+1;}
			*r++ = '_';
			continue;
		}
		*r++ = c;
	}
	*r = 0;
	return start;
}

int main(int argc,char *argv[]){
	FILE *cfp = fopen(argv[1],"w");
	static unsigned char buffer[1024*1024*32];
	size_t filelen=0,readlen=0,ii;
	int lc=0,i;
	if(cfp == NULL){
		fprintf(stderr,"Error opening src/tmp/assets*");
		return 1;
	}
	for(i=2;i<argc;i++){
		FILE *afp = fopen(argv[i],"rb");
		if(afp == NULL){
			fprintf(stderr,"Error opening %s\n",argv[i]);
			return 2;
		}
		readlen=0;
		fseek(afp,0,SEEK_END);
		filelen = ftell(afp);
		fseek(afp,0,SEEK_SET);

		while(readlen < filelen){
			readlen += fread(buffer+readlen,1,filelen-readlen,afp);
		}
		fclose(afp);

		fprintf(cfp,"unsigned  int %s_len    = %u;\n",fileSlug(argv[i]),(int)filelen);
		fprintf(cfp,"unsigned char %s_data[] = {\n ",fileSlug(argv[i]));
		lc=0;
		for(ii=0;ii<filelen;ii++){
			fprintf(cfp,"0x%02X,",buffer[ii]);
			if(++lc > 19){
				fprintf(cfp,"\n ");
				lc=0;
			}
		}
		fprintf(cfp,"0\n};\n\n");
	}
	fclose(cfp);
	return 0;
}
