# Nujel
A Lisp inspired language for games and other latency sensitive applications.
Development started in order to give [WolkenWelten](https://sr.ht/~melchizedek6809/WolkenWelten/) a scripting runtime.
While Nujel is still very early in development, and for the most part just a terrible Scheme, it is stabilizing
rapidly. Even though the language is still very much in flux, the following points might give
you an idea as to which direction Nujel is going in and which tradeoffs have been made.

# Why Nujel?
Mostly because I need a language/runtime with the following properties for
WolkenWelten:


## Should allow for fast code
In general the performance of the scripting layer is not that important, except
for the parts where it is, like worldgen. It does not have to be pretty or easy
to write but is essential to allow for certain parts to be optimized extremely.

### Static typing
While dynamic typing can be fast, it is much harder to pull off, the much
simpler solution is to put that burden on the programmer. Although Nujel might
default to an any/boxed type, which should feel similar to dynamic typing.

### Focus on Imperative looping constructs
As with dynamic typing, it is just much easier to support fast for-loops than
it is to make map/reduce fast.


## Portable
Windows/ARM/WASM need to be supported perfectly, requiring x86/Linux would exclude
too many users.

### Build environment
Nujel should only require Make and a somewhat modern C compiler as a build
environement, this makes it much easier to port to various platforms, since they
probably already have a great C compiler.


## Powerful metaprogramming
This mostly means macros/runtime code generation, enabling DSLs which I would love
to use in all kinds of areas of WolkeWelten (Animal Behaviour/WorldGen/UI)


## Easy for beginners
Modding for WolkenWelten should be as open and inclusive as possible, this means spending a lot of
time and effort on polishing documentation, writing tutorials, improving error handling/messages and
building a welcoming and supportive community.


## Highly concurrent
In order to make scripting easier each entity in WolkenWelten should have a fiber associated to it, which
can block or be preempted without interfering with other fibers. This should make scripting a lot easier
since modders can just write, simple, synchronous code.


## No huge GC pauses
While a GC is probably necessary to ensure memory safety and also be easy and convenient for beginners, huge
stop the world GC pauses are just unacceptable for a videogame. To enable this while also allowing for thousands of concurrent
fibers.


# Try it out yourself
You can easily start experimenting with Nujel right in your browser by using a [WASM release](https://wolkenwelten.net/nujel/).

### GitHub CI
| Master | Develop   |
|--------|-----------|
| [![CI](https://github.com/Melchizedek6809/Nujel/actions/workflows/ci.yml/badge.svg?branch=master)](https://github.com/Melchizedek6809/Nujel/actions/workflows/ci.yml) | [![CI](https://github.com/Melchizedek6809/Nujel/actions/workflows/ci.yml/badge.svg?branch=develop)](https://github.com/Melchizedek6809/Nujel/actions/workflows/ci.yml) |

### sourcehut CI
| Operating System     | CI Status |
|----------------------|-----------|
| Arch Linux (gcc)     | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/arch.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/arch.yml?)|
| Arch Linux (clang)   | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/arch_clang.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/arch_clang.yml?)|
| Arch Linux (tcc-git) | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/arch_tcc.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/arch_tcc.yml?)|
| Debian Sid / ARM64   | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/debian_arm.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/debian_arm.yml?)|
| Guix                 | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/guix.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/guix.yml?)|
| Rocky Linux          | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/guix.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/rocky.yml?)|
| Alpine Linux         | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/alpine.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/alpine.yml?)|
| FreeBSD              | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/freebsd.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/freebsd.yml?)|
| NetBSD               | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/netbsd.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/netbsd.yml?)|
| OpenBSD              | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/openbsd.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/openbsd.yml?)|
