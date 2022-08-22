(de bench-run ()
    (let ret 0
      (for i (- 10000000 1) (setq ret (+ ret i)))
    ret))
(prinl "The result is: " (bench-run))
(bye)
