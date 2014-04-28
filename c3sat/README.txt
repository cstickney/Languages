Chris Stickney
c.stickney117@gmail.com
This project replicates the functionality of lists in Scheme, and is capable of solving 3sat problems in roughly the same way as in assignment 2.

compile the 3sat program with "make threesat" 
compile the listaxioms program with "make listaxioms"

3sat.c
list.c
list.h
listaxioms.c
Makefile
README.txt
clauses.txt
state.txt

type list(element) imports boolean, int
operations:
	createList: list
	isEmpty: list -> boolean
	prepend: list x elem -> list
	append: list x elem
	head: list -> elem
	tail: list -> list
	length: list x index x elem -> int
	insert: list x index -> list
	remove: list x index -> list
	retrieve: list x index -> elem

variables: l: list; e: element; i: index

axioms:
	isEmpty(createList) = true
	isEmpty(prepend(l,e)) = false
	head(createList) = error
	head(append(l,e)) = if isEmpty(l) then e, else head(l)
	tail(createList) = error
	tail(prepend(l,e)) = if isEmpty(l) then createList, else tail(l)
	insert(l,i,e) = if i > length(l) + 1 or i<0 then error, 	else insert(l,i,e)
	remove(l,i) = if i>length(l) or i < 0 then error, 	else remove(l,i)
	retrieve(l,i) = if i>length(l) or i < 0 then error,	 else retrieve(l,i)
	
	
	Scheme is useful for a few reasons. As it does not depend on the state of a program, it consistently reproduces the same results, given the same inputs. However, this limits how much Scheme can actually do, as it cannot produce side effects, so it cannot perform certain tasks, and may have a much harder time with others. Since it is a functional programming language, it excels at solving any computation that can be represented by a function. Scheme should be useful for programmers or mathematicians who need to solve problems that can be turned into functions.
	
	Prolog has a few advantages over Scheme. Prolog’s built in list operations make parsing lists a simple task. In addition, it is capable of looping, which Scheme cannot do. Its greatest strength is that it is built to allow nondeterministic evaluation. However, its primary use seems to be for querying data, so its usefulness for other programming tasks may be limited. It seems to be most useful for academic use, for its mathematical applications. Scheme should be useful for programmers or mathematicians who need to solve problems where nondeterminism is useful.
	
		C is a very useful language, with a variety of benefits. Its primary benefit is that, as a procedural language, it can perform almost any task that is needed. Its other main benefit is that it is more closely related to machine code than most other languages, allowing it to perform efficient computations. Its primary downside is that , while it can perform a multitude of tasks, it may take more work on the part of the programmer than other specialized languages would. Still, its versatility makes it a useful tool for nearly all programming tasks. C is a useful language for all programmers that need to create programs to solve tasks.