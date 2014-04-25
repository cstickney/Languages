%TODO: done for now
%		eval-var(a,[[a,true],[b,false]], X).	
%1. eval-var(VAR,STATES,RETURN).
 
eval-var(_, [], [not|found]):- 			 					%var does not exist
	print('variable not found')								%returns a non-atom to fail atomic checks
.
eval-var(Var, [[HH|[HT|_]]|T], Return):- 					%evaluates Var
	Var == HH							 					%if var match
		->Return = HT									 	%return truth value
	;eval-var(Var, T, Return)								%else check next var
.


%TODO: done for now
%		eval-clause([[a,true],[b,false],[c,true],[d,false]],[a,[not,b],c],X).
%2. eval-clause(STATES,CLAUSE,VALUE).

eval-clause(_, [], false).									%no more statements, eval false

eval-clause(States, [[_|[H|_]]|T],Return):- 				%next is a not statement
	eval-var(H,States, ReturnB), 							%get value of stmt var
	ReturnB==false											%if statement is true
		->Return = true										%return true
	;eval-clause(States,T, Return)							%else check tail for true
.

eval-clause(States, [H|T],Return):-							%next is a regular stmt
	atomic(H) ->(											%ensure H is a variable, not a not stmt
	eval-var(H,States, ReturnB), 							%get value of stmt var
	ReturnB==true											%if statement is true
		->Return = true										%return true
	;eval-clause(States,T, Return)      )					%else check tail for true
.

%TODO: done for now
%		get-vars([a,[not,b],c],X).
%3. get-vars(CLAUSE,VALUE).

get-vars(Clause, Return):-									%calls first iter of getvars
	get-vars(Clause, [], Return)
.
get-vars([], Current, Return):-								%done. sets return list
	reverse(Current,X,[]),									%reverses list of variables to original order (unnecessary)
	Return = X												%returns reversed list
.
get-vars([[_|[H|_]]|T], Current, Return):-					%not statement
	get-vars(T, [H|Current], Return)						%appends var list to var H, calls get-vars on tail
.
get-vars([H|T], Current, Return):-							%normal statement
	atomic(H)												%checks that H is a string, not a not stmt
	->get-vars(T, [H|Current], Return)						%appends var list to var H, calls get-vars on tail
.

reverse([],Z,Z). 											%empty list base case
reverse([H|T],Z,Acc) :- reverse(T,Z,[H|Acc]).				%reverses a depth 1 list



%TODO: done for now
%		get-all-vars([[a,[not,b],c],[c,[not,d],e],[a,[not,e],f]],X).
%4. get-all-vars(CLAUSES,VALUE).

get-all-vars(Clauses, Return):-								%calls first iter of getallvars
	get-all-vars(Clauses, [], Return)
.
get-all-vars([], Current, Return):-							%No more clauses, fix list and return it
	combine(Current, [], Returnb),							%combines individual clause lists into one
	remove-duplicates(Returnb, [],Returnc),					%removes all duplicates (and scrambles the list for some reason but ehhhhhh... Still technically 														 right.)
	Return = Returnc										%returns the list of variables
.
get-all-vars([H|T], Current, Return):-						%gets clause lists of variables
	get-vars(H,Vars),										%gets vars for clause
	get-all-vars(T,[Vars|Current],Return)					%appends current to variable list and calls getallvars again
.

combine([[HHH|[HTH|[HTT|[]]]]|T],Current, Return):-			%combines variables from clause lists
	combine(T, [HHH|[HTH|[HTT|Current]]], Return)			%combines the head clause variables with current and calls combine again
.
combine([], Return, Return).								%all variables are combined, return them.

remove-duplicates([H|T], Current, Return):-					%removes duplicates of each element until end of list
	remove-element(H, T,[],Tnew),							%removes duplicates of head of list in tail
	remove-duplicates(Tnew,[H|Current],Return)				%recursive call prepending H to current, using a modified T with no dups of Var
.
remove-duplicates([], Return, Return).						%return the duplicate-free list

remove-element(Var, [H|T], Current, Return):-				%removes duplicates of var from a list
	Var == H												%variable match
		-> remove-element(Var, T, Current, Return)			%recursive call on tail. Head is discarded
	;remove-element(Var,T,[H|Current],Return)				%recursive call on tail, Head is prepended to Current
.
remove-element(_, [], Return, Return).						%return the list without duplicates of Var

%TODO: done for now
%		unsat-clauses([[a,[not,b],c],[a,[not,b],[not,c]],[a,[not,b],d]],[[a,false],[b,true],[c,true],[d,false]],X).
%5. unsat-clauses(CLAUSES,STATES,VALUE).

unsat-clauses(Clauses, States, Return):-					%calls first iter of unsat-clauses
	unsat-clauses(Clauses, States, [], Return)
.
unsat-clauses([],_,Return,Return).							%no more clauses, return all unsatisfied clauses
unsat-clauses([H|T],States, Current, Return):-				%checks each clause for truth, and adds to list if false
	eval-clause(States, H, Truth), 							%checks current clause for truth
	Truth == 'false'										%if not true
		->unsat-clauses(T, States, [H|Current], Return)		%prepend current clause to current,and recurse on tail.
	;unsat-clauses(T, States, Current, Return)				%else recurse on tail, discard head
.

%TODO: done for now
%		flip-var(d,[[a,false],[b,true],[c,true],[d,false]],X).
%6. flip-var(VAR,STATES,VALUE).

flip-var(Var,[[HH|[HT|[]]]|T],Return):-     						%first iter
	HH == Var														%Variable match
		-> (HT == 'true'											%true match
			->Return = [[HH|['false'|[]]]|T]						%toggle to false, and append tail to new state
		; HT =='false'												%false match
			->Return = [[HH|['true'|[]]]|T]							%toggle to true,and append tail to new state
	);flip-var(Var, [[HH|[HT|[]]]], T, Return) %Variable mismatch	%variable mismatch, check tail of list
.
flip-var(Var, Current, [[HH|[HT|[]]]|T], Return):-							%2nd+ iter
	HH==Var																	%variable match
		-> (HT=='true'														%true match
			->flip-var([im|done], [[HH|['false'|[]]]|Current], T, Returnb), %toggle to true, append current to new state, continue building current until end
			reverseb(Returnb, Return, [])	%restore list order				%restore list order
		;HT=='false'														%false match
			->flip-var([im|done], [[HH|['true'|[]]]|Current],T,Returnb),	%toggle to true, append current to new state, continue building current until end
			reverseb(Returnb, Return, [])									%restore list order
	);flip-var(Var, [[HH|[HT|[]]]|Current], T, Return)%Variable mismatch	%variable mismatch, check tail of list
.
flip-var(Var, Return, [], Return):-											%end of state list
	not(atomic(Var))														%true if variable was found and toggled, else [im|done] isnt atomic so it fails
.		


reverseb([[HH|HT]|T],Z,Acc):- 								%reverses lists of pairs
	reverseb(T,Z,[[HH|HT]|Acc])								%recursive call on tail, prepends to Acc
.
reverseb([HH|[HT|_]],Z,[]):- 								%only one element
	atomic(HH)												%prevents from tunneling into a pair
		-> Z = [HH|[HT|[]]]									%returns the singleton
.						
reverseb([],Z,Z).											%input list is empty, return the reversed list