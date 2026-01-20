# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
make              # Build nujel binary (debug build with -O2)
make release      # Optimized release build with strip
make clean        # Remove all build artifacts
```

## Testing

```bash
make test          # Run standard test suite
make test.slow     # Run extended tests including slow ones
make test.verbose  # Verbose test output
make valgrind      # Run tests under valgrind
```

Tests are written in Nujel and located in `tests/testsuite/*.nuj`. The test runner is `tools/tests.nuj`.

## Project Overview

Nujel is a fast, embeddable Lisp dialect written in C99. It features:
- Bytecode VM (not using C call stack for Nujel function calls)
- Mark-and-sweep garbage collection with static heap pools
- Self-hosted compiler and macro expander (written in Nujel)
- FASL binary format for serialization

## Architecture

### C Runtime (`lib/`)
- `nujel.h` - Public API header
- `nujel-private.h` - Internal definitions including bytecode opcodes (`lOpcode` enum)
- `vm.c` - Bytecode interpreter with computed goto dispatch (when GCC available)
- `reader.c` - S-expression reader
- `allocator.c` - Fixed-size pool allocators for runtime objects
- `garbage-collection.c` - Mark-and-sweep GC

### Standard Library (`stdlib/`)
Core language implemented in Nujel:
- `core/` - Essential macros and functions (`let`, `defn`, list operations)
- `compiler/` - Self-hosted bytecode compiler
  - `frontend/` - Macro expansion, parsing
  - `backend_bytecode/` - Code generation
- `collections/` - List, array, tree operations
- `string/` - String manipulation

### Modules (`stdlib_modules/`)
Optional modules loaded on demand: `net/`, `term/`, `crypto/`, `ansi.nuj`, `repl.nuj`, etc.

### Binary Application (`bin/`)
- `main.c` - Entry point and REPL
- `io.c`, `net.c`, `port.c` - I/O primitives exposed to Nujel
- `binlib/` - Nujel code bundled into the binary (beyond stdlib)

### Bootstrapping
The compiler is self-hosted. `bootstrap/image.c` contains a pre-compiled image that allows building without an existing Nujel binary. To update it after modifying stdlib:
```bash
make update-stdlib
```

## Build System

Uses a portable Makefile that works with GNU make, BSD make, and nmake:
- `GNUmakefile` - GNU make entry point → includes `mk/gmake.mk`
- `Makefile` - BSD/nmake compatibility shim
- `mk/common.mk` - Shared variables
- `mk/targets.mk` - Build targets

## Value Representation

Core value type is `lVal` (16-bit type tag + 64-bit union). Types defined in `lType` enum:
- Immediate: `ltNil`, `ltBool`, `ltInt`, `ltFloat`, `ltSymbol`, `ltKeyword`
- Heap-allocated: `ltPair`, `ltArray`, `ltTree`, `ltMap`, `ltString`, `ltBuffer`, `ltLambda`, `ltMacro`, `ltEnvironment`

## Adding Native Functions

Use the `lAddNativeFunc*` family in C code. Pattern: `lAddNativeFuncVV` takes 2 `lVal` args, `lAddNativeFuncCVV` takes closure + 2 `lVal` args. See `lib/arithmetic.c` for examples.

## Cross-Platform Builds

- WASM: `make nujel.wasm` (requires wasi-sdk)
- Emscripten: `make web/index.html` (requires emsdk)
- Static musl: `make release.musl`
- Windows: Builds with MSVC via `mk/nmake.mk`
