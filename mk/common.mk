prefix               := /usr/local
exec_prefix           = $(prefix)
bindir                = $(exec_prefix)/bin/

AFL_CC               := afl-gcc
AFL_FUZZ             := afl-fuzz
FUZZ_NUJEL           := fuzz-nujel
CC                   := cc
CAT                  := cat
AR                   := ar
INSTALL              := install
STRIP                := strip

EMCC                 := emcc
EMAR                 := emar
EMMEM                := -s TOTAL_MEMORY=96MB -s ALLOW_MEMORY_GROWTH=1

NUJEL                := nujel
FUTURE_NUJEL         := future-nujel
PROG                  = $(NUJEL)

CC_MUSL              := musl-gcc
CFLAGS               := -g -D_GNU_SOURCE
CINCLUDES            :=
LDFLAGS              :=
CSTD                 := -std=c99
OPTIMIZATION         := -O2
WARNINGS             := -Wall -Werror -Wextra -Wshadow -Wcast-align -Wno-missing-braces

LIBS                 := -lm

RELEASE_OPTIMIZATION := -O2
VERSION_ARCH         := $(shell uname -m)

WASI_CLANG           := clang

WASI_SDK_PATH        := /usr/share/wasi-sysroot/
