;; Produces test output for complex functions using GNU Calc.
;;
;; Generate output with
;;
;;   emacs -batch -l test.el -f test1 | sed  's/00\+e/0e/g' > results1.h
;;   emacs -batch -l test.el -f test2 | sed  's/00\+e/0e/g' > results.h
;;
;; Note: this takes a long time to run

;; set Calc to use radians mode, turn off symbolic evaluation and use
;; a reasonable precision.

(setq calc-display-working-message t) ;; display short working messages
(setq max-lisp-eval-depth 4000)
(setq max-specpdl-size 2400)
(setq calc-internal-prec 64) ;; probably unnecessarily high, but found
			     ;; a few discrepancies at prec=20
(setq calc-infinite-mode t)
(setq calc-angle-mode 'rad)
(setq calc-float-format '(sci 20))
;;(setq calc-full-float-format '(sci 0))

(setq var-EvalRules (calc-eval "[ sec(x) := 1 / cos(x),
  csc(x) := 1 / sin(x),
  cot(x) := 1 / tan(x),
  sech(x) := 1 / cosh(x),
  csch(x) := 1 / sinh(x),
  coth(x) := 1 / tanh(x),
  arcsec(x) := arccos(1 / x),
  arccsc(x) := arcsin(1 / x),
  arccot(x) := arctan(1 / x),
  arcsech(x) := arccosh(1 / x),
  arccsch(x) := arcsinh(1 / x),
  arccoth(x) := arctanh(1 / x),
  abs2(x) := x * conj(x),
  logabs(x) := log(abs(x)),
  sqrtreal(x) := sqrt(x),
  arcsinreal(x) := arcsin(x),
  arccosreal(x) := arccos(x),
  arccoshreal(x) := arccosh(x),
  arctanhreal(x) := arctanh(x),
  arccscreal(x) := arccsc(x),
  arcsecreal(x) := arcsec(x) ]" 'raw))

;; Convert floating point numbers into exact binary representation

(defun binary (x)
  (if (= x 0)
      x
    (let ((y (/ x (expt 2.0 (+ 1 (logb (abs x)))))))
      (concat (if (>= y 0) "+" "-") (mantissa (abs y)) "e" (+ 1 (logb (abs x)))))))

(defun mantissa (x)
  (let ((y "2#0."))
    (while (> x 0)
      (progn ;(message "x = %g  y = %s" x y)
             (setq x (* 2 x))
             (if (>= x 1) 
                 (progn (setq y (concat y "1"))
                        (setq x (- x 1)))
               (setq y (concat y "0")))))
    y))



;;(binary 9.9999999999999995e-08)


(defun reflections (a b)
  (let ((a (float a)) (b (float b)))
    (list
     (list a b)
     (list a (- b))
     (list (- a) b)
     (list (- a) (- b)))))

(defun permute (fn a b)
  (let ((a1 a) (result nil))
    (while a1
      (progn 
        (let ((b1 b))
          (while b1
            (progn 
              (setq result (append result (funcall fn (car a1) (car b1))))
              (setq b1 (cdr b1))
              )
            )
          )
        (setq a1 (cdr a1))
        )
      )
    result
    )
  )

(defun trim (a)
  (let ((result nil))
    (while a
      (setq result (cons (car a) result))
      (setq a (delete (car a) a))
      )
    (reverse result))
)

(defun combine (a b)
  (trim (permute 'reflections a b)))



(defun evaltest (function arg)
  (let* ((x (nth 0 arg))
         (y (nth 1 arg))
         (z (format "(%s,%s)" (binary x) (binary y)))
         (v (concat "clean(0.0 + clean(" function "(" z "),60))"))
         (result (calc-eval v)))

    (message "%s (%g %g) = %s" function x y result)
    (if (string-match "clean(\\(.*\\))" result)
        (setq result (replace-match "\\1" nil nil result)))
    (if (string-match "clean(\\(.*\\), *[0-9]*)" result)
        (setq result (replace-match "\\1" nil nil result)))
    (if (string-match "(\\(.*\\),\\(.*\\))" result)
        (setq result (replace-match "\\1,\\2" nil nil result)))
    (if (string-match "^\\([^,]*\\)$" result)
        (setq result (replace-match "\\1, 0.0" nil nil result)))
    (if (not (or (string-match "(" result) ;; skip any unsimplified results
                 (string-match "inf" result)))  
        (princ (format "  {FN (%s), ARG(%.20e,%.20e), RES(%s)},\n" function x y result))
      )))


(defun evaltestreal (function arg)
  (let* ((x arg)
         (z (format "(%s)" (binary x)))
         (fn (if (string-match "_" function)
                 (replace-match "" nil nil function)
               function))
         (v (concat "clean(0.0 + clean(" fn "(" z "),60))"))
         (result (calc-eval v)))

    (message "%s (%g) = %s" function x result)
    (if (string-match "clean(\\(.*\\))" result)
        (setq result (replace-match "\\1" nil nil result)))
    (if (string-match "clean(\\(.*\\), *[0-9]*)" result)
        (setq result (replace-match "\\1" nil nil result)))
    (if (string-match "(\\(.*\\),\\(.*\\))" result)
        (setq result (replace-match "\\1,\\2" nil nil result)))
    (if (string-match "^\\([^,]*\\)$" result)
        (setq result (replace-match "\\1, 0.0" nil nil result)))
    (if (not (or (string-match "(" result) ;; skip any unsimplified results
                 (string-match "inf" result)))  
        (princ (format "  {FN (%s), %.20e, RES(%s)},\n" function x result))
      )))


;;(evaltest "sin" "10" "0")

;; loop over all possible combinations of a,b,c

(defun test (a b)
  (let ((b1 b))
    (while b1
      (progn
        (let* ((z (car b1)))
          (if (listp z) (evaltest a z)
            (evaltestreal a z)))
        (setq b1 (cdr b1))
        )
      )
    )
  )
  
;;

(setq pi 3.14159265358979323846264338328);
(setq flteps 1.1920928955078125e-07);
(setq delta (sqrt flteps))

(setq eps (list flteps))
(setq zero (list 0.0))
(setq simple (list 0.5  1.0  2.0))
(setq inf  (list (/ 1.0 flteps)))
(setq finite (append eps simple))
(setq zfinite (append zero eps simple))

(setq realpos (list flteps delta  0.125 0.5 0.75  1.0  2.0  10.0))
(setq real (append (reverse (mapcar '- realpos)) 
                   zero 
                   realpos))

(setq circ (list (- 0 delta)
                 (+ 0 delta)
                 (- (* 0.5 pi) delta)
                 (+ (* 0.5 pi) delta)
                 (- pi delta)
                 (+ pi delta)
                 (- (* 1.5 pi) delta)
                 (+ (* 1.5 pi) delta)
                 (- (* 2 pi) delta)
                 (+ (* 2 pi) delta)
                 (- (* 3 pi) delta)
                 (+ (* 3 pi) delta)))

(setq trig (list (+ (sqrt pi) delta)
                 (+ (log pi) delta)
                 (- (sqrt pi) delta)
                 (- (log pi) delta)))

(setq z0 (combine (append zero eps simple inf)
                  (append zero eps simple inf)))


(setq z1 (append (combine (append eps simple inf)
                          (append zero eps simple inf))
                 (combine (append zero eps simple inf)
                          (append eps simple inf))))


;(setq z2 (append (combine simple real) 
;                 (combine edge (append edge simple) )))

(setq zcirc (append (combine circ zfinite)))
(setq zicirc (append (combine zfinite circ)))

(defun test1 ()
  (test "arg" z0)
  (test "abs" z0)
  (test "abs2" z0)
  (test "logabs" z1)
)

(defun test2 ()
  (test "sqrt" z0)

  (test "log" z1)
  (test "log10" z1)
  (test "exp" zicirc)

  (test "sin" zcirc)
  (test "cos" zcirc)
  (test "tan" zcirc)

  (test "arcsin" z0)
  (test "arccos" z0)
  (test "arctan" z0)

  (test "sinh" zicirc)
  (test "cosh" zicirc)
  (test "tanh" zicirc)

  (test "arcsinh" z0)
  (test "arccosh" z0)
  (test "arctanh" z0)

  (test "csc" zcirc)
  (test "sec" zcirc)
  (test "cot" zcirc)

  (test "arccsc" z0)
  (test "arcsec" z0)
  (test "arccot" z0)

  (test "csch" zicirc)
  (test "sech" zicirc)
  (test "coth" zicirc)

  (test "arccsch" z0)
  (test "arcsech" z0)
  (test "arccoth" z0)
)

(defun test3 ()
  (test "sqrt_real" real)

  (test "arcsin_real" real)
  (test "arccos_real" real)

  (test "arccosh_real" real)
  (test "arctanh_real" real)

  (test "arccsc_real" real)
  (test "arcsec_real" real)
)


;;(test1)
;;(test-all)
;;(test3)

