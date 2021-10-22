#pragma once
#include "../lib/common.h"

void *loadFile(const char *filename,size_t *len);
void saveFile(const char *filename,const void *buf, size_t len);
