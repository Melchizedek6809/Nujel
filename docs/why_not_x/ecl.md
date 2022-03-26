ECL?
---------------------------------------------------------

I tried out ECL 21.2.1 and have to say I am very impressed overall! I am a bit disappointed in the overall performance, since interpreted functions seem just a bit faster than Nujel and while compiled functions are quite fast, the compiler is very slow, probably because as far as I understood it is generating C code, executing the system's C Compiler to create a .so/.dll and then dlopen it afterwards, but this seems to result in Issues on platforms like MacOS/Windows where a C compiler is quite unlikely, or even worse: WASM, where a C compiler in just plain unavailable.

## Test Program
```lisp
(defun messung ()
  (let ((ret 0))
  (dotimes (i 10000000)
   ;(incf ret i)) - Not sure why but this is A LOT slower on ECL, while on CLISP it seems a tiny bit faster
	(incf ret i))
  ret))

(defun main ()
  (format T "~a~%" (messung))
  (quit))

(setf *compile-verbose* nil)
;(compile 'messung)
(main)
```
Which I then ran using: `time ecl --load test.lisp` which takes **~1.6** seconds on my laptop (as compared to **~2.1** seconds for *Nujel* or **~1.1** seconds for *CPython*...), loading the file and then executing `(time (main))` didn't seem to help much.

## Pro
- Batteries included, additionally a lot of CL libs should just work and could be included by default
- OK build times, only took a couple of minutes to build on my machine
- Can generate C code, so some default mods could be compiled to native C code for performance

## Contra
- Pretty slow (seems to only be slightly faster than the Nujel tree walker for interpreted code, compiled code seems to be quite fast BUT the compiler is quite slow, even for very simple functions)
- Pretty Big, .so is about 11M on my laptop, and it links to some other libraries as well
- No continuations
- ??? Emscripten/WASM support, might be possible to do but so far haven't found an Emscripten ECL build.

In general I feel as though ECL kinda falls in-between SBCL/CLISP where I'd rather use SBCL if I can generate native code, or CLISP when this is not a possibility and I have to use bytecode.