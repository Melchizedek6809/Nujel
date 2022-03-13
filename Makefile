.include "mk/ansi_colors.mk"
.include "mk/common.mk"

LIB_SRCS             != find lib -type f -name '*.c'
LIB_HDRS             != find lib -type f -name '*.h'
LIB_OBJS             := $(LIB_SRCS:.c=.o)
LIB_DEPS             := ${LIB_SRCS:.c=.d}
STDLIB_NUJS          != find stdlib -type f -name '*.nuj' | sort
STDLIB_NOBS          := $(STDLIB_NUJS:.nuj=.no)

BIN_SRCS             != find bin vendor -type f -name '*.c'
BIN_HDRS             != find bin vendor -type f -name '*.h'
BINLIB_NUJS          != find binlib -type f -name '*.nuj' | sort
BINLIB_NOBS          := $(BINLIB_NUJS:.nuj=.no)

BIN_OBJS             := $(BIN_SRCS:.c=.o)
BIN_DEPS             := $(BIN_SRCS:.c=.d)

all: $(NUJEL)
.PHONY: clean distclean all release release.musl test check test.slow test.ridiculous run runl rnd runn install profile profile-when

.if "${MAKECMDGOALS}" != "clean"
-include $(LIB_DEPS)
-include $(BIN_DEPS)
.endif

FILES_TO_CLEAN != find bin lib vendor bootstrap binlib stdlib -type f -name '*.o' -o -name '*.wo' -o -name '*.obj' -o -name '*.d' -o -name '*.wd' -o -name '*.deps'
NOBS_TO_CLEAN  != find binlib stdlib -type f -name '*.no'

