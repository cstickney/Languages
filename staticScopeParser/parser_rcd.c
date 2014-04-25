#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"
#include "scanner.h"
#include "turtle.h"
//#include "strdup.h"
#define DEBUG 1
#define SCALE 0

enum { //assign values to constants representing the available actions
	ASSIGN_STMT = 1,
	BLOCK_STMT = 2,
	WHILE_STMT = 3,
	IF_STMT = 4,
	ACTION_STMT = 5,
	NUM_EXPR = 6,
	VAR_EXPR = 7,
	ADD_EXPR = 8,
	SUB_EXPR = 9,
	MULT_TERM = 10,
	DIV_TERM = 11,
	NEG_EXPR =12,
	INIT_STMT= 13
};
typedef struct Expr {//type, op
	int type;  //NUM_EXPR, VAR_EXPR, ADD_EXPR, SUB_EXPR //=, <, >, NE, LE, GE
	union {
		float num;      //NUM_EXPR
		SymTab *sym;    //VAR_EXPR
		struct Expr *unary;  //unary operation (NEG_EXPR, ... );
		struct {
			struct Expr *left, *right;
		} binary;  //binary operation (ADD_EXPR,...);
	} op;
} Expr;
typedef struct Stmt {//type, s, next
	int type;  //WHILE_, IF_, ASSIGN_, BLOCK_
	union {
		struct {
			SymTab *lval;
			Expr *rval;
		} assign_;
		struct {
			Expr *cond;
			struct Stmt *body;
		} while_;
		struct {
			struct Stmt *list;
		} block_;
		struct {
			Expr *cond;
			struct Stmt *body;
			struct Stmt *elseBody;
		} if_;
		struct {
			int action;
			Expr *rval;
		} action_;
	} s;
	struct Stmt *next;
} Stmt;

SymTab *symtab = NULL;//variable linked list
int lookahead = 0;//type of next element
LVAL *lval;//current symbol
int initOk = 1;

void program();
void stmt_seq();
Stmt* stmt();//
Stmt* action_stmt();
Stmt* assign_stmt();
Stmt* init_stmt();
Stmt* block_stmt();
int blockChk();
Stmt* if_stmt();
Stmt* elsif_stmt();
Stmt* while_stmt();
Stmt* createInitStmt();
Stmt* createActionStmt(int action, Expr *rval);
Stmt* createAssignStmt(SymTab* var, Expr *rval);
Stmt* createBlockStmt(Stmt* list);
Stmt* createIfStmt(Expr* cond, Stmt* body, Stmt* elseBody);
Stmt* createElsifStmt(Expr* cond, Stmt* body, Stmt* elseBody);
Stmt* createWhileStmt(Expr* cond, Stmt* body);
float* getValue(char* name);
float evalExpr(Expr *expr);
void match(int tok);
void executeStmt(Stmt* stmt);
void executeInitStmt(Stmt* stmt);
void execActionStmt(Stmt* stmt);
void execAssignStmt(Stmt* stmt);
void execBlockStmt(Stmt* stmt);
void execIfStmt(Stmt* stmt);
void execWhileStmt(Stmt* stmt);
Expr* expr();
Expr* term();
Expr* factor();
Expr* bool();
Expr* bool_term();
Expr* bool_factor();
Expr* cmp();
void destroyStmt(Stmt* s);
void destroyExpr(Expr* e);
void destroySymtab(SymTab* s);
char *strdup(const char *str);
SymTab symcpy(SymTab *original);
SymTab* symRestore(SymTab* backup, SymTab* current, SymTab* first);

