#lang racket/base

(define (multiple-of-3-or-5? α)
  (or (zero? (modulo α 3))
      (zero? (modulo α 5))))

(define (test-run)
  (do ([i 0 (+ 1 i)]
       [ret 0])
      ((>= i 10000000) ret)
    (when (multiple-of-3-or-5? i)
      (set! ret (+ ret i)))))

(display "The sum is:")
(display (test-run))
(newline)