clean:
	@rm -f -- nujel nujel.exe nujel-bootstrap nujel-bootstrap.exe nujel.a nujel.wa nujel.com nujel.com.dbg tools/assets tools/assets.exe
	@rm -f -- $(FILES_TO_CLEAN)
	@rm -f -- $(NOBS_TO_CLEAN)
	@rm -f ./callgrind.out.*
	@rm -f ./web/index.html ./web/index.js ./web/index.wasm
	@rm -f ./bootstrap/*.h ./bootstrap/*.c
	@rm -rf tmp
	@echo "$(ANSI_BG_RED)" "[CLEAN]" "$(ANSI_RESET)" "nujel"
distclean: clean

.SUFFIXES:
.SUFFIXES: .c .o
.c.o:
	@$(CC) -o $@ -c $< $(CFLAGS) $(CINCLUDES) $(OPTIMIZATION) $(WARNINGS) $(CSTD) -MD > ${<:.c=.d}
	@echo "$(ANSI_GREEN)" "[CC] " "$(ANSI_RESET)" $@

tools/assets: tools/assets.c
	@$(CC) -o $@ $> $(CFLAGS) $(CINCLUDES) $(OPTIMIZATION) $(WARNINGS) $(CSTD) $(LIBS)
	@echo "$(ANSI_BG_GREY)" "[CC] " "$(ANSI_RESET)" $@

nujel.a: $(LIB_OBJS)
	@rm -rf $@
	$(AR) cq $@ $>
	@echo "$(ANSI_BG_CYAN)" "[AR] " "$(ANSI_RESET)" $@

nujel: $(BIN_OBJS) $(LIB_OBJS) tmp/stdlib.o tmp/binlib.o
	@rm -f $@
	@$(CC) -o $@ $> $(LDFLAGS) $(CFLAGS) $(CINCLUDES) $(OPTIMIZATION) $(WARNINGS) $(CSTD) $(LIBS)
	@echo "$(ANSI_BG_GREEN)" "[CC] " "$(ANSI_RESET)" $@

nujel-bootstrap: $(BIN_OBJS) $(LIB_OBJS) bootstrap/stdlib.o bootstrap/binlib.o
	@rm -f $@
	@$(CC) -o $@ $> $(LDFLAGS) $(CFLAGS) $(CINCLUDES) $(OPTIMIZATION) $(WARNINGS) $(CSTD) $(LIBS)
	@echo "$(ANSI_BG_GREEN)" "[CC] " "$(ANSI_RESET)" $@

release: $(BIN_SRCS) $(LIB_SRCS) tmp/stdlib.c tmp/binlib.c
	@rm -f $(NUJEL)
	@$(CC) -o $(NUJEL) $> $(CFLAGS) $(CINCLUDES) $(RELEASE_OPTIMIZATION) $(CSTD) $(STATIC_LIBS)
	@echo "$(ANSI_BG_GREEN)" "[CC] " "$(ANSI_RESET)" $(NUJEL)

DOSNUJEL.EXE: nujel tools/watcom.nuj
	@source /opt/watcom/owsetenv.sh && ./$(NUJEL) tools/watcom.nuj

bootstrap/stdlib.c: bootstrap/stdlib.no $(ASSET)
	@./$(ASSET) bootstrap/stdlib bootstrap/stdlib.no
	@echo "$(ANSI_GREY)" "[ST] " "$(ANSI_RESET)" $@

bootstrap/binlib.c: bootstrap/binlib.no $(ASSET)
	@./$(ASSET) bootstrap/binlib bootstrap/binlib.no
	@echo "$(ANSI_GREY)" "[ST] " "$(ANSI_RESET)" $@

tmp/stdlib.no: $(STDLIB_NUJS) $(BINLIB_NUJS) $(NUJEL_BOOTSTRAP)
	@mkdir -p tmp/
	@./$(NUJEL_BOOTSTRAP) tools/bootstrap.nuj
	@cat $(STDLIB_NOBS) > tmp/stdlib.no
	@cat $(BINLIB_NOBS) > tmp/binlib.no
	@echo "$(ANSI_GREEN)" "[CAT]" "$(ANSI_RESET)" tmp/stdlib.no
	@echo "$(ANSI_GREEN)" "[CAT]" "$(ANSI_RESET)" tmp/binlib.no

tmp/binlib.no: tmp/stdlib.no
	@true

tmp/stdlib.c: tmp/stdlib.no $(ASSET)
	@mkdir -p tmp/
	@./$(ASSET) tmp/stdlib tmp/stdlib.no
	@echo "$(ANSI_GREY)" "[ST] " "$(ANSI_RESET)" $@
tmp/stdlib.h: tmp/stdlib.c
	@true

tmp/binlib.c: tmp/binlib.no $(ASSET)
	@mkdir -p tmp/
	@./$(ASSET) tmp/binlib tmp/binlib.no
	@echo "$(ANSI_GREY)" "[ST] " "$(ANSI_RESET)" $@
tmp/binlib.h: tmp/binlib.c
	@true

.PHONY: test.bootstrap
test.bootstrap: $(NUJEL_BOOTSTRAP)
	@./$(NUJEL_BOOTSTRAP) --only-test-suite tools/tests.nuj

test: $(NUJEL)
	@./$(NUJEL) tools/tests.nuj
check: test

test.slow: $(NUJEL)
	@./$(NUJEL) --slow-test tools/tests.nuj

test.ridiculous: $(NUJEL)
	@./$(NUJEL) --slow-test --ridiculous-test tools/tests.nuj

run: $(NUJEL)
	@./$(NUJEL) --only-test-suite tools/tests.nuj

runl: $(NUJEL)
	@./$(NUJEL) --only-test-suite --bytecoded tools/tests.nuj

rund: $(NUJEL)
	gdb $(NUJEL) -ex "r"

runn: $(NUJEL)
	rlwrap $(NUJEL)

install: release
	mkdir -p $(bindir)
	$(INSTALL) $(NUJEL) $(bindir)

update-bootstrap: tmp/stdlib.no tmp/binlib.no
	cp -f tmp/stdlib.no bootstrap/stdlib.no
	cp -f tmp/binlib.no bootstrap/binlib.no
	sed -i 's/\[def test-context \"Nujel Standalone\"\]/\[def test-context \"Nujel Bootstrap\"\]/g' bootstrap/binlib.no
