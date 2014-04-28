#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <assert.h>
#include "list.h"
#define TRUE 1
#define FALSE 0
#define stateLen 71//char count of the state file
#define clausesLen 1397//char count of the clause file

typedef struct lelem {//holds lists or symbols
	int type;  //1: list, 2:symbol, 3: truth value, 4: not
	union {
		list* l;      //a list
		char c;    //a state, truth value, or not symbol
	} elem;
} lelem;

void printList(list* l);//prints a list of type lelem
list* textToList(char* text);//converts a char array to a list of type lelem
list* deepCopy(list* original);//creates a copy of a list of type lelem
//void freeList(list* l);
int evalVar(char var, list* states);//evaluates a char variable
int evalClause(list* clause, list* states);//evaluates a clause
list* getVars(list* clause);//gets all the vars from a clause
list* getAllVars(list* clauses);//get all the vars in the clause list
int findVar(list* allVars, char state);//finds if a var exists in a variable list. returns 1 if not present, else 0
list* unsatClauses(list* clauses, list* states);//returns the unsatisfied clauses in a clause list for a set of states
list* flipVar(char var, list* states);//flips a specified variable
list* getBetterNeighbor(list* clauses, list* states, list* vars, int numUnsat);//flips variables until one set of states has less unsat
list* simpleHillClimb(list* clauses, list* states, int dist, list* unsat);//finds the best states for a list of clauses

