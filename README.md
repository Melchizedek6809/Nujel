# Nujel
A Lisp inspired language for games and other latency sensitive applications.
Developed to give [WolkenWelten](https://sr.ht/~melchizedek6809/WolkenWelten/) a scripting layer.

| Operating System  | Master | Develop |
|-------------------|-----------|-----------|
| Windows           | [![Windows](https://github.com/Melchizedek6809/Nujel/actions/workflows/windows.yml/badge.svg?branch=master)](https://github.com/Melchizedek6809/Nujel/actions/workflows/windows.yml)|[![Windows](https://github.com/Melchizedek6809/Nujel/actions/workflows/windows.yml/badge.svg?branch=develop)](https://github.com/Melchizedek6809/Nujel/actions/workflows/windows.yml)|
| MacOS             | [![MacOS](https://github.com/Melchizedek6809/Nujel/actions/workflows/macos.yml/badge.svg?branch=master)](https://github.com/Melchizedek6809/Nujel/actions/workflows/macos.yml)|[![MacOS](https://github.com/Melchizedek6809/Nujel/actions/workflows/macos.yml/badge.svg?branch=develop)](https://github.com/Melchizedek6809/Nujel/actions/workflows/macos.yml)|
| Ubuntu            | [![Ubuntu](https://github.com/Melchizedek6809/Nujel/actions/workflows/ubuntu.yml/badge.svg?branch=master)](https://github.com/Melchizedek6809/Nujel/actions/workflows/ubuntu.yml)|[![Ubuntu](https://github.com/Melchizedek6809/Nujel/actions/workflows/ubuntu.yml/badge.svg?branch=develop)](https://github.com/Melchizedek6809/Nujel/actions/workflows/ubuntu.yml)|

| Operating System  | CI Status |
|-------------------|-----------|
| Arch Linux        | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/arch.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/arch.yml?)|
| Alpine Linux      | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/alpine.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/alpine.yml?)|
| FreeBSD           | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/freebsd.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/freebsd.yml?)|
| NetBSD            | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/netbsd.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/netbsd.yml?)|
| OpenBSD           | [![builds.sr.ht status](https://builds.sr.ht/~melchizedek6809/Nujel/commits/openbsd.yml.svg)](https://builds.sr.ht/~melchizedek6809/Nujel/commits/openbsd.yml?)|


## Infix operations
One part that is quite different to most Lisps is that one can evaluate `[display [1 + 2 * 3]]` and get `7`,
as one would expect from other languages. This is due to each type having a convenience function associated to them,
which is an infix evaluator in the case of the numeric types, or `cat` for strings and `arr-ref` for arrays.
So you can also write `[display ["The result is: " [1 + 1]]]`. Or `[display [#["Yay" "Nay"] 0]]`.

## Static Typing
Nujel will use a static type system, since this makes it much easier to generate
fast code. By using type inference and multiple dispatch the inconvenience shouldn't
be too big.
