(define (fib n)
      (cond ((= 0 n) 0)
            ((= 1 n) 1)
            (#t (+ (fib (- n 1)) (fib (- n 2))))))

(display "fib(40) = ")
(display (fib 40))
(display "\n")