int main(void) {
	lval = (LVAL*) malloc(sizeof(LVAL));
	turtleInit();
	lookahead = nextToken(lval);//gets first token

	if (!lookahead){//empty program
		fprintf(stderr, "Nothing to read");
		return(1);
	}
	else//begin parsing statements
		program();
	turtleDumpImage(stdout);
	destroySymtab(symtab);//frees the symbol table
	return 0;

}
void program() {// program -> stmt_list
	stmt_seq();
}
void stmt_seq() {//iterates through the stmts
	Stmt *s = stmt(); //Build our AST
	if (s == NULL)
		printf("execute stmt null \n");

	executeStmt(s); //Eval our stmt
	destroyStmt(s);
	do {
		Stmt *s = stmt(); //Build our AST
		if (s == NULL){
			printf("error in stmt_seq, execute stmt null \n");
			exit(1);
		}
		if (s->type == INIT_STMT){
			printf("error: tried to initialize a variable in the middle of stmt sequence\n");
			exit(1);
		}

		executeStmt(s); //Eval our stmt
		destroyStmt(s);
	}
	while (lookahead != 0);
}
Stmt* stmt() {//stmt -> assign|while|if|action (also elsif)
	switch (lookahead) {
		case FORWARD_:
			return action_stmt();
			break;
		case LEFT_:
			return action_stmt();
			break;
		case RIGHT_:
			return action_stmt();
			break;
		case HOME_:
			return action_stmt();
			break;
		case PENDOWN_:
			return action_stmt();
			break;
		case PENUP_:
			return action_stmt();
			break;
		case PUSHSTATE_:
			return action_stmt();
			break;
		case POPSTATE_:
			return action_stmt();
			break;
		case WHILE_:
			return while_stmt();
			break;
		case IF_:
			return if_stmt();
			break;
		case ELSIF_:
			return elsif_stmt();
		case IDENT_:
			return assign_stmt();
			break;
		case VAR_:
			if(initOk)return init_stmt();
			else {
				fprintf(stderr, "cannot initialize a variable in the middle of a code block\n");
				exit(1);
			}
			break;
		default:
			fprintf(stderr, "error in stmt(), invalid token number(not a statement): %d", lookahead);
			exit(1);
			break;
	}
	return NULL;
}
Stmt* action_stmt(){//action -> home|penup|pendown|forward|left|right|pushstate|popstate
	int action = lookahead;
	lookahead = nextToken(lval);//moves cursor to read in parameter
	switch (action) {
		case FORWARD_://FORWARD expr
			return createActionStmt(action, expr());
			break;
		case LEFT_://LEFT expr
			return createActionStmt(action, expr());
			break;
		case RIGHT_://RIGHT expr
			return createActionStmt(action, expr());
			break;
		case HOME_://HOME
			return createActionStmt(action, NULL);
			break;
		case PENDOWN_://PENDOWN
			return createActionStmt(action, NULL);
			break;
		case PENUP_://PENUP
			return createActionStmt(action, NULL);
			break;
		case PUSHSTATE_://PUSHSTATE
			return createActionStmt(action, NULL);
			break;
		case POPSTATE_://POPSTATE
			return createActionStmt(action, NULL);
			break;
		default:
			fprintf(stderr, "error in action_stmt(), invalid token number(not an action stmt): %d", action);
			exit(1);
			break;
	}
	return NULL;
}

