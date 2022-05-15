#lang typed/racket

(: test-run (-> Integer Integer Integer))
(define (test-run i ret)
  (cond ((>= i 10000000) ret)
        (else (test-run (+ 1 i) (+ ret i)))))

(display (time (test-run 0 0)))
(newline)
