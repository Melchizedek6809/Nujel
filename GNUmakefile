ifneq (, $(shell which dash))
	SHELL   := $(shell which dash)
endif

EMCC        := emcc
EMAR        := emar
EMMEM       := -s TOTAL_MEMORY=64MB -s ALLOW_MEMORY_GROWTH=1

LIB_SRCS    := $(shell find lib -type f -name '*.c')
LIB_HDRS    := $(shell find lib -type f -name '*.h')
LIB_OBJS    := $(LIB_SRCS:.c=.o)
LIB_DEPS    := ${LIB_SRCS:.c=.d}
STDLIB_NUJS := $(shell find stdlib -type f -name '*.nuj' | sort)
STDLIB_NOBS := $(STDLIB_NUJS:.nuj=.no)

LIB_WASM_OBJS := $(LIB_SRCS:.c=.wo)
LIB_WASM_DEPS := ${LIB_SRCS:.c=.wd}

NUJEL       := ./nujel
NUJEL_BOOT  := ./nujel-bootstrap
ASSET       := ./tools/assets

CC                   := cc
CC_MUSL              := musl-gcc
CFLAGS               := -g -D_GNU_SOURCE
CSTD                 := -std=c99
OPTIMIZATION         := -O2
WARNINGS             := -Wall -Werror -Wextra -Wshadow -Wcast-align -Wno-missing-braces

LIBS                 := -lm

RELEASE_OPTIMIZATION := -O3 -flto
VERSION_ARCH         := $(shell uname -m)

BIN_SRCS    := $(shell find bin -type f -name '*.c')
BIN_HDRS    := $(shell find bin -type f -name '*.h')
BINLIB_NUJS := $(shell find bin/lib -type f -name '*.nuj' | sort)
BINLIB_NOBS := $(BINLIB_NUJS:.nuj=.no)
ifeq ($(OS),Windows_NT)
	NUJEL := ./nujel.exe
	ASSET := ./tools/assets.exe
	LIBS  += -lpthread
else
	BIN_SRCS += vendor/bestline/bestline.c
endif
BIN_OBJS    := $(BIN_SRCS:.c=.o)
BIN_DEPS    := ${BIN_SRCS:.c=.d}

BIN_WASM_OBJS := $(BIN_SRCS:.c=.wo)
BIN_WASM_DEPS := $(BIN_SRCS:.c=.wd)

all: $(NUJEL)
.PHONY: all release release.musl

include mk/ansi_colors.mk
include mk/disable_implicit_rules.mk

ifneq ($(MAKECMDGOALS),clean)
-include $(LIB_DEPS)
-include $(BIN_DEPS)
-include $(LIB_WASM_DEPS)
-include $(BIN_WASM_DEPS)
endif

ifdef EMSDK
all: nujel.wa
endif

.PHONY: clean
clean:
	@rm -f -- nujel nujel.exe nujel.a nujel.wa nujel.com nujel.com.dbg tools/assets tools/assets.exe
	@rm -f -- $(shell find bin lib -type f -name '*.o')
	@rm -f -- $(shell find bin lib -type f -name '*.wo')
	@rm -f -- $(shell find bin lib -type f -name '*.d')
	@rm -f -- $(shell find bin lib -type f -name '*.wd')
	@rm -f -- $(shell find bin/lib stdlib -type f -name '*.no')
	@rm -f ./callgrind.out.*
	@rm -f ./web/index.html ./web/index.js ./web/index.wasm
	@rm -rf tmp
	@echo "$(ANSI_BG_RED)" "[CLEAN]" "$(ANSI_RESET)" "nujel"

%.o: %.c
	@$(CC) -o $@ -c $< $(CFLAGS) $(CINCLUDES) $(OPTIMIZATION) $(WARNINGS) $(CSTD) -MMD > ${<:.c=.d}
	@echo "$(ANSI_GREEN)" "[CC] " "$(ANSI_RESET)" $@

%.no: %.nuj | $(NUJEL_BOOT)
	@$(NUJEL_BOOT) -x "[file/compile \"$^\"]"
	@echo "$(ANSI_PINK)" "[NUJ]" "$(ANSI_RESET)" $@

%.wo: %.c
	@$(EMCC) -o $@ -c $< $(CFLAGS) $(CINCLUDES) $(OPTIMIZATION) $(WARNINGS) $(CSTD) -MMD > ${<:.c=.wd}
	@echo "$(ANSI_GREEN)" "[WCC]" "$(ANSI_RESET)" $@

nujel.wa: $(LIB_WASM_OBJS) tmp/stdlib.wo
	@rm -rf $@
	@$(EMAR) cq $@ $^
	@echo "$(ANSI_BG_CYAN)" "[WAR]" "$(ANSI_RESET)" $@

$(ASSET): tools/assets.c
	@$(CC) -o $@    $^ $(CFLAGS) $(CINCLUDES) $(OPTIMIZATION) $(WARNINGS) $(CSTD) $(LIBS)
	@echo "$(ANSI_BG_GREY)" "[CC] " "$(ANSI_RESET)" $@

nujel.a: $(LIB_OBJS)
	@rm -rf $@
	@ar cq $@ $^
	@echo "$(ANSI_BG_CYAN)" "[AR] " "$(ANSI_RESET)" $@

$(NUJEL): $(BIN_OBJS) nujel.a tmp/stdlib.o tmp/binlib.o
	@$(CC) -o $@ $^ $(CFLAGS) $(CINCLUDES) $(OPTIMIZATION) $(WARNINGS) $(CSTD) $(LIBS)
	@echo "$(ANSI_BG_GREEN)" "[CC] " "$(ANSI_RESET)" $@

