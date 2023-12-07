/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "private.h"
#endif

extern u8 binlib_no_data[];

static lClosure *initBinRootClosure(lClosure *c){
	lOperationsIO(c);
	lOperationsPort(c);
	lOperationsInit(c);
	lOperationsNet(c);
	return c;
}

/* Return a new root Closure, with all native functions in place */
static lClosure *createRootClosure(lClosure *c){
	return lLoad(initBinRootClosure(c), (const char *)binlib_no_data);;
}

static lClosure *createRootClosureFromExternalImage(const char *filename, lVal *init){
	size_t len = 0;
	void *img = loadFile(filename, &len);
	initBinRootClosure(lInitRootClosure());
	lVal imgVal = readImage(img, len, false);
	lClosure *imgC = findRoot(imgVal);
	if(imgC == NULL){
		fprintf(stderr,"Can't determine root closure of that image, exiting\n");
		exit(131);
	}
	lClosure *c = lRedefineNativeFuncs(imgC);
	*init = imgVal;
	free(img);
	return c;
}

/* Initialize the Nujel context with an stdlib as well
 * as parsing arguments passed to the runtime */
int initNujel(int argc, char *argv[]){
	lClosure *c = NULL;
	lVal ret = NIL;
	lVal init = NIL;
	for(int i = argc-1; i >= 0; i--){
		if(strcmp(argv[i], "--base-image") == 0){
			if(c != NULL){
				fprintf(stderr, "You can only specify one image\n");
				exit(124);
			}
			if(i > (argc-2)){
				fprintf(stderr, "Please specify an image\n");
				exit(125);
			}
			c = createRootClosureFromExternalImage(argv[i+1], &init);
			ret = lCdr(ret);
			continue;
		}
		ret = lCons(lValString(argv[i]), ret);
	}
	if(c == NULL){
		c = createRootClosure(lNewRoot());
	}
	initEnvironmentMap(c);

	if(init.type == ltNil){
		init = lGetClosureSym(c, lSymS("init"));
	}
	lApply(init, ret);
	return 0;
}

int main(int argc, char *argv[]){
	(void)argc; (void)argv;
	// printf("%s",stdlib_no_data); return 0;
	setvbuf(stdout, NULL, _IONBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);
	lInit();
	setIOSymbols();

	return initNujel(argc,argv);
}