Stmt* assign_stmt(){//assign -> IDENT ASSIGN expr
	SymTab* var = malloc(sizeof(SymTab));
	var->sym = strdup(lval->s);//make IDENT a symbol
	var->next = NULL;
	lookahead = nextToken(lval);//next token
	match(ASSIGN_);//make sure is ASSIGN
	return createAssignStmt(var, expr());//return created assign stmt of symbol and expr call
}
Stmt* block_stmt(){//block -> stmt {stmt}
	Stmt* list = stmt();//get first statement
	Stmt* current = list;//set current to first stmt in list
	current->next = NULL;
	while(blockChk()){//make sure block has not ended from a FI or OD
		current->next = stmt();//assign next as stmt call
		current = current->next;//move current to next
		current->next = NULL;
	}
	return createBlockStmt(list);//creates a block with the stmt list
}
int blockChk(){//checks for OD and FI
	switch(lookahead){
		case IDENT_:
		case WHILE_:
		case IF_:
		case HOME_:
		case PENUP_:
		case PENDOWN_:
		case FORWARD_:
		case RIGHT_:
		case LEFT_:
		case PUSHSTATE_:
		case POPSTATE_://is a stmt
			return 1;
		default://close block
			return 0;
	}
}
Stmt* if_stmt(){//IF bool THEN block {ELSIF bool THEN block} [ELSE block] FI
	Stmt* body;
	Expr* cond;
	Stmt* elseBody;
	lookahead = nextToken(lval);//move past IF
	cond = bool();//creates a cond from a bool call
	match(THEN_);//make sure next token is a THEN
	body = block_stmt();//creates a block from a block_stmt call
	elseBody = NULL;
	if(lookahead == ELSIF_){//checks for an ELSIF
		elseBody= elsif_stmt();//creates an elseif stmt
	}
	else if(lookahead == ELSE_){//checks for an ELSE
		lookahead = nextToken(lval);//skips the else
		elseBody = block_stmt();//creates a body for the ELSE
	}
	match(FI_);//matches closing if
	return createIfStmt(cond, body, elseBody);//returns new if stmt
}
Stmt* elsif_stmt(){//ELSIF bool THEN block {ELSIF bool THEN block} [ELSE block]
	Expr* cond;
	Stmt* body;
	Stmt* elseBody;
	lookahead = nextToken(lval);//moves past ELSIF
	cond = bool();//creates a cond from a bool call
	match(THEN_);//make sure next token is a THEN
	body = block_stmt();//creates a block from a block_stmt call
	elseBody = NULL;
	if(lookahead == ELSIF_){//checks for an ELSIF
		elseBody= elsif_stmt();//creates an elseif stmt
	}
	else if(lookahead == ELSE_){//checks for an ELSE
		lookahead = nextToken(lval);//skips the else
		elseBody = block_stmt();//creates a body for the ELSE
	}
	return createElsifStmt(cond, body, elseBody);
}
Stmt* while_stmt(){//WHILE bool DO block OD
	Expr* cond;
	Stmt* body;
	lookahead = nextToken(lval);//skip while
	cond = bool();//create a bool
	match(DO_);//matches DO
	body = block_stmt();//create a block
	match(OD_);//matches closing OD
	return createWhileStmt(cond, body);//creates a while
}
Stmt* createActionStmt(int action, Expr* rval) {//creates an action statement
	Stmt* s = (Stmt*) malloc(sizeof(Stmt));
	s->type = ACTION_STMT;//sets stmt type
	s->s.action_.action = action;//copies action to stmt
	s->s.action_.rval = rval;//copies rval to stmt, if there is one
	s->next = NULL;
	return s;
}
Stmt* createAssignStmt(SymTab* var, Expr* rval){//creates an assign statement
	Stmt* s = (Stmt*) malloc(sizeof(Stmt));
	s->type = ASSIGN_STMT;//sets statement type
	s->s.assign_.lval = var;//copies symbol to stmt
	s->s.assign_.rval = rval;//copies the expression to the stmt
	s->next = NULL;
	return s;
}
Stmt* createBlockStmt(Stmt* list){//creates a block stmt
	Stmt* s = (Stmt*) malloc(sizeof(Stmt));
	s->type = BLOCK_STMT;//sets statement type
	s->s.block_.list = list;//copies stmt list to block
	return s;
}
Stmt* createIfStmt(Expr* cond, Stmt* body, Stmt* elseBody){//creates an if stmt
	Stmt* s = (Stmt*) malloc(sizeof(Stmt));
	s->type = IF_STMT;//sets statement type
	s->s.if_.cond = cond;//copies bool to if
	s->s.if_.body = body;//copies block to if
	s->s.if_.elseBody = elseBody;//copies else block to elsebody
	return s;
}
Stmt* createElsifStmt(Expr* cond, Stmt* body, Stmt* elseBody){//creates an elsif
	Stmt *s = (Stmt*) malloc(sizeof(Stmt));
	s->type = ELSIF_;//sets statement type
	s->s.if_.cond = cond;//copies condition to stmt
	s->s.if_.body = body;//copies body to stmt
	s->s.if_.elseBody = elseBody;//copies else block to elsebody
	return s;
}
Stmt* createWhileStmt(Expr* cond, Stmt* body){//creates a while
	Stmt *s = (Stmt*) malloc(sizeof(Stmt));
	s->type = WHILE_STMT;//sets statement type
	s->s.while_.cond = cond;//copies condition
	s->s.while_.body = body;//copies body
	return s;
}
float* getValue(char* name){//returns the value of a variable, or NULL if it does not exist
	if(DEBUG) fprintf(stderr, "getvalue");
	SymTab* current;//reference to working symbol
	if(symtab == NULL){//symbol table is null
		fprintf(stderr, "error in getValue(), variable does not exist (%s)", *name);
		exit(1);
	}
	current = symtab;//assigns current to first element
	while(current->next != NULL){//checks if next symbol is null
		if(strncmp(current->sym, name, strlen(name)) == 0){//checks if name matches symbol
			float* value = &current->val;//passes the value of the current symbol
			if(DEBUG) fprintf(stderr, "%s =%f\n",name, *value);
			return value;
		}
		current = current->next;//moves current to the next symbol
	}
	if(strncmp(current->sym, name, strlen(name)) == 0){//checks last symbol
		float* value = &current->val;//passes value of the last symbol
		if(DEBUG) fprintf(stderr, "%s =%f\n",name, *value);
		return value;
	}
	fprintf(stderr, "error in getValue(), variable does not exist (%s)", *name);
	exit(1);
}
float evalExpr(Expr *expr) {//recursively evaluates value of an expression
	switch (expr->type) {
		case NUM_EXPR://returns number
			return expr->op.num;
			break;
		case VAR_EXPR://returns a getvalue call on the variable name
			return *getValue(expr->op.sym->sym);
			break;
		case ADD_EXPR://returns expr + expr
			return evalExpr(expr->op.binary.left) + evalExpr(expr->op.binary.right);
			break;
		case SUB_EXPR://returns expr - expr
			return evalExpr(expr->op.binary.left) - evalExpr(expr->op.binary.right);
			break;
		case NEG_EXPR://returns -expr
			return -evalExpr(expr->op.unary);
			break;
		case MULT_TERM://returns expr * expr
			return evalExpr(expr->op.binary.left) * evalExpr(expr->op.binary.right);
			break;
		case DIV_TERM://returns expr / expr
			if(DEBUG) fprintf(stderr, "here?\n");
			return evalExpr(expr->op.binary.left) / evalExpr(expr->op.binary.right);
			break;
		case '=': //returns 1 if expr = expr, 0 if !=
			if (evalExpr(expr->op.binary.left)== evalExpr(expr->op.binary.right)) return 1;
			else return 0;
			break;
		case '<'://returns 1 if expr < expr, 0 if >=
			if (evalExpr(expr->op.binary.left)< evalExpr(expr->op.binary.right)) return 1;
			else return 0;
			break;
		case '>'://returns 1 if expr > expr, 0 if <=
			if (evalExpr(expr->op.binary.left)> evalExpr(expr->op.binary.right)) return 1;
			else return 0;
			break;
		case NE_://returns 1 if expr != expr, 0 if ==
			if (evalExpr(expr->op.binary.left)!= evalExpr(expr->op.binary.right)) return 1;
			else return 0;
			break;
		case LE_://returns 1 if expr <= expr, 0 if >
			if (evalExpr(expr->op.binary.left)<= evalExpr(expr->op.binary.right)) return 1;
			else return 0;
			break;
		case GE_://returns 1 if expr >= expr, 0 if <
			if (evalExpr(expr->op.binary.left)>= evalExpr(expr->op.binary.right)) return 1;
			else return 0;
			break;
		case NOT_://returns 1 if false, 0 if true
			if (evalExpr(expr->op.unary) == 0) return 1;
			else return 0;
			break;
		case AND_://returns 1 if expr and expr are true, else 0
			if(evalExpr(expr->op.binary.left) && evalExpr(expr->op.binary.right)) return 1;
			else return 0;
			break;
		case OR_://returns 1 if expr or expr are true, else 0
			if(evalExpr(expr->op.binary.left) || evalExpr(expr->op.binary.right)) return 1;
			else return 0;
			break;
		default:
			fprintf(stderr, "error in evalExpr(), invalid token number(not an expr type): %d", expr->type);
			exit(1);
			return 0;
			break;
	}
	return 0;
}
void match(int tok) {// enforce match of current token, fetch next token
	if (tok == lookahead)//match
		lookahead = nextToken(lval);
	else{//not match
		fprintf(stderr, "token matching error. expected %d, found %d \n", tok, lookahead);
		exit(1);
	}

}
void executeStmt(Stmt *stmt) {//passes execution of stmt to appropriate function
	switch (stmt->type) {
		case ACTION_STMT:
			execActionStmt(stmt);
			break;
		case ASSIGN_STMT:
			execAssignStmt(stmt);
			break;
		case BLOCK_STMT:
			execBlockStmt(stmt);
			break;
		case IF_STMT:
		case ELSIF_://elsifs are the same executionwise as ifs
			execIfStmt(stmt);
			break;
		case WHILE_STMT:
			execWhileStmt(stmt);
			break;
		case INIT_STMT:
			executeInitStmt(stmt);
			break;
		default:
			fprintf(stderr, "error in executeStmt(), invalid token number(not a stmt): %d", stmt->type);
			exit(1);
			break;
	}
	return;
	//execute some logics or call turtle API
}
void execActionStmt(Stmt *stmt) {//Executes an action statement from the turtle API.
	int action = stmt->s.action_.action;
	float param;
	initOk = 0;
	switch (action) {
		case FORWARD_:
			param = evalExpr(stmt->s.action_.rval);
			if(DEBUG) fprintf(stderr, "fwd:%f\n",param);
			if(SCALE) param = param/6;
			turtleForward(param);
			break;
		case LEFT_:
			param = evalExpr(stmt->s.action_.rval);
			if(DEBUG) fprintf(stderr, "left:%f\n",param);
			turtleRotate(param);
			break;
		case RIGHT_:
			param = evalExpr(stmt->s.action_.rval);
			if(DEBUG) fprintf(stderr, "right:%f\n",param);
			turtleRotate(-param);
			break;
		case HOME_:
			if(DEBUG) fprintf(stderr, "home\n");
			turtleHome();
			break;
		case PENUP_:
			if(DEBUG) fprintf(stderr, "penup\n");
			turtlePenUp(1);
			break;
		case PENDOWN_:
			if(DEBUG) fprintf(stderr, "pendown\n");
			turtlePenUp(0);
			break;
		case PUSHSTATE_:
			if(DEBUG) fprintf(stderr, "push\n");
			turtlePushState();
			break;
		case POPSTATE_:
			if(DEBUG) fprintf(stderr, "pop\n");
			turtlePopState();
			break;
		default:
			fprintf(stderr, "error in execActionStmt(), invalid token number(not an action) %d", action);
			exit(1);
			break;
	}
	return;
	//execute action statement by calling turtle API
}
void execAssignStmt(Stmt *stmt){//adds a variable to the symbol table
	char* name = stmt->s.assign_.lval->sym;//gets name from stmt
	float value = evalExpr(stmt->s.assign_.rval);//gets expr value from stmt
	SymTab* current = symtab;//set current to beginning of table
	initOk = 0;
	if(symtab == NULL){
		fprintf(stderr, "error in execAssignStmt(), variable is not defined (%s)",name);
		exit(1);
	}
	while(current->next != NULL){//check if next element of symbol table is null
		if(strncmp(current->sym, name, strlen(name)) ==0){//check if current matches name
			current->val = value;//overwrite existing value
			break;
		}
			current = current->next;//moves current to next symbol
	}
	if(strncmp(current->sym, name, strlen(name)) ==0){//checks last element
		current->val = value;//overwrite existing value
	}
	else{//variable does not exist
		fprintf(stderr, "error in execAssignStmt(), variable is not defined (%s)",name);
		exit(1);
	}
	return;
}
void execBlockStmt(Stmt* stmt){//executes a block statement
	SymTab copy = symcpy(symtab);
	Stmt* current = stmt->s.block_.list;//sets current to beginning of block
	initOk = 1;

	if(current->type == INIT_STMT){
		executeInitStmt(current);
		current = current->next;
	}
	initOk = 0;
	while(current->next != NULL){//while next statement exists
		executeStmt(current);//executes current statement
		current = current->next;//moves current to next statement
	}
	executeStmt(current);//executes current statement

	symtab=symRestore(&copy, symtab, symtab);//restore the symbol table to previous state
}
SymTab symcpy(SymTab* original){
	SymTab copy;
	SymTab* current, *currentCopy;
	currentCopy = &copy;
	current =  original;
	while(current->next != NULL){
		*currentCopy = *current;
		current = current->next;
		currentCopy->next = malloc(sizeof(SymTab));
		currentCopy = currentCopy->next;
	}
	*currentCopy = *current;
	currentCopy->next = NULL;
	return copy;
}

