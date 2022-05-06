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
- [X] Bytecoded
- [ ] Continuations
- [ ] Fibers
- [ ] Serializable Fibers

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
