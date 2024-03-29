Nujel
--------------------------------------------------------
A fast, tiny and and embeddable Lisp dialect.
This started out as an experiment to provide a fast scripting system for [WolkenWelten](https://sr.ht/~melchizedek6809/WolkenWelten/),
without bloating up the executable size. Now I'm just enjoying working on the runtime and language, trying to keep everything as simple as possible
while improving performance.

## Performance
To make sure that there are no performance regressions, benchmarks are regularly
run which are also used to compare Nujel with other runtimes, if you like
colorful charts you can see the results [here](https://nujel.net/performance/report.html) here. Beware however that these benchmarks only test a tiny part of the language.

## Syntax
A lot of the language is still undocumented, this is because I want to preserve myself the ability to change those parts of the language as I develop a better feeling for what works and what doesn't. You can look in the [docs](./docs/README.md) directory for documentation about the various parts of the Nujel language as well as how likely that part is to change.

## Current status
Here is a collection of features already implemented, or about to be finished.
- [X] Garbage collection (simple Mark-and-Sweep for now)
- [X] Extensive test suite
- [X] Lexical scoping
- [X] WASM support (only via Emscripten)
- [X] Formatted output (inspired by Python/Rust/Zig)
- [X] Maps (using binary trees)
- [X] Macros (expander written in Nujel itself)
- [X] Bytecoded (compiler/assembler/disassembler all written in Nujel)
- [X] Constant Folding
- [X] Module system
- [X] Printer written in Nujel
- [X] "Stackless" funcalls (not using the C call stack for Nujel funcalls)
- [X] Value types
- [X] Exceptions (without using setjmp/longjmp for improved portability, especially relevant for WASM/WASI)
- [X] Gopher client and terminal browser
- [X] Simple HTTP 1.1 client
- [X] (somewhat) useable object system
- [X] Binary/FASL format
- [ ] Simple HTTP 1.1 server
- [ ] Reader written in Nujel (requires FASL for bootstrapping)
- [ ] Tail-call optimization (doesn't seem to be much of a problem right now)
- [ ] Fibers

## Current Limitations
These will be addressed in later versions
- [ ] Static Heap (works far better than expected)

## Documentation
I've started to write some documentation, which you can read here: [https://nujel.net/](https://nujel.net/)

### GitHub CI (Windows/MacOS/Ubuntu/WASM)
| Master | Develop   |
|--------|-----------|
| [![CI](https://github.com/Melchizedek6809/Nujel/actions/workflows/ci.yml/badge.svg?branch=master)](https://github.com/Melchizedek6809/Nujel/actions/workflows/ci.yml) | [![CI](https://github.com/Melchizedek6809/Nujel/actions/workflows/ci.yml/badge.svg?branch=develop)](https://github.com/Melchizedek6809/Nujel/actions/workflows/ci.yml) |

### sourcehut CI
| Operating System     | Master | Develop |
|----------------------|--------|---------|
| Arch Linux (gcc)     | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/arch.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/arch.yml?)| [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/arch.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/arch.yml?)|
| Arch Linux (tcc-git) | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/arch_tcc.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/arch_tcc.yml?)| [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/arch_tcc.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/arch_tcc.yml?)|
| Debian Sid / ARM64   | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/debian_arm.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/debian_arm.yml?)| [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/debian_arm.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/debian_arm.yml?)|
| Guix                 | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/guix.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/guix.yml?)| [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/guix.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/guix.yml?)|
| Rocky Linux          | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/rocky.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/rocky.yml?)| [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/rocky.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/rocky.yml?)|
| Alpine Linux         | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/alpine.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/alpine.yml?)| [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/alpine.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/alpine.yml?)|
| FreeBSD              | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/freebsd.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/freebsd.yml?)| [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/freebsd.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/freebsd.yml?)|
| NetBSD               | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/netbsd.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/netbsd.yml?)| [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/netbsd.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/netbsd.yml?)|
| OpenBSD              | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/openbsd.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/openbsd.yml?)| [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/openbsd.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/openbsd.yml?)|