SymTab* symRestore(SymTab* backup, SymTab* current, SymTab* first){
	if(first == NULL){
		return NULL;
	}
	if(first->next == NULL){
		if(backup == NULL){
			return NULL;
		}
		else{
			return first;
		}
	}

	if(strncmp(current->sym, current->next->sym, strlen(current->sym))==0){//duplicate symbol found
		if(backup->next == NULL){// end of backup x/xx
			first = first->next;
			return first;
		}
		else if(strncmp(current->sym, backup->next->sym, strlen(current->sym)) !=0){//next symbols are different

			first = first->next;

			current = current->next;
			backup = backup->next;
			current->next= symRestore(backup->next, current->next->next, first->next);
			return first;
		}
		else{//next symbols are both duplicates
			current = current->next;
			backup = backup->next;
			return symRestore(backup, current, first);
		}
	}
	else{//single token
		first->next = symRestore(backup->next, current->next, first->next);
		return first;

	}
}


void execIfStmt(Stmt* stmt){//executes an if statement
	initOk = 0;
	if(evalExpr(stmt->s.if_.cond) == 1){//checks that if condition is true
		executeStmt(stmt->s.if_.body);//executes body statement
	}

	else if(stmt->s.if_.elseBody != NULL){//looks for an else body
		executeStmt(stmt->s.if_.elseBody);//executes else body
	}
}
void execWhileStmt(Stmt* stmt){//executes a while statement
	int i=1;
	initOk = 0;
	while(evalExpr(stmt->s.while_.cond)){//recursively checks the condition
		if(DEBUG) fprintf(stderr, "loop %d\n", i);
		++i;
		executeStmt(stmt->s.while_.body);//executes the while body
	}
}

