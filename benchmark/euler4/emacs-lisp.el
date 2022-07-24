;; The biggest product of 2 3-digit numbers that is a palindrome
;; https://projecteuler.net/problem=4

(require 'cl-lib)

(defun reverse-num (a)
  (let ((ret 0))
    (while (> a 0)
           (progn (setf ret (+ (* ret 10) (mod a 10)))
                  (setf a (floor a 10))))
    ret))

(defun palindrome? (a)
  (eq a (reverse-num a)))

(defun start-search ()
  (let ((max 0))
    (dotimes (a 1000 max)
      (dotimes (b 1000)
        (let ((p (* a b)))
          (when (and (palindrome? p)
                     (> p max))
            (setf max p)))))))

(byte-compile 'reverse-num)
(byte-compile 'palindrome?)
(byte-compile 'start-search)

(princ (format "The biggest product of 2 3-digit numbers that is a palindrome is: %d\n" (start-search)))
