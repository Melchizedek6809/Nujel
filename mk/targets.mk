clean:
	@rm -f -- nujel nujel.exe nujel-bootstrap nujel-bootstrap.exe nujel.a nujel.wa nujel.com nujel.com.dbg tools/assets tools/assets.exe DOSNUJEL.EXE
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

release.wasm: web/index.html

tmp/stdlib.no: $(STDLIB_NOBS) $(STDLIB_MOBS)
	@mkdir -p tmp/
	@cat $(STDLIB_NOBS) $(STDLIB_MOBS) > tmp/stdlib.no
	@echo "$(ANSI_GREEN)" "[CAT]" "$(ANSI_RESET)" tmp/stdlib.no

tmp/binlib.no: $(BINLIB_NOBS)
	@mkdir -p tmp/
	@cat $(BINLIB_NOBS) > tmp/binlib.no
	@echo "$(ANSI_GREEN)" "[CAT]" "$(ANSI_RESET)" tmp/binlib.no

tmp/stdlib.c: tmp/stdlib.no tools/build-bootstrap.nuj $(NUJEL_BOOTSTRAP)
	@./$(NUJEL_BOOTSTRAP) "tools/build-bootstrap.nuj" -x "[create-c-asset \"./tmp/stdlib.no\" \"./tmp/stdlib.c\" \"stdlib_no_data\"]"
	@echo "$(ANSI_GREY)" "[ST] " "$(ANSI_RESET)" $@

tmp/binlib.c: tmp/binlib.no tools/build-bootstrap.nuj $(NUJEL_BOOTSTRAP)
	@./$(NUJEL_BOOTSTRAP) "tools/build-bootstrap.nuj" -x "[create-c-asset \"./tmp/binlib.no\" \"./tmp/binlib.c\" \"binlib_no_data\"]"
	@echo "$(ANSI_GREY)" "[ST] " "$(ANSI_RESET)" $@

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

test.bootstrap.verbose: $(NUJEL_BOOTSTRAP)
	@./$(NUJEL_BOOTSTRAP) --verbose --only-test-suite tools/tests.nuj

test.slow: $(NUJEL)
	@./$(NUJEL) --slow-test tools/tests.nuj

test.debug: $(NUJEL)
	@gdb ./$(NUJEL) -ex "r tools/tests.nuj"

test.slow.debug: $(NUJEL)
	@gdb ./$(NUJEL) -ex "r --slow-test tools/tests.nuj"

test.ridiculous: $(NUJEL)
	@./$(NUJEL) --slow-test --ridiculous-test tools/tests.nuj

rund: $(NUJEL)
	@gdb ./$(NUJEL) -ex "r"

runn: $(NUJEL)
	@rlwrap ./$(NUJEL)

install: release
	mkdir -p $(bindir)
	$(INSTALL) ./$(NUJEL) $(bindir)

install.musl: release.musl
	mkdir -p $(bindir)
	$(INSTALL) ./$(NUJEL) $(bindir)

profile: $(NUJEL)
	valgrind --tool=callgrind --dump-instr=yes ./$(NUJEL) --only-test-suite tools/tests.nuj

web:
	./tools/buildwasm
	rsync -avhe ssh --delete ./web/ wolkenwelten.net:/home/nujel/nujel/

benchmark: release
	cp -f $(NUJEL) ~/bin/
	./$(NUJEL) ./tools/benchmark.nuj && ./tools/benchmark-sync.nuj

benchmark-nujel: release
	cp -f $(NUJEL) ~/bin/
	./$(NUJEL) --no-overwrite --only-nujel ./tools/benchmark.nuj && ./tools/benchmark-sync.nuj

update-bootstrap: tmp/stdlib.c tmp/binlib.c
	cp -f tmp/stdlib.c bootstrap/stdlib.c
	cp -f tmp/binlib.c bootstrap/binlib.c
