#lang racket

;(define state '((A #t) (B #t) (C #t) (D #f)))
;(define clause '(A (not B) C))
;(define clauses '((A (not B) C) (A B (not C)) ((not A) (not B) D)))

;;1: returns the boolean value associated with a variable, or errors out if the variable is not present in state
(define eval-var (lambda (var state) 
                  (if (pair? var) 
                     (cond ;not statement
                              [(member (cons (car(cdr var)) (cons #t '())) state) '#f] ;second element of pair is true, return false
                              [(member (cons (car(cdr var)) (cons #f '())) state) '#t] ;second element of pair is false, return true
                              [else (error "eval-var: VAR NOT FOUND")] ;DNE
                     )
                     (cond ;single variable
                              [(member (cons var (cons #t '())) state) '#t] ;element is true
                              [(member (cons var (cons #f '())) state) '#f] ;element is false
                              [else (error "eval-var: VAR NOT FOUND")]
                     )
                   )
))

;;2: evaluates the truth value of one clause
(define eval-clause (lambda (clause state)
                      (cond ((eval-var (car clause) state)) ;if first arg is #t, returns #t
                      (else (cond ((eval-var (car(cdr clause)) state));if second arg is #t, returns #t
                      (else (cond ((eval-var (car(cdr(cdr clause))) state));if third arg is #t, returns #t
                      (else #f) ;all args are #f, return #f
                      )))))))

;;3: returns a list of all variables in one clause
(define get-vars (lambda (clause)
                   (cons (if (list? (car clause)) (car(cdr(car clause))) (car clause) ) ;checks if statement is a not statement, removes not if present
                   (cons (if (list? (car(cdr clause))) (car(cdr(car(cdr clause)))) (car(cdr clause))) ;creates list of all three elements of the clause
                   (cons (if (list? (car(cdr(cdr clause)))) (car(cdr(car(cdr(cdr clause))))) (car(cdr(cdr clause)))) '());and returns it
                   ))
))

;;4: combines the get-vars of car clauses with a recursive call on cdr clauses, unless if clauses is empty then return an empty list
(define get-all-vars (lambda (clauses)
          (remove-duplicates (flatten ;flattens list and then removes all duplicate variables
              (if (pair? clauses) ;checks if clauses is a pair. a single clause is still a pair
                  (cons (get-vars(car clauses)) (get-all-vars(cdr clauses))) ;returns concat of the current clause's vars and a recursive call on the rest of the clauses
                  '();not a pair, return empty list so all the cons can collapse
              )
          ))
))

;;5: if unsat-clauses gets an empty list, return it
;;else if the head of clauses is false, combine with a recursive call on the cdr of clauses, else just return the recursive call on the cdr
(define unsat-clauses (lambda (clauses state)
                        (if (pair? clauses) ;checks if clauses is a pair. a single clause is still a pair
                            (if (not (eval-clause (car clauses) state)) ;if eval-clause returns false
                                  (cons (car clauses) (unsat-clauses (cdr clauses) state)); returns concat this clause with a recursive call on remaining clauses
                                  (unsat-clauses (cdr clauses) state);returns recursive call on remaining clauses
                            )
                            '();not a pair, return empty list so all the cons can collapse
                        )
))

;;6: flips the truth value of a specific variable in a state list
(define flip-var (lambda(var state)
              (if (pair? state);checks for end of list
                   (cond ;checks if current variable in state equals var
                     [(equal? (cons var (cons #t '())) (car state)) (cons (cons var (cons #f '())) (cdr state))] ;variable was true
                     [(equal? (cons var (cons #f '())) (car state)) (cons (cons var (cons #t '())) (cdr state))] ;variable was false
                     [else (cons (car state) (flip-var var (cdr state)))] ;not the right variable. check next variable by calling self on cdr state
                   )
                  (error "flip-var: VAR NOT FOUND");hit end of state without finding var
              )
))

;;7: flips each var, checks if it is a better neighbor, returns the state with the flipped var if it is, otherwise calls itself replacing vars with (cdr vars)
;;if there is no better neighbor, gives an error.
;(get-better-neighbor clauses state (get-all-vars clauses) (get-num-unsat clauses state))
(define get-better-neighbor (lambda (clauses state vars num-unsat)
                              (if (pair? vars) ;there are still vars to check their flipped value
                                  (if (< (get-num-unsat clauses (flip-var (car vars) state)) num-unsat);checks if flipping current var yields less unsat clauses
                                      (flip-var (car vars) state);returns state with the flipped var
                                      (get-better-neighbor clauses state (cdr vars) num-unsat);calls self on remainder of vars
                                  )
                                  (error "get-better-neighbor: better neighbor DNE"); no better neighbor
                              
                              )
))

;;counts the result of unsat-clauses
(define get-num-unsat (lambda (clauses state)
                        (count (unsat-clauses clauses state) 0);calls count on the result of unsat-clauses
))

;;counts how many elements are in a list
;(count list 0)
(define count (lambda (list num)
                (if (pair? list); a single element '(var) is still a pair of 'var & '()
                    (count (cdr list) (+ num 1)); return recursively call on the rest of the list and the incremented count
                    num;;return the count
                )
))

;;8: if state is not a solution, recursively check the better neighbor of state until it is, and then return the state that is a solution
;(simple-hill-climb clauses state (* (get-num-vars clauses)  (get-num-vars clauses)) (unsat-clauses clauses state))
(define simple-hill-climb (lambda (clauses state dist unsat)
                            (if (pair? unsat) ;if unsat is not empty 
                                (simple-hill-climb ;returns recursive call on clauses, a better neighbor, dist - 1, and the num of unsat clauses of the better neighbor
                                                   clauses 
                                                   (get-better-neighbor clauses state (get-all-vars clauses) (get-num-unsat clauses state)) 
                                                   (- dist 1) 
                                                   (unsat-clauses clauses (get-better-neighbor clauses state (get-all-vars clauses) (get-num-unsat clauses state)))
                                )
                                
                                state;all clauses are satisfied, return state
                            )
))
;;counts the result of get-all-vars
(define get-num-vars (lambda (clauses)
                        (count (get-all-vars clauses) 0);calls count on the result of get-all-vars
))