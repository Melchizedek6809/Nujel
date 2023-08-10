(define (test-run)
    (do ([i 0 (+ i 1)]
         [ret 0 (+ ret i)])
           ((>= i 10000000) ret)))

(display (test-run))
(newline)
