include mk/ansi_colors.mk
include mk/common.mk
include mk/disable_implicit_rules.mk

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Linux)
	LIBS := -lm -lrt
endif

LIB_SRCS      := $(shell find lib -type f -name '*.c')
LIB_OBJS      := $(LIB_SRCS:.c=.o)
LIB_WASM_OBJS := $(LIB_SRCS:.c=.wo)
STDLIB_NUJS   := $(shell find stdlib -type f -name '*.nuj' | sort)
STDLIB_MODS   := $(shell find stdlib_modules -type f -name '*.nuj' | sort)
STDLIB_NOBS   := $(STDLIB_NUJS:.nuj=.no)
STDLIB_MOBS   := $(STDLIB_MODS:.nuj=.no)

ifeq (, $(shell which $(CC)))
CC                   := gcc
endif
ifeq (, $(shell which $(CC)))
CC                   := clang
endif
ifeq (, $(shell which $(CC)))
CC                   := tcc
endif

BIN_SRCS    := $(shell find bin -type f -name '*.c')
BIN_OBJS    := $(BIN_SRCS:.c=.o)
BIN_WASM_OBJS := $(BIN_SRCS:.c=.wo)
BINLIB_NUJS := $(shell find binlib -type f -name '*.nuj' | sort)
BINLIB_NOBS := $(BINLIB_NUJS:.nuj=.no)

FUTURE_SRCS = $(BIN_SRCS) $(LIB_SRCS) tmp/image.c
FUTURE_OBJS = $(BIN_OBJS) $(LIB_OBJS) tmp/image.o

RUNTIME_SRCS = $(BIN_SRCS) $(LIB_SRCS) bootstrap/image.c
RUNTIME_OBJS = $(BIN_OBJS) $(LIB_OBJS) bootstrap/image.o

ifeq ($(OS),Windows_NT)
	NUJEL   := nujel.exe
	LIBS    += -lpthread
	STATIC_LIBS := -static -lpthread
	LDFLAGS := -Wl,--stack,16777216
endif

ifeq ($(shell uname -s),Darwin)
	STATIC_LIBS := -lm
endif

all: $(NUJEL)
.PHONY: all release release.musl release.amalgamation
.PHONY: rund runn install install.musl profile web
.PHONY: test.future check test test.verbose test.debug test.slow test.slow.debug test.ridiculous test.wasm

ifdef EMSDK
all: nujel.wa
endif

FILES_TO_CLEAN := $(shell find bin lib bootstrap binlib stdlib -type f -name '*.o' -o -name '*.wo' -o -name '*.obj' -o -name '*.d' -o -name '*.wd' -o -name '*.deps')
NOBS_TO_CLEAN  := $(shell find binlib stdlib stdlib_modules -type f -name '*.no')

$(BIN_OBJS): lib/nujel.h lib/nujel-private.h bin/private.h
$(LIB_OBJS): lib/nujel.h lib/nujel-private.h

%.o: %.c
	@$(CC) -o $@ -c $< $(CFLAGS) $(CINCLUDES) $(OPTIMIZATION) $(WARNINGS) $(CSTD)
	@echo "$(ANSI_GREEN)" "[CC] " "$(ANSI_RESET)" $@

%.wo: %.c
	@$(EMCC) -o $@ -c $< $(CFLAGS) $(CINCLUDES) $(OPTIMIZATION) $(WARNINGS) $(CSTD)
	@echo "$(ANSI_GREEN)" "[WCC]" "$(ANSI_RESET)" $@

nujel.wa: $(LIB_WASM_OBJS) tmp/stdlib.wo
	@rm -rf $@
	@$(EMAR) cq $@ $^
	@echo "$(ANSI_BG_CYAN)" "[WAR]" "$(ANSI_RESET)" $@

nujel.a: $(LIB_OBJS)
	@rm -rf $@
	@$(AR) cq $@ $^
	@echo "$(ANSI_BG_CYAN)" "[AR] " "$(ANSI_RESET)" $@

$(NUJEL): $(RUNTIME_OBJS)
	@$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS) $(CINCLUDES) $(OPTIMIZATION) $(WARNINGS) $(CSTD) $(LIBS)
	@echo "$(ANSI_BG_GREEN)" "[CC] " "$(ANSI_RESET)" $@

$(FUTURE_NUJEL): $(FUTURE_OBJS)
	@$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS) $(CINCLUDES) $(OPTIMIZATION) $(WARNINGS) $(CSTD) $(LIBS)
	@echo "$(ANSI_BG_GREEN)" "[CC] " "$(ANSI_RESET)" $@

release: $(RUNTIME_SRCS)
	@rm -f $(NUJEL)
	@$(CC) -o $(NUJEL) $^ $(CFLAGS) $(CINCLUDES) $(RELEASE_OPTIMIZATION) $(CSTD) $(LIBS) $(LDFLAGS)
	@$(STRIP) -xS $(NUJEL)
	@echo "$(ANSI_BG_GREEN)" "[CC] " "$(ANSI_RESET)" $(NUJEL)

