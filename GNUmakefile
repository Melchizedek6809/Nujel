prefix      := /usr/local
exec_prefix  = $(prefix)
bindir       = $(exec_prefix)/bin/

INSTALL     := install
EMCC        := emcc
EMAR        := emar
EMMEM       := -s TOTAL_MEMORY=96MB -s ALLOW_MEMORY_GROWTH=1

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
ifeq (, $(shell which $(CC)))
CC                   := gcc
endif
ifeq (, $(shell which $(CC)))
CC                   := clang
endif
ifeq (, $(shell which $(CC)))
CC                   := tcc
endif

CC_MUSL              := musl-gcc
CFLAGS               := -g -D_GNU_SOURCE
LDFLAGS              :=
CSTD                 := -std=c99
OPTIMIZATION         := -O2
WARNINGS             := -Wall -Werror -Wextra -Wshadow -Wcast-align -Wno-missing-braces

LIBS                 := -lm

RELEASE_OPTIMIZATION := -O3 -flto
VERSION_ARCH         := $(shell uname -m)

STATIC_LIBS := -static -lm
BIN_SRCS    := $(shell find bin -type f -name '*.c') vendor/getline/getline.c
BIN_HDRS    := $(shell find bin -type f -name '*.h')
BINLIB_NUJS := $(shell find binlib -type f -name '*.nuj' | sort)
BINLIB_NOBS := $(BINLIB_NUJS:.nuj=.no)

ifeq ($(OS),Windows_NT)
	NUJEL := ./nujel.exe
	ASSET := ./tools/assets.exe
	LIBS += -lpthread
	LDFLAGS := -Wl,--stack,16777216