int main() {
	FILE *ptr_file;
	char st[stateLen+1];//state char array
	char cl[clausesLen+1];//clause char array
	int x;

	ptr_file =fopen("state.txt","r");
	if(!ptr_file) return 1;
	while(fgets(st,stateLen+1, ptr_file)!=NULL);//reads the state file
	fclose(ptr_file);

	ptr_file =fopen("clauses.txt","r");
	if (!ptr_file) return 1;
	while (fgets(cl,clausesLen+1, ptr_file)!=NULL);//reads the clause file
	fclose(ptr_file);

	list* states;
	states = textToList(st);//makes a state list
	list* clauses;
	clauses = textToList(cl);//makes a clause list

	printf("list text:\n");//prints the states and clauses that were listed
	printList(states);
	printf("\n\n");
	printList(clauses);
	printf("\n\n");

	printf("evalVar:\n");//evaluates the first non-not variable found in the supplied clauses file.
	char var = ((lelem*)((lelem*)clauses->front->next->elem)->elem.l->front->next->elem)->elem.c;
	x= evalVar(var, states);
	printf("%c = ",var);
	if(x==1) printf(" TRUE\n\n");
	else printf(" FALSE\n\n");

	printf("evalClause:\n");//evaluates the first clause of the clauses list
	x=evalClause(((lelem*)clauses->front->elem)->elem.l, states);
	printList(((lelem*)clauses->front->elem)->elem.l);
	if(x==1) printf(" = TRUE\n\n");
	else printf(" = FALSE\n\n");

	printf("getVars:\n");//gets all the vars from the first clause of the clauses list.
	printList(getVars(((lelem*)clauses->front->next->elem)->elem.l));
	printf("\n\n");

	printf("getAllVars:\n");//gets all the vars in the clause list
	printList(getAllVars(clauses));
	printf("\n\n");

	printf("unsatClauses:\n");//gets all unsatisfied clauses for the supplied states
	printList(unsatClauses(clauses, states));
	printf("\n\n");

	printf("flipVar:\n");//flips the first variable
	printList(flipVar(((lelem*)((lelem*)states->front->elem)->elem.l->front->elem)->elem.c, states));
	printf("\n\n");

	printf("getBetterNeighbor\n");//finds a better neighbor for the clauses and states
	getBetterNeighbor(clauses, states, getAllVars(clauses), length(unsatClauses(clauses, states)));
	printf("\n");

	printf("simpleHillClimb\n");//finds the best set of states for the clauses
	printList(simpleHillClimb(clauses, states,length(getAllVars(clauses)) * length(getAllVars(clauses)) ,unsatClauses(clauses,states)));
	printf("\n");
	return 0;
}
void printList(list* l){//prints a list of type lelem
	lnode* chk = l->front;
	printf("(");
	if(chk){//list is not empty
		lelem* elem;
		elem =(lelem*) chk->elem;
		switch(elem->type){
			case 1://list
				printList(elem->elem.l);
				break;
			case 2://char
				printf("%c",elem->elem.c);
				break;
			case 3://truth
				assert(elem->elem.c == '1' || elem->elem.c == '0');
				if(elem->elem.c == '1') printf("#T");
				else printf("#F");
				break;
			case 4://not
				assert(elem->elem.c == '~');
				printf("NOT");
				break;
			default:
				printf("bad type");
				assert(elem->type >=1 && elem->type <=4);
		}
		if(chk != l->back) do{//list has more than one element
			chk = chk->next;
			printf(" ");
			elem = (lelem*) chk->elem;
			switch(elem->type){
				case 1://list
					printList(elem->elem.l);
					break;
				case 2://char
					printf("%c",elem->elem.c);
					break;
				case 3://truth
					assert(elem->elem.c == '1' || elem->elem.c == '0');
					if(elem->elem.c == '1') printf("#T");
					else printf("#F");
					break;
				case 4://not
					assert(elem->elem.c == '~');
					printf("NOT");
					break;
				default:
					printf("bad type");
					assert(elem->type >=1 && elem->type <=4);
			}
		} while(chk != l->back);
	}
	printf(")");
}
list* textToList(char* text){//converts a char array to a list of type lelem
	list* currentList = createList();//initialize list
	int i;
	if(text[0] != '('){//no opening parenthesis.
		printf("invalid list.");
		return NULL;
	}
	for(i=1; text[i] != '\0' && i < strlen(text); i++){//while more text exists
		lelem* elem;
		switch(text[i]){
			case ' '://skip spaces
				break;
			case '('://inner list
				elem = malloc(sizeof(lelem));
				elem->type= 1;
				elem->elem.l = createList();
				elem->elem.l = textToList(&text[i]);
				append(currentList, elem);//append list to

				int open = 1;
				do {//move to closing brace
					i++;
					switch(text[i]){
						case '(':
							open++;
							break;
						case ')':
							open--;
							break;
						default://skip non parentheses
							break;
					}
				}while(open > 0);


				if(text[i+1] == '\0') return currentList; //if end, return
				break;
			case ')'://close this list and return it
				return currentList;
				break;
			case '#'://check for a truth value
				assert(text[i+1] == 'T' || text[i+1]=='F');
				if(text[i+1] == 'T'){//true
					elem = malloc(sizeof(lelem));
					elem->type= 3;
					elem->elem.c = '1';
					append(currentList, elem);
					assert(text[i+2] == ')');//truth value should be end of list
					return currentList;
				}
				else{//false
					elem = malloc(sizeof(lelem));
					elem->type= 3;
					elem->elem.c = '0';
					append(currentList, elem);
					assert(text[i+2] == ')');//truth value should be end of list
					return currentList;
				}
				break;
			case '1'://bad symbols
			case '0':
			case '~':
				printf("Invalid Character.");
				assert(text[i] != '0');
				assert(text[i] != '1');
				assert(text[i] != '~');
				break;
			case 'N'://might be a not
				if(text[i+1] == 'O' && text[i+2] == 'T' && text[i+3] == ' '){//is a not
					elem = malloc(sizeof(lelem));
					elem->type= 4;
					elem->elem.c = '~';
					append(currentList, elem);
					i+=3;
					break;
				}
				//else is state 'N', so fall through to default
			default://is a state
				assert(text[i] >='A' && text[i] <='Z');//States must be a capital letter.
				assert(text[i+1] == ' ' || text[i+1] == ')');//single letter state names only
				elem = malloc(sizeof(lelem));
				elem->type= 2;
				elem->elem.c = text[i];
				append(currentList, elem);//add state to list
		}
	}
	printf("list was not closed properly");//never closed before end of text
	return NULL;
}
list* deepCopy(list* original){//makes a copy of a list composed of type lelem
	if(isEmpty(original)) return createList();//empty list
	list* copy = createList();
	lnode* iter = original->front;

	lelem* elem = malloc(sizeof(lelem));//first element
	elem->type = ((lelem*)iter->elem)->type;
	if(elem->type >=2 && elem->type <=4 ){//char
		elem->elem.c = ((lelem*)iter->elem)->elem.c;
	}
	else elem->elem.l = deepCopy(((lelem*)iter->elem)->elem.l);//list
	append(copy, elem);//add first elem to list
	if(iter != original->back) do{//if more elements, keep going.
		iter = iter->next;
		elem = malloc(sizeof(lelem));
		elem->type = ((lelem*)iter->elem)->type;
		if(elem->type >=2 && elem->type <=4 ){//char
			elem->elem.c = ((lelem*)iter->elem)->elem.c;
		}
		else elem->elem.l = deepCopy(((lelem*)iter->elem)->elem.l);//list
		append(copy, elem);//add next elem to list
	}while(iter != original->back);
	return copy;//return the copy
}
//void freeList(list* l){//frees a list of type lelem
//	if(!isEmpty(l)){
//		lnode* iter = l.front;
//		if(iter == l.back){
//			if(((lelem*)iter->elem))
//		}
//		else {
//
//			do{
//
//
//			}while(iter != l.back);
//		}
//	}
//}
int evalVar(char var, list* states){//evaluates a char variable
	assert(var >= 'A' && var <= 'Z');
	int truth = -1;
	lnode* chk = states->front;
	if(((lelem*)((lelem*)chk->elem)->elem.l->front->elem)->elem.c == var){//check first state
		truth = ((lelem*)((lelem*)chk->elem)->elem.l->front->next->elem)->elem.c - '0';
	}
	else if(chk!=states->back) do{//check consecutive states
		chk = chk->next;
		if(((lelem*)((lelem*)chk->elem)->elem.l->front->elem)->elem.c == var){
			truth = ((lelem*)((lelem*)chk->elem)->elem.l->front->next->elem)->elem.c - '0';
			break;
		}
	}while(chk != states->back);
	return truth;//return the truth value
}
int evalClause(list* clause, list* states){//evaluates a clause
	if(((lelem*)clause->front->elem)->type == 2){//state variable
		int e = evalVar(((lelem*)clause->front->elem)->elem.c, states);
		assert(e==1 || e==0);
		if(e==1) return 1;//true
	}
	else{//not statement, flip truth
		int e = evalVar(((lelem*)((lelem*)clause->front->elem)->elem.l->front->next->elem)->elem.c, states);
		assert(e==1 || e==0);
		if(e==0) return 1;//not false
	}
	if(((lelem*)clause->front->next->elem)->type == 2){//state variable
		int e = evalVar(((lelem*)clause->front->next->elem)->elem.c, states);
		assert(e==1 || e==0);
		if(e==1) return 1;//true
	}
	else{//not statement, flip truth
		int e = evalVar(((lelem*)((lelem*)clause->front->next->elem)->elem.l->front->next->elem)->elem.c, states);
		assert(e==1 || e==0);
		if(e==0) return 1;//not false
	}
	if(((lelem*)clause->front->next->next->elem)->type == 2){//state variable
		int e = evalVar(((lelem*)clause->front->next->next->elem)->elem.c, states);
		assert(e==1 || e==0);
		if(e==1) return 1;//true
	}
	else{//not statement, flip truth
		int e = evalVar(((lelem*)((lelem*)clause->front->next->next->elem)->elem.l->front->next->elem)->elem.c, states);
		assert(e==1 || e==0);
		if(e==0) return 1;//not false
	}
	return 0;//all are false, return 0
}
list* getVars(list* clause){//gets all variables from a clause
	list* vars = createList();
	lelem* elemA, *elemB, *elemC;
	elemA = malloc(sizeof(lelem));
	elemA->type= 2;
	if(((lelem*)clause->front->elem)->type == 2)//state
		elemA->elem.c = ((lelem*)clause->front->elem)->elem.c;
	else//not
		elemA->elem.c = ((lelem*)((lelem*)clause->front->elem)->elem.l->front->next->elem)->elem.c;

	elemB = malloc(sizeof(lelem));
	elemB->type= 2;
	if(((lelem*)clause->front->next->elem)->type == 2)//state
		elemB->elem.c = ((lelem*)clause->front->next->elem)->elem.c;
	else//not
		elemB->elem.c = ((lelem*)((lelem*)clause->front->next->elem)->elem.l->front->next->elem)->elem.c;

	elemC = malloc(sizeof(lelem));
	elemC->type= 2;
	if(((lelem*)clause->front->next->next->elem)->type == 2)//state
		elemC->elem.c = ((lelem*)clause->front->next->next->elem)->elem.c;
	else//not
		elemC->elem.c = ((lelem*)((lelem*)clause->front->next->next->elem)->elem.l->front->next->elem)->elem.c;

	append(vars, elemA);//build the var list
	append(vars, elemB);
	append(vars, elemC);
	return vars;
}
list* getAllVars(list* clauses){//get all the vars in the clause list
	list* allVars, *vars;
	lnode* clause = clauses->front;

	allVars = createList();//initialize var list
	vars = getVars(((lelem*)clause->elem)->elem.l);

	append(allVars, vars->front->elem);//first elem cant be in an empty list. add it.
	if(findVar(allVars, ((lelem*)vars->front->next->elem)->elem.c))//if not present, add it.
		append(allVars, vars->front->next->elem);
	if(findVar(allVars, ((lelem*)vars->front->next->next->elem)->elem.c))//if not present, add it.
		append(allVars, vars->front->next->next->elem);

	if(clause != clauses->back) do{//if more clauses
		clause = clause->next;
		vars = getVars(((lelem*)clause->elem)->elem.l);
		if(findVar(allVars, ((lelem*)vars->front->elem)->elem.c))//if not present, add it.
			append(allVars, vars->front->elem);
		if(findVar(allVars, ((lelem*)vars->front->next->elem)->elem.c))//if not present, add it.
			append(allVars, vars->front->next->elem);
		if(findVar(allVars, ((lelem*)vars->front->next->next->elem)->elem.c))//if not present, add it.
			append(allVars, vars->front->next->next->elem);
	}while(clause != clauses->back);

	return allVars;
}
int findVar(list* allVars, char state){//finds if a var exists in a variable list. returns 1 if not present, else 0
	lnode* var = allVars->front;
	if(((lelem*)var->elem)->elem.c == state){//check first element
		return 0;//present
	}
	else if(var != allVars->back) do{//check consecutive elements
		var = var->next;
		if(((lelem*)var->elem)->elem.c == state){
			return 0;//present
		}
	}while(var != allVars->back);
	return 1;//not present
}
list* unsatClauses(list* clauses, list* states){//returns the unsatisfied clauses in a clause list for a set of states
	list* unsat = createList();//initialize
	lnode* chk = clauses->front;
	if(evalClause(((lelem*)chk->elem)->elem.l, states) == 0){//first is unsat
		append(unsat, chk->elem);
	}
	if(chk!=clauses->back) do{//check next clauses
		chk = chk->next;
		if(evalClause(((lelem*)chk->elem)->elem.l, states) == 0){//next is unsat
			append(unsat, chk->elem);
		}
	}while(chk != clauses->back);
	return unsat;
}
list* flipVar(char var, list* states){//flips a specified variable
	list* newStates;
	newStates = deepCopy(states);//copy the states

	lnode* state = newStates->front;
	if(((lelem*)((lelem*)state->elem)->elem.l->front->elem)->elem.c == var){//if first is the var
		if(((lelem*)((lelem*)state->elem)->elem.l->front->next->elem)->elem.c == '1')//if true
			((lelem*)((lelem*)state->elem)->elem.l->front->next->elem)->elem.c = '0';//make false
		else ((lelem*)((lelem*)state->elem)->elem.l->front->next->elem)->elem.c = '1';//else is false, so make true
		return newStates;
	}
	else if(state != newStates->back) do{//check consecutive states
		state = state->next;
		if(((lelem*)((lelem*)state->elem)->elem.l->front->elem)->elem.c == var){//if next is var
			if(((lelem*)((lelem*)state->elem)->elem.l->front->next->elem)->elem.c == '1')//if true
				((lelem*)((lelem*)state->elem)->elem.l->front->next->elem)->elem.c = '0';//make false
			else ((lelem*)((lelem*)state->elem)->elem.l->front->next->elem)->elem.c = '1';//else is false, make true
			return newStates;
		}
	}while(state != newStates->back);
	return newStates;//var didnt exist, return original list.
}
list* getBetterNeighbor(list* clauses, list* states, list* vars, int numUnsat){//flips variables until one set of states has less unsat
	list* newStates;
	lnode* iter = vars->front;

	newStates = flipVar(((lelem*)iter->elem)->elem.c, states);//try flipping first var
	if(length(unsatClauses(clauses,	newStates))<numUnsat){//better neighbor
		printf("found better neighbor.\n");
		printList(newStates);
		printf("\n");
		return newStates;
	}
	if(iter != vars->back) do{//if more vars
		iter = iter->next;
		newStates = flipVar(((lelem*)iter->elem)->elem.c, states);//try flipping next var
		if(length(unsatClauses(clauses,	newStates))<numUnsat){//better neighbor
			printf("found better neighbor.\n");
			printList(newStates);
			printf("\n");
			return newStates;
		}
	}while(iter != vars->back);
	printf("no better neighbor...\n");//couldnt find a better neighbor
	return NULL;
}
list* simpleHillClimb(list* clauses, list* states, int dist, list* unsat){//finds the best states for the clauses
	if(isEmpty(unsat)) return states;//if all are satisfied, return these states
	list* newStates = getBetterNeighbor(clauses, states, getAllVars(clauses), length(unsat));//find a better neighbor
	if(newStates == NULL){//there was no better neighbor
		printf("This is the best we can do. (%d unsatisfied: ", length(unsat));
		printList(unsat);
		printf(" )\n");
		return states;
	}
	else return simpleHillClimb(clauses, newStates, dist-1, unsatClauses(clauses, newStates));//recurse with the new states
}
