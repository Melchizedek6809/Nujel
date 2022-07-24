;; The biggest product of 2 3-digit numbers that is a palindrome
;; https://projecteuler.net/problem=4

(defun reverse-num (a)
  (declare (fixnum a))
  (let ((ret 0))
    (declare (fixnum ret))
    (loop while (> a 0) do
           (progn (setf ret (+ (* ret 10) (rem a 10)))
                  (setf a (floor a 10))))
    ret))

(defun palindrome? (a)
  (declare (fixnum a))
  (eq a (reverse-num a)))

(defun start-search ()
  (let ((max 0))
    (declare (fixnum max))
    (dotimes (a 1000 max)
      (declare (fixnum a))
      (dotimes (b 1000)
        (declare (fixnum b))
        (let ((p (* a b)))
          (declare (fixnum p))
          (when (and (palindrome? p)
                     (> p max))
            (setf max p)))))))

(compile 'reverse-num)
(compile 'palindrome?)
(compile 'start-search)

(format T "The biggest product of 2 3-digit numbers that is a palindrome is: ~a~%" (start-search))