endif

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	STATIC_LIBS := -lm
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
	@rm -f -- nujel nujel.exe nujel-bootstrap nujel-bootstrap.exe nujel.a nujel.wa nujel.com nujel.com.dbg tools/assets tools/assets.exe
	@rm -f -- $(shell find bin lib vendor bootstrap -type f -name '*.o')
	@rm -f -- $(shell find bin lib vendor bootstrap -type f -name '*.wo')
	@rm -f -- $(shell find bin lib vendor bootstrap -type f -name '*.d')
	@rm -f -- $(shell find bin lib vendor bootstrap -type f -name '*.wd')
	@rm -f -- $(shell find bin lib vendor bootstrap -type f -name '*.deps')
	@rm -f -- $(shell find binlib stdlib -type f -name '*.no')
	@rm -f ./callgrind.out.*
	@rm -f ./web/index.html ./web/index.js ./web/index.wasm
	@rm -f ./bootstrap/*.h ./bootstrap/*.c
	@rm -rf tmp
	@echo "$(ANSI_BG_RED)" "[CLEAN]" "$(ANSI_RESET)" "nujel"

.PHONY: distclean
distclean:
	@rm -rf tools/emsdk

%.o: %.c
	@$(CC) -o $@ -c $< $(CFLAGS) $(CINCLUDES) $(OPTIMIZATION) $(WARNINGS) $(CSTD) -MD > ${<:.c=.d}
	@echo "$(ANSI_GREEN)" "[CC] " "$(ANSI_RESET)" $@

%.wo: %.c
	@$(EMCC) -o $@ -c $< $(CFLAGS) $(CINCLUDES) $(OPTIMIZATION) $(WARNINGS) $(CSTD) -MD > ${<:.c=.wd}
	@echo "$(ANSI_GREEN)" "[WCC]" "$(ANSI_RESET)" $@

nujel.wa: $(LIB_WASM_OBJS) tmp/stdlib.wo
	@rm -rf $@
	@$(EMAR) cq $@ $^
	@echo "$(ANSI_BG_CYAN)" "[WAR]" "$(ANSI_RESET)" $@

$(ASSET): tools/assets.c
	@$(CC) -o $@ $^ $(CFLAGS) $(CINCLUDES) $(OPTIMIZATION) $(WARNINGS) $(CSTD) $(LIBS)
	@echo "$(ANSI_BG_GREY)" "[CC] " "$(ANSI_RESET)" $@

nujel.a: $(LIB_OBJS)
	@rm -rf $@
	@$(AR) cq $@ $^
	@echo "$(ANSI_BG_CYAN)" "[AR] " "$(ANSI_RESET)" $@

$(NUJEL): $(BIN_OBJS) $(LIB_OBJS) tmp/stdlib.o tmp/binlib.o
	@$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS) $(CINCLUDES) $(OPTIMIZATION) $(WARNINGS) $(CSTD) $(LIBS)
	@echo "$(ANSI_BG_GREEN)" "[CC] " "$(ANSI_RESET)" $@

nujel-bootstrap: $(BIN_OBJS) $(LIB_OBJS) bootstrap/stdlib.o bootstrap/binlib.o
	@$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS) $(CINCLUDES) $(OPTIMIZATION) $(WARNINGS) $(CSTD) $(LIBS)
	@echo "$(ANSI_BG_GREEN)" "[CC] " "$(ANSI_RESET)" $@
	@$(NUJEL_BOOT) -x "[exit [test-run]]"

release: $(BIN_SRCS) $(LIB_SRCS) tmp/stdlib.c tmp/binlib.c
	@rm -f $(NUJEL)
	@$(CC) -o $(NUJEL) $^ $(CFLAGS) $(CINCLUDES) $(RELEASE_OPTIMIZATION) $(CSTD) $(STATIC_LIBS)
	@$(NUJEL) -x "[exit [test-run]]"
	@echo "$(ANSI_BG_GREEN)" "[CC] " "$(ANSI_RESET)" $(NUJEL)

release.musl: $(BIN_SRCS) $(LIB_SRCS) tmp/stdlib.c tmp/binlib.c
	@rm -f $(NUJEL)
	@$(CC_MUSL) -s -static -o $(NUJEL) $^ $(CFLAGS) $(CINCLUDES) $(RELEASE_OPTIMIZATION) $(CSTD) $(LIBS)
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

tmp/stdlib.no: $(STDLIB_NUJS) $(BINLIB_NUJS) $(NUJEL_BOOT)
	@mkdir -p tmp/
	@$(NUJEL_BOOT) tools/bootstrap.nuj
	@cat $(STDLIB_NOBS) > tmp/stdlib.no
	@cat $(BINLIB_NOBS) > tmp/binlib.no
	@echo "$(ANSI_GREEN)" "[CAT]" "$(ANSI_RESET)" tmp/stdlib.no
	@echo "$(ANSI_GREEN)" "[CAT]" "$(ANSI_RESET)" tmp/binlib.no

tmp/binlib.no: tmp/stdlib.no
	@true

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
	@$(NUJEL) tools/tests.nuj

.PHONY: check
check: run

.PHONY: test.slow
test.slow: $(NUJEL)
	@$(NUJEL) --slow-test tools/tests.nuj

.PHONY: test.ridiculous
test.ridiculous: $(NUJEL)
	@$(NUJEL) --slow-test --ridiculous-test tools/tests.nuj

.PHONY: run
run: $(NUJEL)
	@$(NUJEL) -x "[exit [test-run]]"

.PHONY: runl
runl: $(NUJEL)
	@$(NUJEL) -x "[exit [test-run-bytecode]]"

.PHONY: rund
rund: $(NUJEL)
	gdb $(NUJEL) -ex "r"

.PHONY: runn
runn: $(NUJEL)
	rlwrap $(NUJEL)

.PHONY: install
install: release
	mkdir -p $(bindir)
	$(INSTALL) $(NUJEL) $(bindir)

.PHONY: install.musl
install.musl: release.musl
	mkdir -p $(bindir)
	$(INSTALL) $(NUJEL) $(bindir)

.PHONY: profile
profile: $(NUJEL)
	valgrind --tool=callgrind --dump-instr=yes $(NUJEL) -x "[test-run]"

.PHONY: profile-while
profile-while: $(NUJEL)
	valgrind --tool=callgrind --dump-instr=yes $(NUJEL) -x "[display [let* [def v 0] [while [< v 2,000,000] [set! v [+ 1 v]]] v]]"

.PHONY: web
web:
	./tools/buildwasm
	rsync -avhe ssh --delete ./web/ wolkenwelten.net:/home/nujel/nujel/

update-bootstrap: tmp/stdlib.no tmp/binlib.no
	cp -f tmp/stdlib.no bootstrap/stdlib.no
	cp -f tmp/binlib.no bootstrap/binlib.no
	sed -i 's/\[def test-context \"Nujel Standalone\"\]/\[def test-context \"Nujel Bootstrap\"\]/g' bootstrap/binlib.no
