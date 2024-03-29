# BSD Make
.include "ansi_colors.mk"
.include "common.mk"

LIB_SRCS             != find lib -type f -name '*.c'
LIB_OBJS             := $(LIB_SRCS:.c=.o)
STDLIB_NUJS          != find stdlib -type f -name '*.nuj' | sort
STDLIB_MODS          != find stdlib_modules -type f -name '*.nuj' | sort
STDLIB_NOBS          := $(STDLIB_NUJS:.nuj=.no)
STDLIB_MOBS          := $(STDLIB_MODS:.nuj=.no)

BIN_SRCS             != find bin -type f -name '*.c'
BINLIB_NUJS          != find binlib -type f -name '*.nuj' | sort
BINLIB_NOBS          := $(BINLIB_NUJS:.nuj=.no)

BIN_OBJS             := $(BIN_SRCS:.c=.o)

FUTURE_SRCS = $(BIN_SRCS) $(LIB_SRCS) tmp/image.c
FUTURE_OBJS = $(BIN_OBJS) $(LIB_OBJS) tmp/image.o

RUNTIME_SRCS = $(BIN_SRCS) $(LIB_SRCS) bootstrap/image.c
RUNTIME_OBJS = $(BIN_OBJS) $(LIB_OBJS) bootstrap/image.o

all: $(NUJEL)
.PHONY: clean distclean all release release.musl test check test.slow test.ridiculous run runl rnd runn install profile profile-when

FILES_TO_CLEAN != find bin lib bootstrap binlib stdlib -type f -name '*.o' -o -name '*.wo' -o -name '*.obj' -o -name '*.d' -o -name '*.wd' -o -name '*.deps'
NOBS_TO_CLEAN  != find binlib stdlib stdlib_modules -type f -name '*.no'

.for LIB_OBJ in $(LIB_OBJS)
$(LIB_OBJ): lib/nujel.h lib/nujel-private.h
.endfor

.for BIN_OBJ in $(BIN_OBJS)
$(BIN_OBJ): lib/nujel.h lib/nujel-private.h bin/private.h
.endfor

.SUFFIXES:
.SUFFIXES: .c .o
.c.o:
	@$(CC) -o $@ -c $< $(CFLAGS) $(CINCLUDES) $(OPTIMIZATION) $(WARNINGS) $(CSTD)
	@echo "$(ANSI_GREEN)" "[CC] " "$(ANSI_RESET)" $@

nujel.a: $(LIB_OBJS)
	@rm -rf $@
	$(AR) cq $@ $>
	@echo "$(ANSI_BG_CYAN)" "[AR] " "$(ANSI_RESET)" $@

future-nujel: $(FUTURE_OBJS)
	@rm -f $@
	@$(CC) -o $@ $> $(LDFLAGS) $(CFLAGS) $(CINCLUDES) $(OPTIMIZATION) $(WARNINGS) $(CSTD) $(LIBS)
	@echo "$(ANSI_BG_GREEN)" "[CC] " "$(ANSI_RESET)" $@

nujel: $(RUNTIME_OBJS)
	@rm -f $@
	@$(CC) -o $@ $> $(LDFLAGS) $(CFLAGS) $(CINCLUDES) $(OPTIMIZATION) $(WARNINGS) $(CSTD) $(LIBS)
	@echo "$(ANSI_BG_GREEN)" "[CC] " "$(ANSI_RESET)" $@

release: $(RUNTIME_SRCS)
	@rm -f $(NUJEL)
	@$(CC) -o $(NUJEL) $> $(CFLAGS) $(CINCLUDES) $(RELEASE_OPTIMIZATION) $(CSTD) $(LIBS) $(LDFLAGS)
	@$(STRIP) -xS $(NUJEL)
	@echo "$(ANSI_BG_GREEN)" "[CC] " "$(ANSI_RESET)" $(NUJEL)

nujel.h: mk/amalgamation/prefix.h lib/nujel.h lib/nujel-private.h mk/amalgamation/implementation-prefix.h $(LIB_SRCS) bootstrap/stdlib.c mk/amalgamation/implementation-suffix.h mk/amalgamation/suffix.h
	$(CAT) $> > nujel.h

nujel.c: mk/amalgamation/bin-prefix.h mk/amalgamation/prefix.h lib/nujel.h lib/nujel-private.h bin/private.h mk/amalgamation/implementation-prefix.h $(LIB_SRCS) $(BIN_SRCS) $(VENDOR_SRCS) bootstrap/stdlib.c bootstrap/binlib.c mk/amalgamation/implementation-suffix.h mk/amalgamation/suffix.h
	$(CAT) $> > nujel.c

release.amalgamation: nujel.c
	@$(CC) -o $(NUJEL) nujel.c $(CFLAGS) $(CINCLUDES) $(RELEASE_OPTIMIZATION) $(CSTD) $(LIBS)
	@$(STRIP) -xS $(NUJEL)
	@echo "$(ANSI_BG_GREEN)" "[CC] " "$(ANSI_RESET)" $(NUJEL)

.include "targets.mk"
