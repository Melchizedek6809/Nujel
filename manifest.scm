;; manifest.scm
(setenv "CC" "gcc")
(specifications->manifest
  (list "gcc-toolchain"
        "make"))
