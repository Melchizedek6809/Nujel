# Nujel
A Lisp inspired language for games and other latency sensitive applications.
Development started in order to give [WolkenWelten](https://sr.ht/~melchizedek6809/WolkenWelten/) a scripting layer.

# Why Nujel?
Mostly because I wanted the following for WolkenWelten:
- A Lisp
- with static typing (still not implemented!)
- embeddable in C
- designed for beginners

## Trees / Maps
Nujel also has built-in support for maps using binary trees, including support by the reader for a convenient
syntax to definie literals with, for example `[@[:one 1 :two 2] :two]` returns `2`.

# Try it out yourself
You can easily start experimenting with Nujel right in your Browser by using a [WASM Release](https://wolkenwelten.net/nujel/).

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
