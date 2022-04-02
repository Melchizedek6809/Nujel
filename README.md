Nujel
--------------------------------------------------------
A Lisp inspired language for games and other latency sensitive applications.
Development started in order to give [WolkenWelten](https://sr.ht/~melchizedek6809/WolkenWelten/) a scripting runtime.
While Nujel is still very early in development, and for the most part just a terrible Scheme, it is evolving
rapidly.

## Future direction
To get an idea about the design and future direction of Nujel have a look at the [FUTURE.md](/FUTURE.md).

## Current status
Here is a collection of features already implemented, or about to be finished.
- [X] Extensive test suite
- [X] Lexical scoping
- [X] Exceptions
- [X] WASM support
- [X] Formated output (inspired by python3/rust/zig)
- [X] Maps (using binary Trees)
- [X] Garbage Collection (Mark-and-Sweep)
- [X] Macros (expander written in Nujel itself)
- [ ] Continuations
- [ ] Bytecoded (already included but not used by default)

## Try it out
You can try out a current [WASM Build over here](https://wolkenwelten.net/nujel/).

### GitHub CI (Windows/MacOS/Ubuntu)
| Master | Develop   |
|--------|-----------|
| [![CI](https://github.com/Melchizedek6809/Nujel/actions/workflows/ci.yml/badge.svg?branch=master)](https://github.com/Melchizedek6809/Nujel/actions/workflows/ci.yml) | [![CI](https://github.com/Melchizedek6809/Nujel/actions/workflows/ci.yml/badge.svg?branch=develop)](https://github.com/Melchizedek6809/Nujel/actions/workflows/ci.yml) |

### sourcehut CI
| Operating System     | CI Status |
|----------------------|-----------|
| Arch Linux (gcc)     | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/arch.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/arch.yml?)|
| Arch Linux (clang)   | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/arch_clang.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/arch_clang.yml?)|
| Arch Linux (tcc-git) | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/arch_tcc.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/arch_tcc.yml?)|
| Arch Linux (bmake)   | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/arch_bmake.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/arch_bmake.yml?)|
| Debian Sid / ARM64   | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/debian_arm.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/debian_arm.yml?)|
| Guix                 | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/guix.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/guix.yml?)|
| Rocky Linux          | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/guix.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/rocky.yml?)|
| Alpine Linux         | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/alpine.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/alpine.yml?)|
| FreeBSD              | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/freebsd.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/freebsd.yml?)|
| NetBSD               | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/netbsd.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/netbsd.yml?)|
| OpenBSD              | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/openbsd.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/openbsd.yml?)|
