![Automated Tests](https://github.com/Melchizedek6809/Nujel/actions/workflows/tests.yml/badge.svg?branch=master)

# Nujel
A Lisp inspired language for games and other latency sensitive applications.
## This Language should NOT be used for anything important right now, maybe in a couple years
For now Nujel is still pretty similar to others Lisps, but it will probably distance
itself further one commit at a time.

### Infix operations
One part that is quite different to most Lisps is that one can evaluate `[display [1 + 2 * 3]]` and get `7`,
as one would expect from other languages. This is due to each type having a convenience function associated to them,
which is an infix evaluator in the case of the numeric types, or `cat` for strings and `arr-ref` for arrays.
So you can also write `[display ["The result is: " [1 + 1]]]`. Or `[display [#["Yay" "Nay"] 0]]`.

## Things to expect
Nujel will switch to a static type system and use type inference and multimethods to make this convenient.

## Things that are unknown right now
Memory management, right Nujel uses a simple mark and sweep GC but that will probably change, although
I am not quite sure where exactly, still have to ponder that moreand do some profiling, for now the GC pauses
are acceptable due to the small heap so other things are more important right now.
