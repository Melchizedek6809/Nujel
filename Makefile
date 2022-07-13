.include "mk/ansi_colors.mk"
.include "mk/common.mk"

LIB_SRCS             != find lib -type f -name '*.c'
LIB_OBJS             := $(LIB_SRCS:.c=.o)
STDLIB_NUJS          != find stdlib -type f -name '*.nuj' | sort
STDLIB_MODS          != find stdlib_modules -type f -name '*.nuj' | sort
STDLIB_NOBS          := $(STDLIB_NUJS:.nuj=.no)
STDLIB_MOBS          := $(STDLIB_MODS:.nuj=.no)

BIN_SRCS             != find bin vendor -type f -name '*.c'
BINLIB_NUJS          != find binlib -type f -name '*.nuj' | sort
BINLIB_NOBS          := $(BINLIB_NUJS:.nuj=.no)

BIN_OBJS             := $(BIN_SRCS:.c=.o)

all: $(NUJEL)
.PHONY: clean distclean all release release.musl test check test.slow test.ridiculous run runl rnd runn install profile profile-when

FILES_TO_CLEAN != find bin lib vendor bootstrap binlib stdlib -type f -name '*.o' -o -name '*.wo' -o -name '*.obj' -o -name '*.d' -o -name '*.wd' -o -name '*.deps'
NOBS_TO_CLEAN  != find binlib stdlib stdlib_modules -type f -name '*.no'

.SUFFIXES:
.SUFFIXES: .c .o
.c.o:
	@$(CC) -o $@ -c $< $(CFLAGS) $(CINCLUDES) $(OPTIMIZATION) $(WARNINGS) $(CSTD)
	@echo "$(ANSI_GREEN)" "[CC] " "$(ANSI_RESET)" $@

$(BIN_OBJS): nujel.h lib/nujel-private.h bin/private.h
$(LIB_OBJS): nujel.h lib/nujel-private.h

.SUFFIXES: .nuj .no
.nuj.no: $(NUJEL_BOOTSTRAP)
	@./$(NUJEL_BOOTSTRAP) -x "[file/compile/argv]" $<
	@echo "$(ANSI_GREEN)" "[NUJ]" "$(ANSI_RESET)" $@

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
	@$(CC) -o $(NUJEL) $> $(CFLAGS) $(CINCLUDES) $(RELEASE_OPTIMIZATION) $(CSTD) $(LIBS)
	@$(STRIP) -xS $(NUJEL)
	@echo "$(ANSI_BG_GREEN)" "[CC] " "$(ANSI_RESET)" $(NUJEL)

.include "mk/targets.mk"
