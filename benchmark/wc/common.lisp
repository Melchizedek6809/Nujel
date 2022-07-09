(defun word-count (file-name)
    (with-open-file (s file-name)
        (let ((nl 0) (nw 0) (nc 0) (in-word? nil))
            (loop for c = (read-char s nil)
                    while c
                    do (progn (incf nc)
                    (case c
                        (#\ (setf in-word? nil))
                        (#\Linefeed (progn (incf nl)
                                           (setf in-word? nil)))
                        (otherwise (when (not in-word?) (incf nw))
                                   (setf in-word? T)))))
            (list nl nw nc))))
(compile 'word-count)

(let ((res (word-count "benchmark/bib.txt")))
     (format T "Lines: ~a~%Words: ~a~%Characters: ~a~%" (car res) (cadr res) (caddr res)))