$(NUJEL_BOOT): $(BIN_OBJS) nujel.a bootstrap/stdlib.o bootstrap/binlib.o
	@$(CC) -o $@ $^ $(CFLAGS) $(CINCLUDES) $(OPTIMIZATION) $(WARNINGS) $(CSTD) $(LIBS)
	@echo "$(ANSI_BG_GREEN)" "[CC] " "$(ANSI_RESET)" $@
	@$(NUJEL_BOOT) -x "[exit [test-run]]"

release: $(BIN_SRCS) $(LIB_SRCS) tmp/stdlib.c tmp/binlib.c
	@rm -f $(NUJEL)
	$(CC) -s -o $(NUJEL) $^ $(CFLAGS) $(CINCLUDES) $(RELEASE_OPTIMIZATION) $(CSTD) $(LIBS)
	@$(NUJEL) -x "[exit [test-run]]"
	@echo "$(ANSI_BG_GREEN)" "[CC] " "$(ANSI_RESET)" $(NUJEL)

release.musl: $(BIN_SRCS) $(LIB_SRCS) tmp/stdlib.c tmp/binlib.c
	@rm -f $(NUJEL)
	$(CC_MUSL) -s -static -o $(NUJEL) $^ $(CFLAGS) $(CINCLUDES) $(RELEASE_OPTIMIZATION) $(CSTD) $(LIBS)
	@$(NUJEL) -x "[exit [test-run]]"
	@echo "$(ANSI_BG_GREEN)" "[CC] " "$(ANSI_RESET)" $(NUJEL)

web/index.html: nujel.wa $(BIN_WASM_OBJS) tmp/binlib.wo
	@mkdir -p releases/wasm/
	$(EMCC) $^ -D_GNU_SOURCE $(CSTD) -O3 -s EXPORTED_FUNCTIONS="['_main','_run']" -s EXPORTED_RUNTIME_METHODS=["ccall","cwrap"] -fno-rtti --closure 0 $(EMMEM) --shell-file web/shell.html -o $@

release.wasm: web/index.html

bootstrap/stdlib.c: bootstrap/stdlib.no $(ASSET)
	@$(ASSET) bootstrap/stdlib bootstrap/stdlib.no
	@echo "$(ANSI_GREY)" "[ST] " "$(ANSI_RESET)" $@

bootstrap/binlib.c: bootstrap/binlib.no $(ASSET)
	@$(ASSET) bootstrap/binlib bootstrap/binlib.no
	@echo "$(ANSI_GREY)" "[ST] " "$(ANSI_RESET)" $@

tmp/stdlib.nuj: $(STDLIB_NUJS)
	@mkdir -p tmp/
	@cat $^ > $@
	@echo "$(ANSI_GREY)" "[CAT]" "$(ANSI_RESET)" $@

tmp/stdlib.no: $(STDLIB_NOBS)
	@mkdir -p tmp/
	@cat $^ > $@
	@echo "$(ANSI_GREY)" "[CAT]" "$(ANSI_RESET)" $@

tmp/binlib.nuj: $(BINLIB_NUJS)
	@mkdir -p tmp/
	@cat $^ > $@
	@echo "$(ANSI_GREY)" "[CAT]" "$(ANSI_RESET)" $@

tmp/binlib.no: $(BINLIB_NOBS)
	@mkdir -p tmp/
	@cat $^ > $@
	@echo "$(ANSI_GREY)" "[CAT]" "$(ANSI_RESET)" $@

tmp/stdlib.c: tmp/stdlib.no $(ASSET)
	@mkdir -p tmp/
	@$(ASSET) tmp/stdlib tmp/stdlib.no
	@echo "$(ANSI_GREY)" "[ST] " "$(ANSI_RESET)" $@
tmp/stdlib.h: tmp/stdlib.c
	@true

tmp/binlib.c: tmp/binlib.no $(ASSET)
	@mkdir -p tmp/
	@$(ASSET) tmp/binlib tmp/binlib.no
	@echo "$(ANSI_GREY)" "[ST] " "$(ANSI_RESET)" $@
tmp/binlib.h: tmp/binlib.c
	@true

.PHONY: test
test: $(NUJEL)
	@$(NUJEL) -x "[exit [test-run]]"

.PHONY: run
run: $(NUJEL)
	@$(NUJEL) -x "[exit [test-run]]"

.PHONY: rund
rund: $(NUJEL)
	gdb $(NUJEL) -ex "r"

.PHONY: runn
runn: $(NUJEL)
	$(NUJEL)

.PHONY: runn
runng: $(NUJEL) tmp/stdlib.no tmp/binlib.no
	$(NUJEL) -n tmp/stdlib.no tmp/binlib.no --

.PHONY: runn
testng: $(NUJEL) tmp/stdlib.no tmp/binlib.no
	$(NUJEL) -n tmp/stdlib.no tmp/binlib.no -x "[quit [test-run]]"

.PHONY: profile
profile: $(NUJEL)
	valgrind --tool=callgrind --dump-instr=yes $(NUJEL) -x "[test-run]"

.PHONY: profile-while
profile-while: $(NUJEL)
	valgrind --tool=callgrind --dump-instr=yes $(NUJEL) -x "[display [let* [def v 0] [while [< v 2,000,000] [set! v [+ 1 v]]] v]]"

.PHONY: web
web:
	./tools/buildwasm
	rsync -avhe ssh --delete ./web/ wolkenwelten.net:/var/www/html/nujel/
