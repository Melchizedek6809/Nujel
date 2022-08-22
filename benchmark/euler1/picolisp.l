(de bench-run ()
    (let ret 0
      (for i (- 10000000 1)
           (when (or (== 0 (% i 3))
                     (== 0 (% i 5)))
             (setq ret (+ ret i))))
    ret))
(prinl "The sum is: " (bench-run))
(bye)
