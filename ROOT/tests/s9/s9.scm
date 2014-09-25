;;
;; Scheme 9 from Empty Space
;; Copyright (C) 2007 Nils M Holm <nmh@t3x.org>
;;

;;----- Library -----

(define (not x) (eq? #f x))

(define number? integer?)

(define (port? x)
  (or (input-port? x)
      (output-port? x)))

(define (eqv? a b)
  (cond
    ((number? a)
      (and (number? b) (= a b)))
    ((char? a)
      (and (char? b) (char=? a b)))
    (else (eq? a b))))

(define (equal? a b)
  (cond
    ((eq? a b) #t)
    ((string? a)
      (and (string? b) (string=? a b)))
    ((and (pair? a) (pair? b))
      (and (equal? (car a) (car b))
           (equal? (cdr a) (cdr b))))
    ((and (vector? a) (vector? b))
      (equal? (vector->list a) (vector->list b)))
    (else (eqv? a b))))

(define (null? x)
  (eq? '() x))

(define (list? x)
  (or (null? x)
      (and (pair? x)
           (list? (cdr x)))))

(define (caaaar x) (car (car (car (car x)))))
(define (caaadr x) (car (car (car (cdr x)))))
(define (caadar x) (car (car (cdr (car x)))))
(define (caaddr x) (car (car (cdr (cdr x)))))
(define (cadaar x) (car (cdr (car (car x)))))
(define (cadadr x) (car (cdr (car (cdr x)))))
(define (caddar x) (car (cdr (cdr (car x)))))
(define (cadddr x) (car (cdr (cdr (cdr x)))))
(define (cdaaar x) (cdr (car (car (car x)))))
(define (cdaadr x) (cdr (car (car (cdr x)))))
(define (cdadar x) (cdr (car (cdr (car x)))))
(define (cdaddr x) (cdr (car (cdr (cdr x)))))
(define (cddaar x) (cdr (cdr (car (car x)))))
(define (cddadr x) (cdr (cdr (car (cdr x)))))
(define (cdddar x) (cdr (cdr (cdr (car x)))))
(define (cddddr x) (cdr (cdr (cdr (cdr x)))))

(define (caaar x) (car (car (car x))))
(define (caadr x) (car (car (cdr x))))
(define (cadar x) (car (cdr (car x))))
(define (caddr x) (car (cdr (cdr x))))
(define (cdaar x) (cdr (car (car x))))
(define (cdadr x) (cdr (car (cdr x))))
(define (cddar x) (cdr (cdr (car x))))
(define (cdddr x) (cdr (cdr (cdr x))))

(define (caar x) (car (car x)))
(define (cadr x) (car (cdr x)))
(define (cdar x) (cdr (car x)))
(define (cddr x) (cdr (cdr x)))

(define (assoc x a)
  (cond ((null? a) #f)
    ((equal? (caar a) x) (car a))
    (else (assoc x (cdr a)))))

(define (assq x a)
  (cond ((null? a) #f)
    ((eq? (caar a) x) (car a))
    (else (assq x (cdr a)))))

(define (assv x a)
  (cond ((null? a) #f)
    ((eqv? (caar a) x) (car a))
    (else (assv x (cdr a)))))

(define (map-car f a)
  (letrec
    ((mapc
       (lambda (a r)
         (cond ((null? a) (reverse r))
           (else (mapc (cdr a)
                       (cons (f (car a)) r)))))))
    (mapc a '())))

(define fold-left
  (let ((map-car map-car))
    (lambda (f b . a*)
      (letrec
        ((carof
           (lambda (a)
             (map-car car a)))
         (cdrof
           (lambda (a)
             (map-car cdr a)))
         (fold
           (lambda (a* r)
             (cond ((null? (car a*)) r)
               (else (fold (cdrof a*)
                       (apply f r (carof a*))))))))
        (cond
          ((null? a*)
            (wrong "fold-left: too few arguments"))
          ((null? (car a*)) b)
          (else (fold a* b)))))))

(define fold-right
  (let ((map-car map-car))
    (lambda (f b . a*)
      (letrec
        ((carof
           (lambda (a)
             (map-car car a)))
         (cdrof
           (lambda (a)
             (map-car cdr a)))
         (foldr
           (lambda (a* r)
             (cond ((null? (car a*)) r)
               (else (foldr (cdrof a*)
                       (apply f (append (carof a*) (list r)))))))))
        (cond
          ((null? a*)
            (wrong "fold-right: too few arguments"))
          ((null? (car a*)) b)
          (else (foldr (map reverse a*) b)))))))

(define (reverse a)
  (letrec
    ((reverse2
       (lambda (a b)
         (cond ((null? a) b)
           (else (reverse2 (cdr a)
                   (cons (car a) b)))))))
    (reverse2 a '())))

(define append
  (let ((fold-left fold-left))
    (lambda a
      (letrec
        ((append2
           (lambda (a b)
             (cond ((null? a) b)
               (else (append2 (cdr a) (cons (car a) b))))))
         (append-wrapper
           (lambda (a b)
             (cond ((null? b) a)
               (else (append2 (reverse a) b))))))
        (fold-left append-wrapper '() a)))))

(define (length x)
  (letrec
    ((length2
       (lambda (x r)
         (cond ((null? x) r)
           (else (length2 (cdr x) (+ r 1)))))))
    (length2 x 0)))

(define (list . x) x)

(define (list-ref x n)
  (car (list-tail x n)))

(define (list-tail x n)
  (cond ((zero? n) x)
    ((null? x) (wrong "list-tail: index out of range"))
    (else (list-tail (cdr x) (- n 1)))))

(define (member x a)
  (cond ((null? a) #f)
    ((equal? (car a) x) a)
    (else (member x (cdr a)))))

(define (memq x a)
  (cond ((null? a) #f)
    ((eq? (car a) x) a)
    (else (memq x (cdr a)))))

(define (memv x a)
  (cond ((null? a) #f)
    ((eqv? (car a) x) a)
    (else (memv x (cdr a)))))

(define map
  (let ((map-car map-car))
    (lambda (f . a*)
      (letrec
        ((carof
           (lambda (a)
             (map-car car a)))
         (cdrof
           (lambda (a)
             (map-car cdr a)))
         (map2
           (lambda (a* r)
             (cond ((null? (car a*)) (reverse r))
               (else (map2 (cdrof a*)
                     (cons (apply f (carof a*)) r)))))))
        (cond
          ((null? a*)
            (wrong "map: too few arguments"))
          (else (map2 a* '())))))))

(define (for-each f . a*)
  (cond
    ((null? a*)
      (wrong "for-each: too few arguments"))
    (else (apply map f a*)
          (if #f #f))))

(define (abs x) (if (< x 0) (- x) x))

(define (even? x) (zero? (remainder x 2)))

(define (expt x y)
  (letrec
    ((square
       (lambda (x)
         (* x x)))
     (expt2
       (lambda (x y)
         (cond
           ((zero? y) 1)
           ((even? y)
             (square (expt2 x (quotient y 2))))
           (else (* x (square (expt2 x (quotient y 2)))))))))
    (if (negative? y)
        (wrong "expt: negative exponent" y)
        (expt2 x y))))

(define gcd
  (let ((fold-left fold-left))
    (lambda a
      (letrec
        ((gcd2
           (lambda (a b)
             (cond ((zero? b) a)
               ((zero? a) b)
               ((< a b) (gcd2 a (remainder b a)))
               (else (gcd2 b (remainder a b)))))))
        (fold-left gcd2 0 (map (lambda (x) (abs x))
                               a))))))

(define lcm
  (let ((fold-left fold-left))
    (lambda a
      (letrec
        ((lcm2
           (lambda (a b)
             (let ((cd (gcd a b)))
               (* cd (* (quotient a cd)
                        (quotient b cd)))))))
        (fold-left lcm2 1 (map (lambda (x) (abs x))
                          a))))))

(define max
  (let ((fold-left fold-left))
    (lambda (a . b)
      (fold-left (lambda (a b)
                   (if (> a b) a b))
                 a b))))

(define min
  (let ((fold-left fold-left))
    (lambda (a . b)
      (fold-left (lambda (a b)
                   (if (< a b) a b))
                 a b))))

(define (modulo a b)
  (let ((rem (remainder a b)))
    (cond ((zero? rem) 0)
      ((eq? (negative? a) (negative? b)) rem)
      (else (+ b rem)))))

(define (negative? x) (< x 0))

(define (odd? x) (not (even? x)))

(define (positive? x) (> x 0))

(define (sqrt square)
  (letrec
    ((sqrt2 (lambda (x last)
       (cond
         ((= last x) x)
         ((= last (+ 1 x))
           (if (> (* x x) square) (- x 1) x))
         (else (sqrt2 (quotient
                        (+ x (quotient square x))
                        2)
                      x))))))
    (if (negative? square)
        (wrong "sqrt: negative argument" square)
        (sqrt2 square 0))))

(define (zero? x) (= 0 x))

(define (number->string n . radix)
  (letrec
    ((digits
       (list->vector (string->list "0123456789abcdef")))
     (conv
       (lambda (n rdx res)
         (cond ((zero? n) res)
           (else (conv (quotient n rdx) rdx
                   (cons (vector-ref digits (remainder n rdx))
                         res))))))
     (get-radix
       (lambda ()
         (cond ((null? radix) 10)
           ((< 1 (car radix) 17) (car radix))
           (else (wrong "bad radix in number->string" radix))))))
    (let ((r (get-radix)))
      (cond
        ((zero? n) "0")
        ((negative? n)
          (list->string
            (cons #\- (conv (abs n) r '()))))
        (else (list->string (conv n r '())))))))

(define (string . x) (list->string x))

(define (string->number str . radix)
  (letrec
    ((digits
       (string->list "0123456789abcdef"))
     (value-of-digit
       (lambda (x)
         (letrec
           ((v (lambda (x d n)
             (cond ((null? d) 17)
               ((char=? (car d) x) n)
               (else (v x (cdr d) (+ n 1)))))))
           (v (char-downcase x) digits 0))))
     (conv3
       (lambda (lst res rdx)
         (cond ((null? lst) res)
           (else (let ((dval (value-of-digit (car lst))))
                   (and (< dval rdx)
                        (conv3 (cdr lst)
                          (+ (value-of-digit (car lst))
                             (* res rdx))
                          rdx)))))))
     (conv
       (lambda (lst rdx)
         (if (null? lst) #f (conv3 lst 0 rdx))))
     (sconv
       (lambda (lst rdx)
         (cond
           ((null? lst) #f)
           ((char=? (car lst) #\+)
             (conv (cdr lst) rdx))
           ((char=? (car lst) #\-)
             (let ((r (conv (cdr lst) rdx)))
               (if r (- r) #f)))
           (else (conv lst rdx)))))
     (get-radix
       (lambda ()
         (cond ((null? radix) 10)
           ((< 1 (car radix) 17) (car radix))
           (else (wrong "bad radix in string->number" radix))))))
    (sconv (string->list str) (get-radix))))

(define (vector . x) (list->vector x))

(define (newline . port)
  (apply display #\newline port))

(define (call-with-input-file file proc)
  (proc (open-input-file file)))

(define (call-with-output-file file proc)
  (proc (open-output-file file)))

(define with-input-from-file
  (let ((set-input-port! set-input-port!))
    (lambda (file thunk)
      (let ((outer-port (current-input-port))
            (new-port (open-input-file file)))
        (set-input-port! new-port)
        (let ((input (thunk)))
          (close-input-port new-port)
          (set-input-port! outer-port)
          input)))))

(define with-output-to-file
  (let ((set-output-port! set-output-port!))
    (lambda (file thunk)
      (let ((outer-port (current-output-port))
            (new-port (open-output-file file)))
        (set-output-port! new-port)
        (thunk)
        (close-output-port new-port)
        (set-output-port! outer-port)))))

;;----- Syntax-Expander -----

(define (print x . x*)
  (letrec
    ((p (lambda (x*)
          (cond
            ((not (null? x*))
              (display #\space)
              (write (car x*))
              (p (cdr x*)))))))
    (write x)
    (p x*)
    (newline)))

(define (expand-syntax form)
  (letrec
    ((ext-env 
       (lambda (x v env)
         (cons (cons x v) env)))
     (match-ellipsis
       ; This function uses a longest match first approach, so
       ; (match-ellipsis '(x x k k k) '(k) '(k) ())
       ;   => ((... . (x x k k)))
       (lambda (form pattern literals env)
         (letrec
           ((try-match (lambda (head tail)
             (let ((v (match tail pattern literals env)))
               (cond (v (ext-env '... (reverse head) v))
                 ((null? head) #f)
                 (else (try-match (cdr head)
                                  (cons (car head) tail))))))))
           (try-match (reverse form) ()))))
     (match
       ; Match form against pattern.
       ; Return an environment with bindings of
       ; pattern variables to subforms or #F in
       ; case form does not match pattern.
       (lambda (form pattern literals env)
         (letrec
           ((_match (lambda (form pattern env)
             (cond
               ((memq pattern literals)
                 (if (eq? form pattern) env #f))
               ((and (pair? pattern) (eq? (car pattern) '...))
                 (match-ellipsis form (cdr pattern) literals env))
               ((symbol? pattern)
                 (ext-env pattern form env))
               ((and (pair? pattern) (pair? form))
                 (let ((e (_match (car form) (car pattern) env)))
                   (and e (_match (cdr form) (cdr pattern) e))))
               (else (if (equal? form pattern) env #f))))))
           (_match form pattern env))))
     (find-rule
       (lambda (form rules name literals)
         (cond
           ((null? rules)
             (wrong "bad syntax" name))
           (else (let ((e (match form (caar rules) literals '())))
                   (if e (list (caar rules) (cadar rules) e)
                         (find-rule form (cdr rules) name literals)))))))
     (map-improper
       (lambda (f a)
         (letrec
           ((map-i
              (lambda (a r)
                (cond ((null? a) (reverse r))
                  ((not (pair? a)) (append (reverse r) (f a)))
                  (else (map-i (cdr a) (cons (f (car a)) r)))))))
           (map-i a '()))))
     (subst-ellipsis
       (lambda (var tmpl val* env)
         (map (lambda (v)
                (tmpl->form #f tmpl (cons (cons var v) env)))
              val*)))
     (tmpl->form
       ; Substitute variables of env by values of env in form.
       (lambda (pattern form env)
         (cond
           ((not (pair? form))
             (let ((v (assv form env)))
               (if v (cdr v) form)))
           ((and (pair? form)
                 (pair? (cdr form))
                 (eq? (cadr form) '...))
             (let ((var (if (pair? pattern) (car pattern) pattern)))
               (let ((v-ell (assq '... env))
                     (v-var (assq var env)))
                 (if v-ell
                     (if v-var
                         (append (subst-ellipsis
                                   var
                                   (car form)
                                   (if v-var
                                       (cons (cdr v-var) (cdr v-ell))
                                       (cdr v-ell))
                                   env)
                                 (cddr form))
                         (append (list (tmpl->form #f (car form) env))
                                 (cdr v-ell)
                                 (cddr form)))
                     (wrong "unmatched ... in syntax-rules")))))
           ((pair? form)
             (cons (tmpl->form (if (pair? pattern)
                                   (car pattern)
                                   #f)
                               (car form)
                               env)
                   (tmpl->form (if (pair? pattern)
                                   (cdr pattern)
                                   #f)
                               (cdr form)
                               env)))
           (else form))))
     (transform
       (lambda (form)
         (let ((syn (syntax->list (car form))))
           (if (not syn)
               (wrong "not a syntax transformer" (car form))
               (let ((name (car form))
                     (literals (car syn))
                     (rules (cdr syn)))
                 (let ((pat/tmpl/env (find-rule form rules name literals)))
                   (expand-all
                     (apply tmpl->form pat/tmpl/env))))))))
     (expand-all
       (lambda (form)
         (cond
           ((not (pair? form)) form)
           ((eq? (car form) 'quote) form)
           ((syntax->list (car form))
             (transform form))
           (else (map-improper expand-all form))))))
    (expand-all form)))

;;----- Quasi-quote-expander -----

(define (expand-quasiquote form)
  (letrec
    ((does-splicing?
       (lambda (form)
         (cond
           ((not (pair? form)) #f)
           ((and (pair? (car form))
                 (eq? 'unquote-splicing (caar form)))
             #t)
           (else (does-splicing? (cdr form))))))
     (qq-list
       (lambda (form)
         (if (does-splicing? form)
             (cons 'append
                   (map (lambda (x)
                          (if (and (pair? x)
                                   (eq? 'unquote-splicing (car x)))
                              (cadr x)
                              (list 'list (expand-qq x))))
                        form))
             (cons 'list (map expand-qq form)))))
     (expand-qq
       (lambda (form)
         (cond
           ((vector? form)
             (list 'list->vector (qq-list (vector->list form))))
           ((not (pair? form)) (list 'quote form))
           ((eq? 'quasiquote (car form)) (expand-qq (cadr form)))
           ((eq? 'unquote (car form)) (cadr form))
           (else (qq-list form))))))
    (expand-qq (cadr form))))

;;----- Library -----

(define-syntax case
  (syntax-rules (else)
    ((_ key (else expr . rest))
       (begin expr . rest))
    ((_ key (data expr . rest))
       (if (memv key 'data) 
           (begin expr . rest)
           (if #f #f)))
    ((_ key (data1 expr1 . rest1) more-cases ...)
       (if (memv key 'data1)
           (begin expr1 . rest1)
           (case key more-cases ...)))))

(define-syntax let*
  (syntax-rules ()
    ((_ () x . rest)
       (let () x . rest))
    ((_ ((n v)) x . rest)
       (let ((n v)) x . rest))
    ((_ ((n1 v1) (n2 v2) ...) x . rest)
       (let ((n1 v1))
         (let* ((n2 v2) ...) x . rest)))))

(define-syntax delay
  (syntax-rules ()
    ((_ form)
      (let ((%%r #f))
        (lambda ()
          (cond (%%r (car %%r))
            (else (set! %%r (cons form '()))
                  (car %%r))))))))

(define (force x) (x))
