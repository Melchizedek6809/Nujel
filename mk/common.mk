prefix               := /usr/local
exec_prefix           = $(prefix)
bindir                = $(exec_prefix)/bin/

CC                   := cc
CAT                  := cat
AR                   := ar
INSTALL              := install
STRIP                := strip

EMCC                 := emcc
EMAR                 := emar
EMMEM                := -s TOTAL_MEMORY=96MB -s ALLOW_MEMORY_GROWTH=1

NUJEL                := nujel
NUJEL_BOOTSTRAP      := nujel-bootstrap
ASSET                := tools/assets
PROG                  = $(NUJEL)

CC_MUSL              := musl-gcc
CFLAGS               := -g -D_GNU_SOURCE
CINCLUDES            :=
LDFLAGS              :=
CSTD                 := -std=c99
OPTIMIZATION         := -O2
WARNINGS             := -Wall -Werror -Wextra -Wshadow -Wcast-align -Wno-missing-braces

LIBS                 := -lm

RELEASE_OPTIMIZATION := -O3
VERSION_ARCH         := $(shell uname -m)
