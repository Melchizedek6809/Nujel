What about Guile?
----------------------------------------------------
Guile is another great Scheme implementation, one I use quite often since it works quite well on Windows
and is included on many *nix systems already. Additionally it has fantastic documentation (as seems to be the case for most GNU Projects) which I like to read quite often to learn more about Scheme.

```scheme
(define (test i ret)
  (cond ((>= i 10000000) ret)
        (else (test (+ 1 i) (+ ret i)))))
(display (test 0 0))
(newline)
```
Running this script takes about **~0.08s** on my Laptop with *3.0.8*, or **~0.15s** with *2.2.7* as compared to *Nujel* which takes **~1.3s** for a similar Program making it about **10x** faster than the current *Nujel* tree walker!

## Pros
+ Continuations!
+ Batteries included, contains pretty much everything needed
+ Quite fast, although the Lua Interpreter is still ~2x faster than Guile 2.2.7, Guile 3.0.8 seems to be about as fast as the Lua interpreter.

## Cons
- SLOW build time, took ~13 Minutes with 8 Cores full blazing, can't even use all cores since then my entire machine locks up...
- ? Emscripten/WASM support is unclear right now
- Quite big, guile 3.0.8 has a .so that is 5.5M on my system, the one included by Arch built from 2.2.7 is only 1.2M, but is quite a bit slower

In general I think for WolkenWelten Guile 2.X is more interesting than 3.X since it seems that the addition of a JIT is the main change which won't help for WASM and might be the cause for the increase in .so size. Additionally Guile 2.2.7 can be builtin ~3m23s on my Laptop with 8 Cores. Mainly I think that the main problem is WASM compatibility, since that is something I am not willing to give up. So that means looking into compiling Guile with Emscripten.