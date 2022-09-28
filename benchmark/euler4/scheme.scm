;; The biggest product of 2 3-digit numbers that is a palindrome
;; https://projecteuler.net/problem=4

(define (reverse-num a ret)
  (if (< a 1)
      ret
      (reverse-num (truncate (/ a 10))
                   (+ (* ret 10) (truncate (modulo a 10))))))

(define (palindrome? a)
  (= a (reverse-num a 0)))

(define (search a b ret)
  (cond ((>= a 1000) ret)
        ((>= b 1000) (search (+ a 1) 0 ret))
        (else (search a (+ 1 b) (if (palindrome? (* a b))
                                    (max ret (* a b))
                                    ret)))))

(display "The biggest product of 2 3-digit numbers that is a palindrome is: ")
(display (search 0 0 0))
(newline)
