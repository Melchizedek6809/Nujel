Nujel
--------------------------------------------------------
A fast, tiny and easy to embed Lisp dialect.
While providing a scripting system for [WolkenWelten](https://sr.ht/~melchizedek6809/WolkenWelten/)
was what started the development effort for Nujel, it is now trying to become
useful for general scripting tasks as well.

## Performance
To make sure that there are no performance regressions, benchmarks are regularly
run which are also used to compare Nujel with other runtimes, if you like
colorful charts you can see the results [here](https://wolkenwelten.net/nujel/report.html) here. Beware however that these benchmarks only test a tiny part of the language.

## Syntax
A lot of the language is still undocumented, this is because I want to preserve myself the ability to change those parts of the language as I develop a better feeling for what works and what doesn't. You can look in the [docs](./docs/README.md) directory for documentation about the various parts of the Nujel language as well as how likely that part is to change.

## Current status
Here is a collection of features already implemented, or about to be finished.
- [X] Garbage collection (simple Mark-and-Sweep for now)
- [X] Extensive test suite
- [X] Lexical scoping
- [X] Exceptions (using setjmp/longjmp)
- [X] WASM support (only via Emscripten)
- [X] Formatted output (inspired by Python/Rust/Zig)
- [X] Maps (using binary trees)
- [X] Macros (expander written in Nujel itself)
- [X] Bytecoded (compiler/assembler/disassembler all written in Nujel)
- [X] Constant Folding
- [X] Module system
- [X] Printer written in Nujel
- [ ] "Stackless" funcalls (not using the C call stack for Nujel funcalls)
- [ ] Tail-call optimization
- [ ] Continuations
- [ ] Value types
- [ ] Binary FASL format with C reader/writer
- [ ] Reader written in Nujel
- [ ] Simple HTTP 1.1 client/server

## Current Limitations
These will be addressed in later versions
- [X] No file streams (can only read/write files in their entirety)
- [ ] No networking support
- [ ] Static Heap (works far better than expected)

## Try it out
You can try out a current [WASM Build over here](https://wolkenwelten.net/nujel/).

### GitHub CI (Windows/MacOS/Ubuntu)
| Master | Develop   |
|--------|-----------|
| [![CI](https://github.com/Melchizedek6809/Nujel/actions/workflows/ci.yml/badge.svg?branch=master)](https://github.com/Melchizedek6809/Nujel/actions/workflows/ci.yml) | [![CI](https://github.com/Melchizedek6809/Nujel/actions/workflows/ci.yml/badge.svg?branch=develop)](https://github.com/Melchizedek6809/Nujel/actions/workflows/ci.yml) |

### sourcehut CI
| Operating System     | Master | Develop |
|----------------------|--------|---------|
| Arch Linux (gcc)     | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/arch.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/arch.yml?)| [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/arch.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/arch.yml?)|
| Arch Linux (clang)   | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/arch_clang.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/arch_clang.yml?)| [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/arch_clang.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/arch_clang.yml?)|
| Arch Linux (tcc-git) | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/arch_tcc.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/arch_tcc.yml?)| [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/arch_tcc.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/arch_tcc.yml?)|
| Arch Linux (bmake)   | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/arch_bmake.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/arch_bmake.yml?)| [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/arch_bmake.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/arch_bmake.yml?)|
| Debian Sid / ARM64   | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/debian_arm.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/debian_arm.yml?)| [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/debian_arm.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/debian_arm.yml?)|
| Guix                 | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/guix.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/guix.yml?)| [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/guix.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/guix.yml?)|
| Rocky Linux          | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/rocky.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/rocky.yml?)| [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/rocky.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/rocky.yml?)|
| Alpine Linux         | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/alpine.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/alpine.yml?)| [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/alpine.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/alpine.yml?)|
| FreeBSD              | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/freebsd.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/freebsd.yml?)| [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/freebsd.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/freebsd.yml?)|
| NetBSD               | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/netbsd.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/netbsd.yml?)| [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/netbsd.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/netbsd.yml?)|
| OpenBSD              | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/openbsd.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/master/openbsd.yml?)| [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/openbsd.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/develop/openbsd.yml?)|
