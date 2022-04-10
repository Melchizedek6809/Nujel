(defun test ()
  (let ((ret 0))
    (dotimes (i 10000000)
      (incf ret i))
    ret))

(compile 'test)
(format T "THE RESULT IS ~a~%" (test))
