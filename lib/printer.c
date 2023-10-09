/* Nujel - Copyright (C) 2020-2022 - Benjamin Vincent Schulenburg
 * This project uses the MIT license, a copy should be included under /LICENSE */
#ifndef NUJEL_AMALGAMATION
#include "nujel-private.h"
#endif

static void simplePrintTree(lTree *t){
	if(!t){return;}
	simplePrintTree(t->left);
	if(t->key){
		fprintf(stderr, ":%s ", t->key->c);
		simplePrintVal(t->value);
	}
	simplePrintTree(t->right);
}

/* Super simple printer, not meant for production use, but only as a tool of last resort, for example when
 * we throw past the root exception handler.
 */
void simplePrintVal(lVal v){
	switch(v.type){
	default:
		fprintf(stderr, "#<not-printable-from-c %i> ", v.type);
		break;
	case ltEnvironment:
		fprintf(stderr, "#<env>");
		break;
	case ltBytecodeArr:
		fprintf(stderr, "#<bc-arr>");
		break;
	case ltLambda:
		fprintf(stderr, "#<fn>");
		break;
	case ltNativeFunc:
		fprintf(stderr, "#<NFn>");
		break;
	case ltTree:
		fprintf(stderr, "{");
		simplePrintTree(v.vTree->root);
		fprintf(stderr, "}");
		break;
	case ltNil:
		fprintf(stderr, "#nil");
		break;
	case ltBool:
		fprintf(stderr, "%s", v.vBool ? "#t" : "#f");
		break;
	case ltInt:
		fprintf(stderr, "%lli", (long long int)v.vInt);
		break;
	case ltFloat:
		fprintf(stderr, "%f", v.vFloat);
		break;
	case ltBuffer:
		fprintf(stderr, "#<buf>");
		break;
	case ltString:
		fprintf(stderr, "\"%s\"", (const char *)lBufferData(v.vString));
		break;
	case ltKeyword:
		fprintf(stderr, ":"); // fall-through
	case ltSymbol:
		fprintf(stderr, "%s", v.vSymbol->c);
		break;
	case ltPair: {
		fprintf(stderr, "(");
		lVal t = v;
		bool first = true;
		for(; t.type == ltPair; t = lCdr(t)){
			if(!first){
				fprintf(stderr, " ");
			}
			first = false;
			simplePrintVal(t.vList->car);
		}
		if(t.type != ltNil){
			fprintf(stderr, ". ");
			simplePrintVal(t);
		}
		fprintf(stderr, ")");
		break; }
	case ltArray:
		fprintf(stderr, "##(");
		for(int i=0;i<v.vArray->length;i++){
			simplePrintVal(v.vArray->data[i]);
		}
		fprintf(stderr, ") ");
	}
}
