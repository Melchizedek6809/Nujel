ifneq (, $(shell which dash))
	SHELL   := $(shell which dash)
endif

EMCC        := emcc
EMAR        := emar

LIB_SRCS    := $(shell find lib -type f -name '*.c')
LIB_HDRS    := $(shell find lib -type f -name '*.h')
LIB_OBJS    := $(LIB_SRCS:.c=.o)
LIB_DEPS    := ${LIB_SRCS:.c=.d}
STDLIB_NUJS := $(shell find stdlib -type f -name '*.nuj')

LIB_WASM_OBJS := $(LIB_SRCS:.c=.wo)
LIB_WASM_DEPS := ${LIB_SRCS:.c=.wd}

BIN_SRCS    := $(shell find bin -type f -name '*.c')
BIN_HDRS    := $(shell find bin -type f -name '*.h')
BIN_OBJS    := $(BIN_SRCS:.c=.o)
BIN_DEPS    := ${BIN_SRCS:.c=.d}
BINLIB_NUJS := $(shell find bin/lib -type f -name '*.nuj')

NUJEL       := ./nujel
ASSET       := ./tools/assets
ifeq ($(OS),Windows_NT)
	NUJEL := ./nujel.exe
	ASSET := ./tools/assets.exe
endif

CC                   := cc
CFLAGS               := -g -D_GNU_SOURCE
CSTD                 := -std=c99
OPTIMIZATION         := -O2 -fno-lto -ffast-math -freciprocal-math
WARNINGS             := -Wall -Werror -Wextra -Wshadow -Wcast-align -Wno-missing-braces

LIBS                 := -lm

RELEASE_OPTIMIZATION := -O3 -flto -ffast-math -freciprocal-math
VERSION_ARCH         := $(shell uname -m)

all: $(NUJEL)
.PHONY: all release .deps

include mk/disable_implicit_rules.mk

ifneq ($(MAKECMDGOALS),clean)
-include $(LIB_DEPS)
-include $(BIN_DEPS)
-include $(LIB_WASM_DEPS)
endif

ifdef EMSDK
all: nujel.wa
endif

.PHONY: clean
clean:
	rm -f nujel nujel.exe nujel.a nujel.wa tools/assets tools/assets.exe $(shell find bin lib -type f -name '*.o') $(shell find bin lib -type f -name '*.wo') $(shell find bin lib -type f -name '*.d') $(shell find bin lib -type f -name '*.wd')
	rm -rf tmp

%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS) $(CINCLUDES) $(OPTIMIZATION) $(WARNINGS) $(CSTD) -MMD > ${<:.c=.d}

%.wo: %.c
	$(EMCC) -o $@ -c $< $(CFLAGS) $(CINCLUDES) $(OPTIMIZATION) $(WARNINGS) $(CSTD) -MMD > ${<:.c=.wd}

nujel.wa: $(LIB_WASM_OBJS) tmp/stdlib.wo
	rm -rf $@
	$(EMAR) cq $@ $^

$(ASSET): tools/assets.c
	$(CC) -o $@    $^ $(CFLAGS) $(CINCLUDES) $(OPTIMIZATION) $(WARNINGS) $(CSTD) $(LIBS)

nujel.a: $(LIB_OBJS) tmp/stdlib.o
	rm -rf $@
	ar cq $@ $^

$(NUJEL): $(BIN_OBJS) tmp/binlib.o nujel.a
	$(CC) -o $@ $^ $(CFLAGS) $(CINCLUDES) $(OPTIMIZATION) $(WARNINGS) $(CSTD) $(LIBS)

tmp/stdlib.nuj: $(STDLIB_NUJS)
	@mkdir -p tmp/
	cat $^ > $@

tmp/binlib.nuj: $(BINLIB_NUJS)
	@mkdir -p tmp/
	cat $^ > $@

tmp/stdlib.c: tmp/stdlib.nuj $(ASSET)
	@mkdir -p tmp/
	$(ASSET) tmp/stdlib tmp/stdlib.nuj
tmp/stdlib.h: tmp/stdlib.c
	@true

tmp/binlib.c: tmp/binlib.nuj $(ASSET)
	@mkdir -p tmp/
	$(ASSET) tmp/binlib tmp/binlib.nuj
tmp/binlib.h: tmp/binlib.c
	@true

.PHONY: test
test: $(NUJEL)
	$(NUJEL) -x "[quit [test-run]]"

.PHONY: run
run: $(NUJEL)
	$(NUJEL) -x "[quit [test-run]]"

.PHONY: rund
rund: $(NUJEL)
	gdb $(NUJEL) -ex "r"

.PHONY: runn
runn: $(NUJEL)
	$(NUJEL)
