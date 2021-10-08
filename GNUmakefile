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
STDLIB_NOBS := $(STDLIB_NUJS:.nuj=.no)

LIB_WASM_OBJS := $(LIB_SRCS:.c=.wo)
LIB_WASM_DEPS := ${LIB_SRCS:.c=.wd}

BIN_SRCS    := $(shell find bin -type f -name '*.c')
BIN_HDRS    := $(shell find bin -type f -name '*.h')
BINLIB_NUJS := $(shell find bin/lib -type f -name '*.nuj')
BINLIB_NOBS := $(BINLIB_NUJS:.nuj=.no)

NUJEL       := ./nujel
ASSET       := ./tools/assets

CC                   := cc
CFLAGS               := -g -D_GNU_SOURCE
CSTD                 := -std=c99
OPTIMIZATION         := -O2
WARNINGS             := -Wall -Werror -Wextra -Wshadow -Wcast-align -Wno-missing-braces

LIBS                 := -lm

RELEASE_OPTIMIZATION := -O3
VERSION_ARCH         := $(shell uname -m)

ifeq ($(OS),Windows_NT)
	NUJEL := ./nujel.exe
	ASSET := ./tools/assets.exe
	LIBS  += -lpthread
else
	BIN_SRCS += vendor/bestline/bestline.c
endif
BIN_OBJS    := $(BIN_SRCS:.c=.o)
BIN_DEPS    := ${BIN_SRCS:.c=.d}

all: $(NUJEL)
.PHONY: all release .deps

include mk/ansi_colors.mk
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
	@rm -f nujel nujel.exe nujel.a nujel.wa tools/assets tools/assets.exe $(shell find bin lib -type f -name '*.o') $(shell find bin lib -type f -name '*.wo') $(shell find bin lib -type f -name '*.d') $(shell find bin lib -type f -name '*.wd')
	@rm -rf tmp
	@echo "$(ANSI_BG_RED)" "[CLEAN]" "$(ANSI_RESET)" "nujel"

%.o: %.c
	@$(CC) -o $@ -c $< $(CFLAGS) $(CINCLUDES) $(OPTIMIZATION) $(WARNINGS) $(CSTD) -MMD > ${<:.c=.d}
	@echo "$(ANSI_GREEN)" "[CC] " "$(ANSI_RESET)" $@

%.no: %.nuj
	@$(NUJEL) -x "[file/compile \"$^\"]"
	@echo "$(ANSI_PINK)" "[NUJ]" "$(ANSI_RESET)" $@

%.wo: %.c
	$(EMCC) -o $@ -c $< $(CFLAGS) $(CINCLUDES) $(OPTIMIZATION) $(WARNINGS) $(CSTD) -MMD > ${<:.c=.wd}
	@echo "$(ANSI_GREEN)" "[CC] " "$(ANSI_RESET)" $@

nujel.wa: $(LIB_WASM_OBJS) tmp/stdlib.wo
	@rm -rf $@
	@$(EMAR) cq $@ $^
	@echo "$(ANSI_BG_CYAN)" "[AR] " "$(ANSI_RESET)" $@

$(ASSET): tools/assets.c
	@$(CC) -o $@    $^ $(CFLAGS) $(CINCLUDES) $(OPTIMIZATION) $(WARNINGS) $(CSTD) $(LIBS)
	@echo "$(ANSI_BG_GREY)" "[CC] " "$(ANSI_RESET)" $@

nujel.a: $(LIB_OBJS) tmp/stdlib.o
	@rm -rf $@
	@ar cq $@ $^
	@echo "$(ANSI_BG_CYAN)" "[AR] " "$(ANSI_RESET)" $@

$(NUJEL): $(BIN_OBJS) tmp/binlib.o nujel.a
	@$(CC) -o $@ $^ $(CFLAGS) $(CINCLUDES) $(OPTIMIZATION) $(WARNINGS) $(CSTD) $(LIBS)
	@echo "$(ANSI_BG_GREEN)" "[CC] " "$(ANSI_RESET)" $@

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

tmp/stdlib.c: tmp/stdlib.nuj $(ASSET)
	@mkdir -p tmp/
	@$(ASSET) tmp/stdlib tmp/stdlib.nuj
	@echo "$(ANSI_GREY)" "[ST] " "$(ANSI_RESET)" $@
tmp/stdlib.h: tmp/stdlib.c
	@true

tmp/binlib.c: tmp/binlib.nuj $(ASSET)
	@mkdir -p tmp/
	@$(ASSET) tmp/binlib tmp/binlib.nuj
	@echo "$(ANSI_GREY)" "[ST] " "$(ANSI_RESET)" $@
tmp/binlib.h: tmp/binlib.c
	@true

.PHONY: test
test: $(NUJEL)
	@$(NUJEL) -x "[quit [test-run]]"

.PHONY: run
run: $(NUJEL)
	@$(NUJEL) -x "[quit [test-run]]"

.PHONY: rund
rund: $(NUJEL)
	gdb $(NUJEL) -ex "r"

.PHONY: runn
runn: $(NUJEL)
	$(NUJEL)
