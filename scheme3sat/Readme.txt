Chris Stickney
c.stickney117@gmail.com
This program finds solutions to 3-sat by testing how many clauses a state fails. If it fails 0, then it returns the state. Otherwise it finds a state that fails less and tests again. If the solution does not exist then the program gives an error message.

The hill climb function takes a list of clauses, a list of states, the number of unique states possible for the variable set, and the list of clauses that the states fail on. If all variables are assigned, the program can be quickly run with the following command:

(simple-hill-climb clauses state (* (get-num-vars clauses)  (get-num-vars clauses)) (unsat-clauses clauses state))

state, clause and clauses have been commented out, so reenable mine or define your own to run the functions.