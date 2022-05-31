(require 'cl-lib)
(defun test ()
  (let ((ret 0))
    (dotimes (i 10000000)
      (cl-incf ret i))
    ret))
(byte-compile 'test)

(princ (format "THE RESULT IS %d\n" (test)))
