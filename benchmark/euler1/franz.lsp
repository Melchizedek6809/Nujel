(defun three-or-five-p (v)
  (or (zerop (mod v 3))
      (zerop (mod v 5))))

(defun run-test ()
  (prog (ret i)
     (set 'ret 0)
     (set 'i 0)
   loop
     (cond ((> i 9999999)
            (return ret))
           ((three-or-five-p i)
            (set 'ret (+ ret i))))
     (set 'i (+ i 1))
     (go loop)))

(princ "The sum is: ")
(princ (run-test))
(exit 0)