release.future: $(FUTURE_SRCS)
	@rm -f $(FUTURE_NUJEL)
	@$(CC) -o $(FUTURE_NUJEL) $^ $(CFLAGS) $(CINCLUDES) $(RELEASE_OPTIMIZATION) $(CSTD) $(LIBS) $(LDFLAGS)
	@$(STRIP) -xS $(FUTURE_NUJEL)
	@echo "$(ANSI_BG_GREEN)" "[CC] " "$(ANSI_RESET)" $(FUTURE_NUJEL)

release.static: $(RUNTIME_SRCS)
	@rm -f $(NUJEL)
	@gcc -s -static -o $(NUJEL) $^ $(CFLAGS) $(CINCLUDES) $(RELEASE_OPTIMIZATION) $(CSTD) $(LIBS)  $(LDFLAGS)
	@$(STRIP) -xS $(NUJEL)
	@echo "$(ANSI_BG_GREEN)" "[CC] " "$(ANSI_RESET)" $(NUJEL)

release.musl: $(RUNTIME_SRCS)
	@rm -f $(NUJEL)
	@musl-gcc -s -static -o $(NUJEL) $^ $(CFLAGS) $(CINCLUDES) $(RELEASE_OPTIMIZATION) $(CSTD) $(LIBS)  $(LDFLAGS)
	@$(STRIP) -xS $(NUJEL)
	@echo "$(ANSI_BG_GREEN)" "[CC] " "$(ANSI_RESET)" $(NUJEL)

release.san: $(RUNTIME_SRCS)
	@rm -f $(NUJEL)
	$(CC) -fsanitize=address -fsanitize=undefined -fsanitize-undefined-trap-on-error -g  -Og -fno-lto -o $(NUJEL) $^ $(CFLAGS) $(CINCLUDES) $(CSTD) $(LIBS)
	@echo "$(ANSI_BG_GREEN)" "[CC] " "$(ANSI_RESET)" $(NUJEL)

release.san.future: $(FUTURE_SRCS)
	@rm -f $(FUTURE_NUJEL)
	$(CC) -g  -Og -fno-lto -o $(FUTURE_NUJEL) $^ $(CFLAGS) $(CINCLUDES) $(CSTD) $(LIBS)
	@echo "$(ANSI_BG_GREEN)" "[CC] " "$(ANSI_RESET)" $(FUTURE_NUJEL)

release.amalgamation: nujel.c
	@$(CC) -o $(NUJEL) nujel.c $(CFLAGS) $(CINCLUDES) $(RELEASE_OPTIMIZATION) $(CSTD) $(LIBS)  $(LDFLAGS)
	@$(STRIP) -xS $(NUJEL)
	@echo "$(ANSI_BG_GREEN)" "[CC] " "$(ANSI_RESET)" $(NUJEL)

nujel.wasm: $(RUNTIME_SRCS)
	@rm -f $(NUJEL)
	@$(WASI_CLANG) --target=wasm32-unknown-wasi --sysroot=$(WASI_SDK_PATH) -o nujel.wasm $^ $(CFLAGS) $(CINCLUDES) $(RELEASE_OPTIMIZATION) $(CSTD) $(LIBS) $(LDFLAGS)
	@echo "$(ANSI_BG_GREEN)" "[CC] " "$(ANSI_RESET)" nujel.wasm

web/index.html: nujel.wa $(BIN_WASM_OBJS) tmp/binlib.wo
	@mkdir -p releases/wasm/
	$(EMCC) $^ -D_GNU_SOURCE $(CSTD) -O3 -s EXPORTED_FUNCTIONS="['_main','_run']" -s EXPORTED_RUNTIME_METHODS=["ccall","cwrap"] -fno-rtti --closure 0 $(EMMEM) --shell-file web/shell.html -o $@

nujel.h: mk/amalgamation/prefix.h lib/nujel.h lib/nujel-private.h mk/amalgamation/implementation-prefix.h $(LIB_SRCS) tmp/stdlib.c mk/amalgamation/implementation-suffix.h mk/amalgamation/suffix.h
	@$(CAT) $^ > nujel.h
	@echo "$(ANSI_BG_GREEN)" "[CAT]" "$(ANSI_RESET)" nujel.h

nujel.c: mk/amalgamation/bin-prefix.h mk/amalgamation/prefix.h lib/nujel.h lib/nujel-private.h bin/private.h mk/amalgamation/implementation-prefix.h $(LIB_SRCS) $(BIN_SRCS) bootstrap/stdlib.c bootstrap/binlib.c mk/amalgamation/implementation-suffix.h mk/amalgamation/suffix.h
	@$(CAT) $^ > nujel.c
	@echo "$(ANSI_BG_GREEN)" "[CAT]" "$(ANSI_RESET)" nujel.c

include mk/targets.mk
