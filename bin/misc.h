#pragma once
#include "../lib/common.h"

void *loadFile(const char *filename, size_t *len);
void  saveFile(const char *filename, const void *buf, size_t len);

int  makeDir         (const char *name);
int  makeDirR        (const char *name);
void rmDirR          (const char *name);
void changeToDataDir ();
const char *tempFilename();
