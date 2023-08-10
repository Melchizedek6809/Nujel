#lang racket/base

(define (test-run)
  (do ([i 0 (+ 1 i)]
       [ret 0 (+ ret i)])
      ((>= i 10000000) ret)))

(display (test-run))
(newline)
