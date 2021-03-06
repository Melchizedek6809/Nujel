(define (test-run i ret)
  (if (< i 10000000)
      (test-run (+ 1 i)
                (if (or (zero? (modulo i 3))
                        (zero? (modulo i 5)))
                    (+ ret i)
                    ret))
      ret))

(display "The sum is: ")
(display (test-run 0 0))
(newline)
