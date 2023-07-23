;; The biggest product of 2 3-digit numbers that is a palindrome
;; https://projecteuler.net/problem=4

(define (reverse-num a ret)
  (if (< a 1)
      ret
      (reverse-num (truncate (/ a 10))
                   (+ (* ret 10) (truncate (modulo a 10))))))

(define (palindrome? a)
  (= a (reverse-num a 0)))

(define (search)
  (let ((max-val 0))
  (do ((a 0 (+ a 1)))
      ((>= a 1000) max-val)
    (do ((b 0 (+ b 1)))
        ((>= b 1000))
      (let ((p (* a b)))
           (if (palindrome? p)
               (set! max-val (max p max-val))))))))

(display "The biggest product of 2 3-digit numbers that is a palindrome is: ")
(display (search))
(newline)
