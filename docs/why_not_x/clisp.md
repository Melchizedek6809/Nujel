Why not CLISP?
--------------------------------------------------------------

CLISP is another Common Lisp implementation that might be usable as a scripting system for WolkenWelten, since it
uses bytecode it should be quite portable, although it seems that porting it to WASM is a bit tricky due to it doing
some low-level stack Manipulation that is not possible on WASM (haven't looked at the code directly, but found a thread on reddit).

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
(compile 'messung)
(main)
```
This completes in ~0.8s on my laptop, quite snappy. Just be sure to run with `-norc` since loading quicklisp does slow down
the start up considerably.

## Pro
- Batteries included, additionally a lot of CL libs should just work and could be included by default
- Quite fast (test program is almost twice as fast as when interpreted by ECL, making it faster than CPython)

## Contra
- No continuations
- ??? Emscripten/WASM support, might be possible to do but so far haven't found an Emscripten build.
- Very Slow Startup
- Doesn't seem to be very embeddable
- Last Release is 12 Years old? Doesn't seem to get much attention these days.