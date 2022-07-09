;;; WC -- One of the Kernighan and Van Wyk benchmarks.
;;; Rewritten by Will Clinger into more idiomatic (and correct!) Scheme.
;;;
;;; This program was originally written as a benchmark for the Larceny project.
;;; More info about Larceny can be found here: https://github.com/larcenists/larceny

(define (wcport port)
  (define (loop nl nw nc inword?)
    (let ((x (read-char port)))
      (cond ((eof-object? x)
             (list nl nw nc))
            ((char=? x #\space)
             (loop nl nw (+ nc 1) #f))
            ((char=? x #\newline)
             (loop (+ nl 1) nw (+ nc 1) #f))
            (else
             (loop nl (if inword? nw (+ nw 1)) (+ nc 1) #t)))))
  (loop 0 0 0 #f))

(let ((res (call-with-input-file "benchmark/bib.txt" wcport)))
     (display "Lines: ")
     (display (car res))
     (newline)

     (display "Words: ")
     (display (cadr res))
     (newline)

     (display "Characters: ")
     (display (car (cdr (cdr res))))
     (newline))
