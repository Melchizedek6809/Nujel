# Nujel
A Lisp inspired language for games and other latency sensitive applications.
Development started in order to give [WolkenWelten](https://sr.ht/~melchizedek6809/WolkenWelten/) a scripting runtime.

# Why Nujel?
Mostly because I needed the following for WolkenWelten:

## Easy interaction with C
Since the engine is written in C this needs to be convenient and efficient.

## Should allow for fast code
This might require some effort from the programmer, but it should be possible
to write very fast code for the parts where it actually matters. Because of this
a bunch of decisions were made:
### Static typing
While dynamic typing can be fast, it is much harder to pull off, the much simpler
solution is to put that burden on the programmer. Although Nujel will default to
an any/boxed type.
### Focus on Imperative looping constructs
As with dynamic typing, it is just much easier to support fast for-loops than it is
to make map/reduce fast.

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

# Try it out yourself
You can easily start experimenting with Nujel right in your browser by using a [WASM release](https://wolkenwelten.net/nujel/).

## Build Status
| Operating System   | Master | Develop |
|--------------------|-----------|-----------|
| Windows            | [![Windows](https://github.com/Melchizedek6809/Nujel/actions/workflows/windows.yml/badge.svg?branch=master)](https://github.com/Melchizedek6809/Nujel/actions/workflows/windows.yml)|[![Windows](https://github.com/Melchizedek6809/Nujel/actions/workflows/windows.yml/badge.svg?branch=develop)](https://github.com/Melchizedek6809/Nujel/actions/workflows/windows.yml)|
| MacOS              | [![MacOS](https://github.com/Melchizedek6809/Nujel/actions/workflows/macos.yml/badge.svg?branch=master)](https://github.com/Melchizedek6809/Nujel/actions/workflows/macos.yml)|[![MacOS](https://github.com/Melchizedek6809/Nujel/actions/workflows/macos.yml/badge.svg?branch=develop)](https://github.com/Melchizedek6809/Nujel/actions/workflows/macos.yml)|
| Ubuntu             | [![Ubuntu](https://github.com/Melchizedek6809/Nujel/actions/workflows/ubuntu.yml/badge.svg?branch=master)](https://github.com/Melchizedek6809/Nujel/actions/workflows/ubuntu.yml)|[![Ubuntu](https://github.com/Melchizedek6809/Nujel/actions/workflows/ubuntu.yml/badge.svg?branch=develop)](https://github.com/Melchizedek6809/Nujel/actions/workflows/ubuntu.yml)|
| WASM REPL          | [![WASM](https://github.com/Melchizedek6809/Nujel/actions/workflows/wasm.yml/badge.svg?branch=master)](https://github.com/Melchizedek6809/Nujel/actions/workflows/wasm.yml)|[![WASM](https://github.com/Melchizedek6809/Nujel/actions/workflows/wasm.yml/badge.svg?branch=develop)](https://github.com/Melchizedek6809/Nujel/actions/workflows/wasm.yml)|

| Operating System   | CI Status |
|--------------------|-----------|
| Arch Linux         | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/arch.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/arch.yml?)|
| Alpine Linux       | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/alpine.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/alpine.yml?)|
| FreeBSD            | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/freebsd.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/freebsd.yml?)|
| NetBSD             | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/netbsd.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/netbsd.yml?)|
| OpenBSD            | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/openbsd.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/openbsd.yml?)|
| Debian Sid / ARM64 | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/debian_arm.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/debian_arm.yml?)|
