(defun run-test ()
  (prog (ret i)
     (set 'ret 0)
     (set 'i 0)
   loop
     (cond ((> i 9999999)
            (return ret))
           (t (set 'ret (+ ret i))
              (set 'i (+ i 1))
              (go loop)))))

(princ "This results in: ")
(princ (run-test))
(exit 0)