Expr* expr() {// expr -> term {+ term | - term}
	Expr* temp = term();//assumes first element of expr is a term
	Expr* e;
	e=temp;
	while(1){
		switch(lookahead){
			case '+'://e= expr+expr
				lookahead = nextToken(lval);//moves to next space past +
				e = malloc(sizeof(Expr));
				e->type= ADD_EXPR;//expr is a +
				e->op.binary.left = temp;
				e->op.binary.right = term();
				break;
			case '-'://e= expr-expr
				lookahead = nextToken(lval);//moves to next space past -
				e = malloc(sizeof(Expr));
				e->type= SUB_EXPR;//expr is a -
				e->op.binary.left = temp;
				e->op.binary.right = term();
				break;
			default://no +/-
				return e;
		}
	}
	return e;
}
Expr* term() {//term -> factor {*factor | /factor}
	Expr* temp = factor();//assumes first term is a factor
	Expr* e;
	e=temp;
	while(1){
		switch(lookahead){
			case '*'://e= term*term
				lookahead = nextToken(lval);//moves past *
				e = malloc(sizeof(Expr));
				e->type= MULT_TERM;// term is a *
				e->op.binary.left = temp;
				e->op.binary.right = term();
				break;
			case '/'://e=term/term
				lookahead = nextToken(lval);//moves past /
				e = malloc(sizeof(Expr));
				e->type= DIV_TERM;//term is a /
				e->op.binary.left = temp;
				e->op.binary.right = term();
				break;
			default://no * or /
				return e;
		}
	}
	return e;
}
Expr* factor() {//+factor|-factor|(expr)|IDENT|REAL
	Expr* e;
	SymTab* tempSymTab;
	switch(lookahead){
		case REAL_: //return value of the number
			e = malloc(sizeof(Expr));
			e->type= NUM_EXPR;
			e->op.num = lval->f;
			lookahead = nextToken(lval);//checks next token past #
			break;
		case IDENT_://return the name of the symbol
			e = malloc(sizeof(Expr));
			e->type= VAR_EXPR;
			tempSymTab = malloc(sizeof(SymTab));
			tempSymTab->sym = strdup(lval->s);
			tempSymTab->next = NULL;
			e->op.sym = tempSymTab;
			lookahead = nextToken(lval);
			break;
		case '-': //return expr - expr
			lookahead = nextToken(lval);
			e = malloc(sizeof(Expr));
			e->type = NEG_EXPR;
			e->op.unary = expr();
			return e;
			break;
		case '('://return expr inside parens
			lookahead = nextToken(lval);
			e=expr();
			match(')');
			return e;
			break;
		default:
			fprintf(stderr, "error in factor(), invalid token number(not a factor): %d", lookahead);
			exit(1);
			break;
	}
	return e;
}
Expr* bool(){//bool_term {OR bool_term}
	Expr* temp = bool_term();//assumes first part of bool is a term
	Expr* e;
	e = temp;
	while(1){
		switch(lookahead){
			case OR_:// returns an OR that references booleans on either side.
				lookahead = nextToken(lval);
				e = malloc(sizeof(Expr));
				e->type = OR_;
				e->op.binary.left = temp;
				e->op.binary.right = bool_term();
				temp = e;
				break;
			default://returns the boolean expression
				return e;
				break;
		}
	}
	return e;
}
Expr* bool_term(){//bool_factor {AND bool_factor}
	Expr* temp = bool_factor();//assume first part of bool term is a factor
	Expr* e;
	e=temp;
	while(1){
		switch(lookahead){
			case AND_://returns an and expression
				lookahead = nextToken(lval);
				e = malloc(sizeof(Expr));
				e->type = AND_;
				e->op.binary.left = temp;
				e->op.binary.right = bool_term();
				temp = e;
				break;
			default://returns the boolean expression
				return e;
				break;
		}
	}
	return e;
}
Expr* bool_factor(){//NOT bool_factor | (bool)|cmp
	Expr* e;
	switch(lookahead){
		case NOT_://e = not expr
			lookahead = nextToken(lval);
			e = malloc(sizeof(Expr));
			e->type = NOT_;
			e->op.unary = bool_factor();
			break;
		case '('://handle parentheses
			lookahead = nextToken(lval);//handle  opening paren
			e = bool();//return what was in the paren
			match(')');//handle closing paren
			return e;
		default://e = cmp
			e = cmp();
			break;
	}
	return e;
}
Expr* cmp(){//cmp -> expr cmp_op expr
	Expr* left = expr();
	Expr* e = malloc(sizeof(Expr));
	switch(lookahead){
		case '=':
			lookahead = nextToken(lval);
			e->type = '=';
			e->op.binary.left = left;
			e->op.binary.right = expr();
			break;
		case '<':
			lookahead = nextToken(lval);
			e->type = '<';
			e->op.binary.left = left;
			e->op.binary.right = expr();
			break;
		case '>':
			lookahead = nextToken(lval);
			e->type = '>';
			e->op.binary.left = left;
			e->op.binary.right = expr();
			break;
		case NE_:
			lookahead = nextToken(lval);
			e->type = NE_;
			e->op.binary.left = left;
			e->op.binary.right = expr();
			break;
		case LE_:
			lookahead = nextToken(lval);
			e->type = LE_;
			e->op.binary.left = left;
			e->op.binary.right = expr();
			break;
		case GE_:
			lookahead = nextToken(lval);
			e->type = GE_;
			e->op.binary.left = left;
			e->op.binary.right = expr();
			break;
		default:
			fprintf(stderr, "error in cmp(), invalid token number(not a comparator) %d", lookahead);
			exit(1);
			break;
	}
	return e;
}
void destroyStmt(Stmt* s){//frees a statement
	return;
	Stmt* current, *next;
	if(DEBUG) fprintf(stderr, "destroyStmt\n");
	if(s->next != NULL){
		destroyStmt(s->next);
	}
	switch(s->type){
		case ASSIGN_STMT:
			if(DEBUG) fprintf(stderr, "destroying assign\n");
			if(s->s.assign_.lval != NULL) destroySymtab(s->s.assign_.lval);
			if(s->s.assign_.rval != NULL) destroyExpr(s->s.assign_.rval);
			break;
		case INIT_STMT:
			if(DEBUG) fprintf(stderr, "destroying init\n");
			if(s->s.assign_.lval != NULL) destroySymtab(s->s.assign_.lval);
			break;
		case WHILE_STMT:
			if(DEBUG) fprintf(stderr, "destroying while\n");
			if(s->s.while_.cond != NULL) destroyExpr(s->s.while_.cond);
			if(s->s.while_.body != NULL) destroyStmt(s->s.while_.body);
			break;
		case BLOCK_STMT:
			if(DEBUG) fprintf(stderr, "destroying block\n");
			if(s->s.block_.list != NULL) destroyStmt(s->s.block_.list);
			break;
		case IF_STMT:
		case ELSIF_:
			if(DEBUG) fprintf(stderr, "destroying if(or elsif)\n");
			if(s->s.if_.cond != NULL) destroyExpr(s->s.if_.cond);
			if(s->s.if_.body != NULL) destroyStmt(s->s.if_.body);
			if(s->s.if_.elseBody != NULL) destroyStmt(s->s.if_.elseBody);
			break;
		case ACTION_STMT:
			if(DEBUG) fprintf(stderr, "destroying action\n");
			if(s->s.action_.rval != NULL) destroyExpr(s->s.action_.rval);
			break;
		default:
			fprintf(stderr, "error in destroyStmt(), undefined type: %d\n", s->type);
			exit(1);
			break;
	}
	if(DEBUG) fprintf(stderr, "freeing stmt\n");
	free(s);
}
void destroyExpr(Expr* e){//frees an expression
	return;
	if(DEBUG) fprintf(stderr, "destroyExpr\n");
	switch(e->type){
		case NUM_EXPR:
			break;
		case VAR_EXPR:
			if(DEBUG) fprintf(stderr, "destroying variable\n");
			if(e->op.sym != NULL) destroySymtab(e->op.sym);
			break;
		case NEG_EXPR:
		case NOT_:
			if(DEBUG) fprintf(stderr, "destroying unary\n");
			if(e->op.unary != NULL) destroyExpr(e->op.unary);
			break;
		case ADD_EXPR:
		case SUB_EXPR:
		case DIV_TERM:
		case MULT_TERM:
		case '=':
		case '<':
		case '>':
		case NE_:
		case GE_:
		case LE_:
		case AND_:
		case OR_:
			if(DEBUG) fprintf(stderr, "destroying binary\n");
			if(e->op.binary.left != NULL) destroyExpr(e->op.binary.left);
			if(e->op.binary.right != NULL) destroyExpr(e->op.binary.right);
			break;
		default:
			fprintf(stderr, "Cannot destroy expr: undefined type %d\n", e->type);
			exit(1);
			break;
	}
	if(DEBUG) fprintf(stderr, "freeing expr\n");
	free(e);
}
void destroySymtab(SymTab* s){//frees a symbol table
	return;
	if(DEBUG) fprintf(stderr, "destroySymtab\n");
	if(s == NULL){
		if(DEBUG) fprintf(stderr, "symtab is null\n");
		return;
	}
	if(s->next != NULL){
		if(DEBUG) fprintf(stderr, "destroying chained symtab\n");
		destroySymtab(s->next);
	}
	if(s->sym != NULL){
		if(DEBUG) fprintf(stderr, "freeing symtab name\n");
		free(s->sym);
	}
	if(DEBUG) fprintf(stderr, "freeing symtab\n");
	free(s);
	return;
}
char *strdup(const char *str){//mallocs and duplicates a string
    int n = strlen(str) + 1;
    char *dup = malloc(n * sizeof(char));
    if(dup)
    {
        strcpy(dup, str);
    }
    return dup;
}

