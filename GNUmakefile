include mk/ansi_colors.mk
include mk/common.mk
include mk/disable_implicit_rules.mk

LIB_SRCS      := $(shell find lib -type f -name '*.c')
LIB_HDRS      := $(shell find lib -type f -name '*.h')
LIB_OBJS      := $(LIB_SRCS:.c=.o)
LIB_DEPS      := ${LIB_SRCS:.c=.d}
STDLIB_NUJS   := $(shell find stdlib -type f -name '*.nuj' | sort)
STDLIB_NOBS   := $(STDLIB_NUJS:.nuj=.no)

LIB_WASM_OBJS := $(LIB_SRCS:.c=.wo)
LIB_WASM_DEPS := ${LIB_SRCS:.c=.wd}

ifeq (, $(shell which $(CC)))
CC                   := gcc
endif
ifeq (, $(shell which $(CC)))
CC                   := clang
endif
ifeq (, $(shell which $(CC)))
CC                   := tcc
endif

BIN_SRCS    := $(shell find bin -type f -name '*.c') vendor/getline/getline.c
BIN_HDRS    := $(shell find bin -type f -name '*.h')
BINLIB_NUJS := $(shell find binlib -type f -name '*.nuj' | sort)
BINLIB_NOBS := $(BINLIB_NUJS:.nuj=.no)

ifeq ($(OS),Windows_NT)
	NUJEL   := nujel.exe
	ASSET   := tools/assets.exe
	LIBS    += -lpthread
	STATIC_LIBS := -static -lpthread
	LDFLAGS := -Wl,--stack,16777216
endif

ifeq ($(shell uname -s),Darwin)
	STATIC_LIBS := -lm
endif

BIN_OBJS      := $(BIN_SRCS:.c=.o)
BIN_DEPS      := ${BIN_SRCS:.c=.d}

BIN_WASM_OBJS := $(BIN_SRCS:.c=.wo)
BIN_WASM_DEPS := $(BIN_SRCS:.c=.wd)

all: $(NUJEL)
.PHONY: all release release.musl
.PHONY: rund runn install install.musl profile web
.PHONY: test.bootstrap check test test.verbose test.debug test.slow test.slow.debug test.ridiculous

ifneq ($(MAKECMDGOALS),clean)
-include $(LIB_DEPS)
-include $(BIN_DEPS)
-include $(LIB_WASM_DEPS)
-include $(BIN_WASM_DEPS)
endif

ifdef EMSDK
all: nujel.wa
endif

FILES_TO_CLEAN := $(shell find bin lib vendor bootstrap binlib stdlib -type f -name '*.o' -o -name '*.wo' -o -name '*.obj' -o -name '*.d' -o -name '*.wd' -o -name '*.deps')
NOBS_TO_CLEAN  := $(shell find binlib stdlib -type f -name '*.no')

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

release: $(BIN_SRCS) $(LIB_SRCS) tmp/stdlib.c tmp/binlib.c
	@rm -f $(NUJEL)
	@$(CC) -o $(NUJEL) $^ $(CFLAGS) $(CINCLUDES) $(RELEASE_OPTIMIZATION) $(CSTD) $(LIBS)
	@$(STRIP) -xS $(NUJEL)
	@echo "$(ANSI_BG_GREEN)" "[CC] " "$(ANSI_RESET)" $(NUJEL)

release.musl: $(BIN_SRCS) $(LIB_SRCS) tmp/stdlib.c tmp/binlib.c
	@rm -f $(NUJEL)
	@musl-gcc -s -static -o $(NUJEL) $^ $(CFLAGS) $(CINCLUDES) $(RELEASE_OPTIMIZATION) $(CSTD) $(LIBS)
	@$(STRIP) -xS $(NUJEL)
	@echo "$(ANSI_BG_GREEN)" "[CC] " "$(ANSI_RESET)" $(NUJEL)

release.san: $(BIN_SRCS) $(LIB_SRCS) tmp/stdlib.c tmp/binlib.c
	@rm -f $(NUJEL)
	$(CC) -fsanitize=address -fsanitize=undefined -fsanitize-undefined-trap-on-error -g  -Og -fno-lto -o $(NUJEL) $^ $(CFLAGS) $(CINCLUDES) $(CSTD) $(LIBS)
	@echo "$(ANSI_BG_GREEN)" "[CC] " "$(ANSI_RESET)" $(NUJEL)

bootstrap.san: $(BIN_SRCS) $(LIB_SRCS) bootstrap/stdlib.c bootstrap/binlib.c
	@rm -f $(NUJEL)
	$(CC) -fsanitize=address -fsanitize=undefined -fsanitize-undefined-trap-on-error -g  -Og -fno-lto -o nujel-bootstrap $^ $(CFLAGS) $(CINCLUDES) $(CSTD) $(LIBS)

web/index.html: nujel.wa $(BIN_WASM_OBJS) tmp/binlib.wo
	@mkdir -p releases/wasm/
	$(EMCC) $^ -D_GNU_SOURCE $(CSTD) -O3 -s EXPORTED_FUNCTIONS="['_main','_run']" -s EXPORTED_RUNTIME_METHODS=["ccall","cwrap"] -fno-rtti --closure 0 $(EMMEM) --shell-file web/shell.html -o $@

include mk/targets.mk
