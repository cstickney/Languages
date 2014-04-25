#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include "strdup.h"
#include "symtab.h"
#include "scanner.h"
#include "turtle.h"
#define DEBUG 0
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
	NEG_EXPR =12
};
typedef struct Expr {
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
typedef struct Stmt {
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

void program();
void stmt_seq();
Stmt* stmt();
Stmt* action_stmt();
Stmt* assign_stmt();
Stmt* block_stmt();
int blockChk();
Stmt* if_stmt();
Stmt* elsif_stmt();
Stmt* while_stmt();
Stmt* createActionStmt(int action, Expr *rval);
Stmt* createAssignStmt(SymTab* var, Expr *rval);
Stmt* createBlockStmt(Stmt* list);
Stmt* createIfStmt(Expr* cond, Stmt* body, Stmt* elseBody);
Stmt* createElsifStmt(Expr* cond, Stmt* body, Stmt* elseBody);
Stmt* createWhileStmt(Expr* cond, Stmt* body);
float* getValue(char* name);
float evalExpr(Expr *expr);
void match(int tok);
void executeStmt(Stmt* stmt_);
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

int main(void) {
	lval = (LVAL*) malloc(sizeof(LVAL));
	turtleInit();
	lookahead = nextToken(lval);//gets first token

	if (!lookahead)
		fprintf(stderr, "Nothing to read");
	else
		program();
	turtleDumpImage(stdout);
	destroySymtab(symtab);
	return 0;

}
void program() {/* program -> stmt_list */
	stmt_seq();
}
void stmt_seq() {
	do {
		Stmt *s = stmt(); //Build our AST
		if (s == NULL)
			printf("execute stmt null \n");

		executeStmt(s); //Eval our stmt
		destroyStmt(s);
	}
	while (lookahead != 0);
}
Stmt* stmt() {
//	fprintf(stderr, "%d", lookahead);
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
		default:
			fprintf(stderr, "%d", lookahead);
			break;
	}
	if(DEBUG) fprintf(stderr, "failed\n");
	return NULL;
}
Stmt* action_stmt(){
	int action = lookahead;
	lookahead = nextToken(lval);//moves cursor to read in parameter

	switch (action) {
		case FORWARD_:
			return createActionStmt(action, expr());
			break;
		case LEFT_:
			return createActionStmt(action, expr());
			break;
		case RIGHT_:
			return createActionStmt(action, expr());
			break;
		case HOME_:
			return createActionStmt(action, NULL);
			break;
		case PENDOWN_:
			return createActionStmt(action, NULL);
			break;
		case PENUP_:
			return createActionStmt(action, NULL);
			break;
		case PUSHSTATE_:
			return createActionStmt(action, NULL);
			break;
		case POPSTATE_:
			return createActionStmt(action, NULL);
			break;
		default:
			break;
	}
	return NULL;
}
Stmt* assign_stmt(){
	SymTab* var = malloc(sizeof(SymTab));
	var->sym = strdup(lval->s);
	var->next = NULL;
	lookahead = nextToken(lval);
	match(ASSIGN_);
	return createAssignStmt(var, expr());
}
Stmt* block_stmt(){
	Stmt* list = stmt();
	Stmt* current = list;
	current->next = NULL;
	while(blockChk()){
		current->next = stmt();
		current = current->next;
		current->next = NULL;
	}
	return createBlockStmt(list);
}
int blockChk(){
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
		case POPSTATE_:
			return 1;
		default:
			return 0;
	}
}
Stmt* if_stmt(){
	lookahead = nextToken(lval);
	Expr* cond = bool();
	match(THEN_);
	Stmt* body = block_stmt();
	Stmt* elseBody = NULL;
	if(lookahead == ELSIF_){
		elseBody= elsif_stmt();
	}
	else if(lookahead == ELSE_){
		lookahead = nextToken(lval);
		elseBody = block_stmt();
	}
	match(FI_);
//	if(DEBUG) fprintf(stderr, "%d\n", lookahead);
	return createIfStmt(cond, body, elseBody);
}
Stmt* elsif_stmt(){

	lookahead = nextToken(lval);
	Expr* cond = bool();
	match(THEN_);
	Stmt* body = block_stmt();
	Stmt* elseBody = NULL;
	if(lookahead == ELSIF_){
		elseBody= elsif_stmt();
	}
	if(lookahead == ELSE_){
		lookahead = nextToken(lval);
		elseBody= block_stmt();
	}
	return createElsifStmt(cond, body, elseBody);
}
Stmt* while_stmt(){
	lookahead = nextToken(lval);
	Expr* cond = bool();
	match(DO_);
	Stmt* body = block_stmt();
	match(OD_);
	return createWhileStmt(cond, body);
}
Stmt* createActionStmt(int action, Expr* rval) {
	Stmt* s = (Stmt*) malloc(sizeof(Stmt));
	s->type = ACTION_STMT;
	s->s.action_.action = action;
	s->s.action_.rval = rval;
	s->next = NULL;
	return s;
}
Stmt* createAssignStmt(SymTab* var, Expr* rval){
	Stmt* s = (Stmt*) malloc(sizeof(Stmt));
	s->type = ASSIGN_STMT;
	s->s.assign_.lval = var;
	s->s.assign_.rval = rval;
	s->next = NULL;
	return s;
}
Stmt* createBlockStmt(Stmt* list){
	Stmt* s = (Stmt*) malloc(sizeof(Stmt));
	s->type = BLOCK_STMT;
	s->s.block_.list = list;
	return s;
}
Stmt* createIfStmt(Expr* cond, Stmt* body, Stmt* elseBody){
	Stmt* s = (Stmt*) malloc(sizeof(Stmt));
	s->type = IF_STMT;
	s->s.if_.cond = cond;
	s->s.if_.body = body;
	s->s.if_.elseBody = elseBody;
	return s;
}
Stmt* createElsifStmt(Expr* cond, Stmt* body, Stmt* elseBody){
	Stmt *s = (Stmt*) malloc(sizeof(Stmt));
	s->type = ELSIF_;
	s->s.if_.cond = cond;
	s->s.if_.body = body;
	s->s.if_.elseBody = elseBody;
	return s;
}
Stmt* createWhileStmt(Expr* cond, Stmt* body){
	Stmt *s = (Stmt*) malloc(sizeof(Stmt));
	s->type = WHILE_STMT;
	s->s.while_.cond = cond;
	s->s.while_.body = body;
	return s;
}
float* getValue(char* name){//returns the value of a variable, or NULL if it does not exist
//	if(DEBUG) fprintf(stderr, "Entered getvalue\n");
//	if(DEBUG) fprintf(stderr, "looking for %s\n", name);
	if(symtab == NULL){
		return NULL;
	}
	SymTab* current = symtab;

	while(current->next != NULL){
//		if(DEBUG) fprintf(stderr, "checking:%s =%f\n",current->sym, current->val);
		if(strncmp(current->sym, name, strlen(name)) == 0){

			float* value = &current->val;
			if(DEBUG) fprintf(stderr, "%s =%f\n",name, *value);
			return value;
		}
		current = current->next;
	}
//	if(DEBUG) fprintf(stderr, "checking:%s =%f\n",current->sym, current->val);
	if(strncmp(current->sym, name, strlen(name)) == 0){
		float* value = &current->val;
		if(DEBUG) fprintf(stderr, "%s =%f\n",name, *value);
		return value;
	}
	return NULL;
}
float evalExpr(Expr *expr) {
//	SymTab* current = symtab;
//	if(DEBUG) fprintf(stderr, "here?\n");
//	while(current != NULL){
//		if(DEBUG) fprintf(stderr, "%s = %f\n",current->sym,current->val);
//		current = current->next;
//	}
	switch (expr->type) {
		case NUM_EXPR:
			return expr->op.num;
			break;
		case VAR_EXPR:
			return *getValue(expr->op.sym->sym);
			break;
		case ADD_EXPR:
			return evalExpr(expr->op.binary.left) + evalExpr(expr->op.binary.right);
			break;
		case SUB_EXPR:
			return evalExpr(expr->op.binary.left) - evalExpr(expr->op.binary.right);
			break;
		case NEG_EXPR:
			return -evalExpr(expr->op.unary);
			break;
		case MULT_TERM:
			return evalExpr(expr->op.binary.left) * evalExpr(expr->op.binary.right);
			break;
		case DIV_TERM:
			if(DEBUG) fprintf(stderr, "here?\n");
			return evalExpr(expr->op.binary.left) / evalExpr(expr->op.binary.right);
			break;
		case '=':
			if (evalExpr(expr->op.binary.left)== evalExpr(expr->op.binary.right)) return 1;
			else return 0;
			break;
		case '<':
			if (evalExpr(expr->op.binary.left)< evalExpr(expr->op.binary.right)) return 1;
			else return 0;
			break;
		case '>':
			if (evalExpr(expr->op.binary.left)> evalExpr(expr->op.binary.right)) return 1;
			else return 0;
			break;
		case NE_:
			if (evalExpr(expr->op.binary.left)!= evalExpr(expr->op.binary.right)) return 1;
			else return 0;
			break;
		case LE_:
			if (evalExpr(expr->op.binary.left)<= evalExpr(expr->op.binary.right)) return 1;
			else return 0;
			break;
		case GE_:
			if (evalExpr(expr->op.binary.left)>= evalExpr(expr->op.binary.right)) return 1;
			else return 0;
			break;
		case NOT_:
			if (evalExpr(expr->op.unary) == 0) return 1;
			else return 0;
			break;
		case AND_:
			if(evalExpr(expr->op.binary.left) && evalExpr(expr->op.binary.right)) return 1;
			else return 0;
			break;
		case OR_:
			if(evalExpr(expr->op.binary.left) || evalExpr(expr->op.binary.right)) return 1;
			else return 0;
			break;
		default:
			return 0;
			break;
	}
	return 0;
}
void match(int tok) {/* enforce match of current token, fetch next token*/
	if (tok == lookahead)
		lookahead = nextToken(lval);
	else
		fprintf(stderr, "token matching error \n");

}
void executeStmt(Stmt *stmt_) {
	switch (stmt_->type) {
		case ACTION_STMT:
			execActionStmt(stmt_);
			break;
		case ASSIGN_STMT:
			execAssignStmt(stmt_);
			break;
		case BLOCK_STMT:
			execBlockStmt(stmt_);
			break;
		case IF_STMT:
		case ELSIF_:
			execIfStmt(stmt_);
			break;
		case WHILE_STMT:
			execWhileStmt(stmt_);
			break;
		default:
			break;
	}
	return;
	//execute some logics or call turtle API
}
void execActionStmt(Stmt *stmt) {//Executes an action statement from the turtle API.

	int action = stmt->s.action_.action;
	float param;
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
			break;
	}
	return;
	//execute action statement by calling turtle API
}
void execAssignStmt(Stmt *stmt){
	char* name = stmt->s.assign_.lval->sym;
	float value = evalExpr(stmt->s.assign_.rval);
	if(symtab == NULL){//symbol table is empty
		symtab = malloc(sizeof(SymTab));
		symtab->sym = strdup(name);
		symtab->val = value;
		symtab->next = NULL;
		return;
	}
	SymTab* current = symtab;
	while(current->next != NULL){
		if(strncmp(current->sym, name, strlen(name)) ==0){
			current->val = value;
			break;
		}
			current = current->next;
	}
	if(strncmp(current->sym, name, strlen(name)) ==0){
		current->val = value;
	}
	else{
		SymTab* newVar = malloc(sizeof(SymTab));
		newVar->sym = strdup(name);
		newVar->val = value;
		newVar->next = NULL;
		current->next = newVar;
	}
	return;
}
void execBlockStmt(Stmt* stmt){
	Stmt* current = stmt->s.block_.list;
	while(current->next != NULL){
		executeStmt(current);
		current = current->next;
	}
	executeStmt(current);
}
void execIfStmt(Stmt* stmt){
	if(evalExpr(stmt->s.if_.cond) == 1){
		executeStmt(stmt->s.if_.body);
	}
	else if(stmt->s.if_.elseBody != NULL){
		executeStmt(stmt->s.if_.elseBody);
	}
}
void execWhileStmt(Stmt* stmt){
	int i=1;
	while(evalExpr(stmt->s.while_.cond)){
		if(DEBUG) fprintf(stderr, "loop %d\n", i);
		++i;
		executeStmt(stmt->s.while_.body);
	}
}
Expr* expr() {
//	if(DEBUG) fprintf(stderr, "expr\n");
	Expr* temp = term();
	Expr* e;
	e=temp;
	while(1){
//		if(DEBUG) fprintf(stderr, "expr loop\n");
		switch(lookahead){
			case '+':
				lookahead = nextToken(lval);//moves to next space past +
				e = malloc(sizeof(Expr));
				e->type= ADD_EXPR;
				e->op.binary.left = temp;
				e->op.binary.right = term();
				break;
			case '-':
				lookahead = nextToken(lval);//moves to next space past -
				e = malloc(sizeof(Expr));
				e->type= SUB_EXPR;
				e->op.binary.left = temp;
				e->op.binary.right = term();
				break;
			default:
				return e;
		}
	}
	return e;
}
Expr* term() {
//	if(DEBUG) fprintf(stderr, "term\n");
	Expr* temp = factor();
	Expr* e;
	e=temp;
	while(1){
//		if(DEBUG) fprintf(stderr, "term loop\n");
		switch(lookahead){
			case '*':
				lookahead = nextToken(lval);//moves past *
				e = malloc(sizeof(Expr));
				e->type= MULT_TERM;
				e->op.binary.left = temp;
				e->op.binary.right = term();
				break;
			case '/':
				lookahead = nextToken(lval);//moves past /
				e = malloc(sizeof(Expr));
				e->type= DIV_TERM;
				e->op.binary.left = temp;
				e->op.binary.right = term();
				break;
			default:
				return e;
		}
	}
	return e;
}
Expr* factor() {
//	if(DEBUG) fprintf(stderr, "factor\n");
	Expr* e;
	SymTab* tempSymTab;
	switch(lookahead){
		case REAL_:
//			if(DEBUG) fprintf(stderr, "real\n");
			e = malloc(sizeof(Expr));
			e->type= NUM_EXPR;
			e->op.num = lval->f;
			lookahead = nextToken(lval);//checks next token past #
			break;
		case IDENT_:
//			if(DEBUG) fprintf(stderr, "ident \n");
			e = malloc(sizeof(Expr));
			e->type= VAR_EXPR;
			tempSymTab = malloc(sizeof(SymTab));
			tempSymTab->sym = strdup(lval->s);
			tempSymTab->next = NULL;
			e->op.sym = tempSymTab;
			lookahead = nextToken(lval);
			break;
		case '-':
			lookahead = nextToken(lval);
			e = malloc(sizeof(Expr));
			e->type = NEG_EXPR;
			e->op.unary = expr();
			return e;
			break;
		case '(':
			lookahead = nextToken(lval);
			e=expr();
			match(')');
			return e;
			break;
		default:
			if(DEBUG) fprintf(stderr, "invalid right factor in factor()\n");
			return NULL;
			break;
	}
	return e;
}
Expr* bool(){//deals with ors
//	if(DEBUG) fprintf(stderr, "bool\n");
	Expr* temp = bool_term();
	Expr* e;
	e = temp;
	while(1){
//		if(DEBUG) fprintf(stderr, "bool loop\n");
		switch(lookahead){
			case OR_:
				lookahead = nextToken(lval);
				e = malloc(sizeof(Expr));
				e->type = OR_;
				e->op.binary.left = temp;
				e->op.binary.right = bool_term();
				temp = e;
				break;
			default:
				return e;
				break;
		}
	}
	return e;
}
Expr* bool_term(){//deals with ands
//	if(DEBUG) fprintf(stderr, "boolterm\n");
	Expr* temp = bool_factor();
	Expr* e;
	e=temp;
	while(1){
//		if(DEBUG) fprintf(stderr, "boolterm loop\n");
		switch(lookahead){
			case AND_:
				lookahead = nextToken(lval);
				e = malloc(sizeof(Expr));
				e->type = AND_;
				e->op.binary.left = temp;
				e->op.binary.right = bool_term();
				temp = e;
				break;
			default:
				return e;
				break;
		}
	}
	return e;
}
Expr* bool_factor(){//deals with nots, parens, and comparators
//	if(DEBUG) fprintf(stderr, "boolfactor\n");
	Expr* e;
	switch(lookahead){
		case NOT_:
			lookahead = nextToken(lval);
			e = malloc(sizeof(Expr));
			e->type = NOT_;
			e->op.unary = bool_factor();
			break;
		case '(':
			lookahead = nextToken(lval);
			e = bool();
			match(')');
			return e;
		default:
			e = cmp();
			break;
	}
	return e;
}
Expr* cmp(){
//	if(DEBUG) fprintf(stderr, "cmp\n");
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
			if(DEBUG) fprintf(stderr, "cmp():invalid comparator\n");
			return NULL;
			break;
	}
	return e;
}
void destroyStmt(Stmt* s){
	if(DEBUG) fprintf(stderr, "destroyStmt\n");
	if(s->next != NULL){
		destroyStmt(s->next);
	}
	Stmt* current, *next;
	switch(s->type){
		case ASSIGN_STMT:
			if(DEBUG) fprintf(stderr, "destroying assign\n");
			if(s->s.assign_.lval != NULL) destroySymtab(s->s.assign_.lval);
			if(s->s.assign_.rval != NULL) destroyExpr(s->s.assign_.rval);
			break;
		case WHILE_STMT:
			if(DEBUG) fprintf(stderr, "destroying while\n");
			if(s->s.while_.cond != NULL) destroyExpr(s->s.while_.cond);
			if(s->s.while_.body != NULL) destroyStmt(s->s.while_.body);
			break;
		case BLOCK_STMT:
			if(DEBUG) fprintf(stderr, "destroying block\n");
			if(s->s.while_.body != NULL) destroyStmt(s->s.block_.list);
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
			fprintf(stderr, "Cannot destroy stmt: undefined type\n");
			break;
	}
	if(DEBUG) fprintf(stderr, "freeing stmt\n");
	free(s);
}
void destroyExpr(Expr* e){
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
			break;
	}
	if(DEBUG) fprintf(stderr, "freeing expr\n");
	free(e);
}
void destroySymtab(SymTab* s){
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
char *strdup(const char *str){
    int n = strlen(str) + 1;
    char *dup = malloc(n * sizeof(char));
    if(dup)
    {
        strcpy(dup, str);
    }
    return dup;
}
