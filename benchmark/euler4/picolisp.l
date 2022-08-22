(de reverse-num (a)
    (let ret 0
      (while (> a 0)
             (setq ret (+ (* ret 10) (% a 10)))
             (setq a (/ a 10)))
      ret))

(de palindrome? (a)
    (== a (reverse-num a)))

(de bench-run ()
    (let max 0
      (for a 999
           (for b 999
                (let p (* a b)
                  (when (and (palindrome? p)
                             (> p max))
                    (setq max p)))))
      max))

(prinl "The biggest product of 2 3-digit numbers that is a palindrome is: " (bench-run))
(bye)