Stmt* init_stmt(){//init -> VAR IDENT
	Stmt* list, *current;
	SymTab* var = malloc(sizeof(SymTab));


	match(VAR_);//move to IDENT
	var->sym = strdup(lval->s);//copy token
	var->next = NULL;//set next symtab to null
	list= createInitStmt(var);//add an init stmt with the variable
	current = list;//set current to
	lookahead = nextToken(lval);//get next token

	while(lookahead == VAR_){//deal with the rest of the inits
		match(VAR_);//move to IDENT
		var = malloc(sizeof(SymTab));
		var->sym = strdup(lval->s);//copy token
		var->next = NULL;//set next symtab to null
		current->next= createInitStmt(var);//add an init stmt with the variable
		current = current->next;//moves to next statement location
		lookahead = nextToken(lval);//get next token
	}
	return list;
}

Stmt* createInitStmt(SymTab* var){//creates an init stmt
	Stmt* s = (Stmt*) malloc(sizeof(Stmt));
	s->type = INIT_STMT;//sets statement type
	s->s.assign_.lval = var;//copies symbol to stmt
	s->next = NULL;
	return s;
}

void executeInitStmt(Stmt* stmt){
	if(DEBUG) fprintf(stderr, "executeInitStmt\n");
	char* name = stmt->s.assign_.lval->sym;//gets name from stmt
	SymTab* current = symtab;//set current to beginning of table
	SymTab* new,last;

	if(symtab == NULL){//initialize table if empty
		symtab = malloc(sizeof(SymTab));

		symtab->sym = strdup(name);
		symtab->next = NULL;
		if(stmt->next != NULL){
			executeInitStmt(stmt->next);
		}
		return;
	}

	current = symtab;//set current to beginning of table
	while(current->next != NULL){//check if next element of symbol table is null
		if(strncmp(current->sym, name, strlen(name)) ==0){//check if current matches name
			*new = *current;
			current->sym = strdup(name);
			current->next = new;
			break;
		}
			current = current->next;//moves current to next symbol
	}
	if(strncmp(current->sym, name, strlen(name)) ==0){//checks last element
		*new = *current;
		current->sym = strdup(name);
		current->next = new;
	}
	else{//create new symbol in table
		SymTab* newVar = malloc(sizeof(SymTab));
		newVar->sym = strdup(name);
		newVar->next = NULL;
		current->next = newVar;
	}
	if(stmt->next != NULL) executeInitStmt(stmt->next);
	return;
}
