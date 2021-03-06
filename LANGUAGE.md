# The Nujel Programming Language

## Foreword
Most functions/macros borrow their name directly from Clojure or Common Lisp, while the reader syntax is very similar to (Guile) Scheme, with some changes due to different semantics.

## Parentheses
One very obvious difference however is that Nujel defaults to using brackets instead of parentheses, you can however use parentheses as well since in Nujel (just like most Scheme's) the two are interchangeable.
```scheme
[+ 1 2] ; => 3
(+ 1 2) ; => 3
```
Apart from that Nujel uses pretty standard S-Expression notation, including support for dotted pairs:
```scheme
[cdr '[a . b]] ; => b
```

## Comments
Comments use Scheme syntax, and some SRFI's have been implemented directly.
```scheme
; A single semicolon comments out everything until the next line-break
(+ 1 #;2 3) ; => 4 You can use #; to comment out the following form, should be SRFI-62 compatible!
#|
 | Nujel also allows for SRFI-30 like nested Multi-line comments
 |#
[comment [println "A message, never to be printed."]] ; Nujel also has the comment macro, this however returns #nil unlike #;
```

## Numbers
Nujel supports normal decimal notation and treats `,` and `_` as whitespace characters so you can split big numbers for increased legibility.  There is also special syntax for binary, octal and hexadecimal literals. Scientific notation is **not** supported (that might change but for now I haven't seen the necessity of it).
```scheme
9 ; probably not that suprising
; => 9

100,0; possible, but probably shouldn't be commited that way
; => 1000

1,0,0,0; also a possibility...
; => 1000

1,000 ; much better!
; => 1000

1_000 ; Underscore is also workable, although mostly preferrable for non decimal literals
; => 1000

#b10000 ; This way we can write binary literals
; => 16

#b0001_0000 ; Especially here does it become useful that we can use _ and , to split our literal wherever we choose
; => 16

#x12_34 ; Also helps for hex literals
; => 4660

#o10 ; Octal literals are also possible
; => 8

0x123 ; Using an 0x prefix does NOT work and results in a read error being thrown
; => :read-error

```

## Quote / Quasiquote
Quote works as you would expect, quasiquote however uses the clojure syntax for `unquote`/`unquote-splicing`. This is because I want to be able to treat `,` as white-space (it also looks very similar to a period, which I dislike).
```scheme
'a ; You can quote symbols
; => a

'[1 2 3] ; Or lists
; => [1 2 3]

'(1 2 3) ; With parentheses or brackets
; => [1 2 3]

`[1 2 ~[+ 1 1 1]] ; To unquote you can use a tilde
; => [1 2 3]

`[1 ~@[list 2 3]] ; And ~@ for unquote-splicing
; => [1 2 3]

```

## Symbols / Keywords
Keywords have to start, or end, with a single `:` character and are self-evaluating. They are a distinct type from symbols but can be easily converted.
```scheme
:asd ; A keyword
; => :asd

asd: ; Another keyword
; => asd:

[= asd: :asd] ; It doesn't matter where we put the colon
; => #t

[= :asd [symbol->keyword 'asd]]
; => #t
```

## Functions
Functions use `defn` and `fn`, just like Clojure, however we can freely choose where we want to use brackets and parentheses.  Variadic functions also use the scheme notation with a dotted pair, or omitting the brackets altogether.
```scheme
[defn double [??] [* ?? ??]]
[double 2]
; => 4

[defn multiply-vals [val . l]
      [map l [fn [v] [* v val]]]]
[multiply-vals 2 1 2 3]
; => [2 4 6]

[defn my-list l l]
[my-list 1 2 3 4]
; => [1 2 3 4]
```