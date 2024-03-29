clean:
	@rm -f -- nujel nujel.exe nujel.wasm nujel-bootstrap nujel-bootstrap.exe future-nujel future-nujel.exe nujel.c nujel.h nujel.a nujel.wa nujel.com nujel.com.dbg tools/assets tools/assets.exe DOSNUJEL.EXE
	@rm -f -- $(FILES_TO_CLEAN)
	@rm -f -- $(NOBS_TO_CLEAN)
	@rm -f ./callgrind.out.*
	@rm -f ./web/index.html ./web/index.js ./web/index.wasm
	@rm -rf tmp
	@echo "$(ANSI_BG_RED)" "[CLEAN]" "$(ANSI_RESET)" "nujel"
distclean:
	@rm -rf tools/emsdk

DOSNUJEL.EXE: $(NUJEL) tools/watcom.nuj
	@source /opt/watcom/owsetenv.sh && ./$(NUJEL) tools/watcom.nuj

tmp/image.c: tmp/init.nuji $(NUJEL)
	@./$(NUJEL) ./tools/c-asset-packer.nuj "./tmp/init.nuji" "./tmp/image.c" "bootstrap_image"
	@echo "$(ANSI_GREY)" "[ST] " "$(ANSI_RESET)" $@

tmp/init.nuji: $(NUJEL) $(STDLIB_NUJS) $(STDLIB_MODS) $(BINLIB_NUJS)
	@mkdir -p tmp/
	@./$(NUJEL) tools/build-image.nuj
	@echo "$(ANSI_GREEN)" "[IMG]" "$(ANSI_RESET)" $@

runi: $(NUJEL) tmp/init.nuji
	@./$(NUJEL) --base-image "tmp/init.nuji"

run: $(FUTURE_NUJEL)
	@./$(FUTURE_NUJEL) tools/tests.nuj

test: $(NUJEL)
	@./$(NUJEL) tools/tests.nuj

test.img: $(NUJEL) tmp/init.nuji
	@./$(NUJEL) --base-image tmp/init.nuji tools/tests.nuj

test.v: $(NUJEL)
	@./$(NUJEL) -v tools/tests.nuj

test.verbose: $(NUJEL)
	@./$(NUJEL) --verbose --only-test-suite tools/tests.nuj

test.future: $(FUTURE_NUJEL)
	@./$(FUTURE_NUJEL) tools/tests.nuj

test.future.slow: $(FUTURE_NUJEL)
	@./$(FUTURE_NUJEL) --slow-test tools/tests.nuj

test.slow: $(NUJEL)
	@./$(NUJEL) --slow-test tools/tests.nuj

test.debug: $(NUJEL)
	@gdb ./$(NUJEL) -ex "r tools/tests.nuj"

test.slow.debug: $(NUJEL)
	@gdb ./$(NUJEL) -ex "r --slow-test tools/tests.nuj"

rund: $(NUJEL)
	@gdb ./$(NUJEL) -ex "r"

runn: $(FUTURE_NUJEL)
	@rlwrap ./$(FUTURE_NUJEL)

run.wasm: nujel.wasm
	@rlwrap wasmtime --dir=. nujel.wasm

test.wasm: nujel.wasm
	@wasmtime --dir=. nujel.wasm tools/tests.nuj

install: release
	mkdir -p $(bindir)
	$(INSTALL) ./$(NUJEL) $(bindir)

install.musl: release.musl
	mkdir -p $(bindir)
	$(INSTALL) ./$(NUJEL) $(bindir)

release.wasm: nujel.wasm
	@$(WASI_STRIP) nujel.wasm

$(FUZZ_NUJEL): $(RUNTIME_SRCS)
	@$(AFL_CC) -o $@ $^ $(LDFLAGS) $(CFLAGS) $(CINCLUDES) $(OPTIMIZATION) $(WARNINGS) $(CSTD) $(LIBS)

fuzz: $(FUZZ_NUJEL)
	@$(AFL_FUZZ) -i tests/fuzz/ -o /tmp/wwfuzz -m 128 -t 10000 -d -- ./$(FUZZ_NUJEL) @@

valgrind: $(NUJEL)
	valgrind --error-exitcode=1 ./$(NUJEL) tools/tests.nuj

profile: $(NUJEL)
	valgrind --tool=callgrind --dump-instr=yes --cache-sim=yes --branch-sim=yes ./$(NUJEL) --only-test-suite tools/tests.nuj

web:
	./tools/buildwasm
	rsync -avhe ssh --delete ./web/ wolkenwelten.net:/home/nujel/nujel/

benchmark: release
	cp -f $(NUJEL) ~/bin/
	./$(NUJEL) ./tools/benchmark.nuj && ./tools/benchmark-sync.nuj

benchmark-nujel: release
	cp -f $(NUJEL) ~/bin/
	./$(NUJEL) --no-overwrite --only-nujel ./tools/benchmark.nuj && ./tools/benchmark-sync.nuj

update-stdlib: tmp/image.c
	cp -f tmp/image.c bootstrap/image.c

show-section-size: $(NUJEL)
	nm --print-size --size-sort --radix=d -l ./nujel | awk '{ if ($$3 != "B") { print } }'
