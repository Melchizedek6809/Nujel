(define (test-run)
  (let ((ret 0))
       (do ((i 0 (+ i 1)))
           ((>= i 10000000) ret)
         (set! ret (+ ret i)))))

(display (test-run))
(newline)
