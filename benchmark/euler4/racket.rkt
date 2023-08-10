#lang racket/base

(define (reverse-num a ret)
  (if (< a 1)
      ret
      (reverse-num (floor (/ a 10))
                   (+ (* ret 10) (floor (modulo a 10))))))

(define (palindrome? a)
  (= a (reverse-num a 0)))

(define (search)
  (do ((a 0 (+ 1 a))
       (ret 0))
      ((>= a 1000) ret)
    (do ((b 0 (+ 1 b)))
        ((>= b 1000))
      (when (palindrome? (* a b))
          (set! ret (max ret (* a b)))))))

(display "The biggest product of 2 3-digit numbers that is a palindrome is: ")
(display (search))
(newline)
