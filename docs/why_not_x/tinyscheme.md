Why not tinyscheme?
----------------------------------------------

I tried out tinyscheme 1.41 and am quite impressed by how tight everything is, compile times are nearly instantaneous. But runtime performance is not where I would like it to be and the standard library included is a bit too minimalistic for me.

## Pro
- Small Codebase (~5k SLOC)
- Tiny Executable (~200K dynamic binary, ~1.5M static binary)
- Fast!!! compile times (compiles in about 0.6s on my Laptop)

## Contra
- VERY slow
- No maps
- No vectors (the 3 xyz type, very convenient for 3D games)
- Could use more comments / Code isn't that clear

Overall, **performance** is the main reason I didn't look further into using tinyscheme, mainly I tried the following Code that adds all integers from 1 to a million:
```scheme
(define (test i ret)
  (cond ((>= i 1000000) ret)
        (else (test (+ 1 i) (+ ret i)))))
(display (test 0 0))
(newline)
```
Which on my laptop at least took **~3.1 seconds**, in comparison a similar program in *Nujel* takes only **~0.16 seconds**, which is about a **20x** difference in performance. And that is even though Nujel is using a slow tree walking evaluator. Fast Scheme implementation like *Guile* or *Gauche* finish this same program in about **~0.03 seconds**.