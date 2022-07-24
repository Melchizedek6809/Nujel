#lang racket/base

(define [multiple-of-3-or-5? α]
  [or [zero? [modulo α 3]]
      [zero? [modulo α 5]]])

(define (test-run i ret)
  (cond ((>= i 10000000) ret)
        (else (test-run (+ 1 i) (if (multiple-of-3-or-5? i) (+ ret i) ret)))))

(display "The sum is:")
(display (test-run 0 0))
(newline)
