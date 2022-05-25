clean:
	@rm -f -- nujel nujel.exe nujel-bootstrap nujel-bootstrap.exe nujel.a nujel.wa nujel.com nujel.com.dbg tools/assets tools/assets.exe DOSNUJEL.EXE
	@rm -f -- $(FILES_TO_CLEAN)
	@rm -f -- $(NOBS_TO_CLEAN)
	@rm -f ./callgrind.out.*
	@rm -f ./web/index.html ./web/index.js ./web/index.wasm
	@rm -f ./bootstrap/*.h ./bootstrap/*.c
	@rm -rf tmp
	@echo "$(ANSI_BG_RED)" "[CLEAN]" "$(ANSI_RESET)" "nujel"
distclean:
	@rm -rf tools/emsdk

DOSNUJEL.EXE: $(NUJEL) tools/watcom.nuj
	@source /opt/watcom/owsetenv.sh && ./$(NUJEL) tools/watcom.nuj

release.wasm: web/index.html

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

run: $(NUJEL)
	@./$(NUJEL) --only-test-suite tools/tests.nuj

test: $(NUJEL)
	@./$(NUJEL) tools/tests.nuj

test.v: $(NUJEL)
	@./$(NUJEL) -v tools/tests.nuj

test.verbose: $(NUJEL)
	@./$(NUJEL) --verbose --only-test-suite tools/tests.nuj

test.bootstrap: $(NUJEL_BOOTSTRAP)
	@./$(NUJEL_BOOTSTRAP) --only-test-suite tools/tests.nuj

test.slow: $(NUJEL)
	@./$(NUJEL) --slow-test tools/tests.nuj

test.debug: $(NUJEL)
	@gdb ./$(NUJEL) -ex "r tools/tests.nuj"

test.slow.debug: $(NUJEL)
	@gdb ./$(NUJEL) -ex "r --slow-test tools/tests.nuj"

test.ridiculous: $(NUJEL)
	@./$(NUJEL) --slow-test --ridiculous-test tools/tests.nuj

rund: $(NUJEL)
	@gdb $(NUJEL) -ex "r"

runn: $(NUJEL)
	@rlwrap $(NUJEL)

install: release
	mkdir -p $(bindir)
	$(INSTALL) $(NUJEL) $(bindir)

install.musl: release.musl
	mkdir -p $(bindir)
	$(INSTALL) $(NUJEL) $(bindir)

profile: $(NUJEL)
	valgrind --tool=callgrind --dump-instr=yes $(NUJEL) --only-test-suite tools/tests.nuj

web:
	./tools/buildwasm
	rsync -avhe ssh --delete ./web/ wolkenwelten.net:/home/nujel/nujel/

benchmark: release
	cp -f $(NUJEL) ~/bin/
	./$(NUJEL) ./tools/benchmark.nuj && ./tools/benchmark-sync.nuj

benchmark-nujel: release
	cp -f $(NUJEL) ~/bin/
	./$(NUJEL) --no-overwrite --only-nujel ./tools/benchmark.nuj && ./tools/benchmark-sync.nuj

update-bootstrap: tmp/stdlib.no tmp/binlib.no
	cp -f tmp/stdlib.no bootstrap/stdlib.no
	cp -f tmp/binlib.no bootstrap/binlib.no
	sed -i 's/\[def test-context \"Nujel Standalone\"\]/\[def test-context \"Nujel Bootstrap\"\]/g' bootstrap/binlib.no
