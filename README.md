Nujel
--------------------------------------------------------
A Lisp inspired language for games and other latency sensitive applications.
Development started in order to give [WolkenWelten](https://sr.ht/~melchizedek6809/WolkenWelten/) a scripting runtime.
While Nujel is still very early in development, and for the most part just a terrible Scheme, it is stabilizing
rapidly. Even though the language is still very much in flux, the following points might give
you an idea as to which direction Nujel is going in and which tradeoffs have been made.

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
