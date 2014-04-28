#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "list.h"
#define TRUE 1
#define FALSE 0

int main() {
//	axiom 1: isEmpty(createList) = true
	assert(isEmpty(createList()) == TRUE);
	printf("Passed axiom 1 \n\n");

//	axiom 2: isEmpty(prepend(l,e)) = false
	list* l1 = createList();
	int e1 = 1;
	assert(isEmpty(prepend(l1,&e1)) == FALSE);
	printf("passed axiom 2\n\n");

//	axiom 4a: head(append(l,e)) = if isEmpty(l) then e
	list* l2 =createList();
	if (isEmpty(l2)) {
		assert(head(append(l2, &e1)) == &e1);
		printf("Passed axiom 4a \n\n");
	}
	else printf("didnt test 4a, l2 wasnt empty somehow \n");

//  axiom 4b: head(append(l,e)) = else head(l)
	int e2 = 2;
	if (!isEmpty(l2)) {
		assert(head(append(l2, &e2)) != &e2);
		printf("Passed axiom 4b \n\n");
	}
	else printf("didnt test 4b, l2 was empty somehow \n");

//	axiom 6a: tail(prepend(l,e)) = if isEmpty(l) then createlist
	list* l3 = createList();
	if (isEmpty(l3)) {
		l3 = tail(prepend(l3, &e1));
		assert(!l3->front);
		printf("Passed axiom 6a \n\n");
	}
	else printf("didnt test 6a, l3 wasnt empty somehow \n");

//  axiom 6b: tail(prepend(l,e)) = else tail(l)
	if (!isEmpty(l2)){//l2 = (1,2)
		list* cdr= createList();
		cdr= tail(prepend(l2, &e2));
		assert(cdr->front);
		printf("Passed axiom 6b \n\n");
	}
	else printf("didnt test 6b, l2 was empty somehow \n");

//	axiom 6c: insert(l,i,e) = if !(i > length(l) + 1 or i<0) then insert(l,i,e)
	list* l4 = createList();
	l4= insert(l4, 0, &e1);
	assert(isEmpty(l4) == FALSE);
	printf("Passed axiom 6c \n\n");

//	axiom 8c: retrieve(l,i) = if !(i>length(l) or i < 0) then retrieve(l,i)
	int* ret;
	ret= (int*)retrieve(l4, 0);
	assert(*ret == e1);
	printf("Passed axiom 8c \n\n");

//	axiom 7c: remove(l,i) = if !(i>length(l) or i < 0) then remove(l,i)
	remove_(l4, 0);
	assert(isEmpty(l4) == TRUE);
	printf("Passed axiom 7c \n\n");



	while (1) {
		printf("axioms 3, 5, 7a, 7b, 8a, 8b, 9a & 9b will fail an assertion when they pass.\ninput a number between 1-8 to text an axiom\n1:axiom 3, 2:axiom 5, 3:axiom 7a, 4:axiom 7b, 5:axiom 8a, 6:axiom 8b, 7:axiom 9a, 8:axiom 9b\n");
		int x;
		scanf("%d", &x);
		switch (x) {
			case 1:
				//	axiom 3: head(createList) = error
				head(createList());//find head of empty list
				break;
			case 2:
				//	axiom 5: tail(createList) = error
				tail(createList());//find tail of empty list
				break;
			case 3:
				//	axiom 6a: insert(l,i,e) = if i > length(l) + 1 then error
				insert(l1, 9999999, &e1);//insert something way past the end of the list
				break;
			case 4:
				//  axiom 6b: insert(l,i,e) = if i < 0 then error
				insert(l1, -1, &e1);//insert something with a negative index
				break;
			case 5:
				//	axiom 7a: remove(l,i) = if i>length(l) then error
				remove_(l1, 9999999);//remove something way past the end of the list
				break;
			case 6:
				//  axiom 7b: remove(l,i) = if i < 0 then error
				remove_(l1, -1);//remove something with a negative index
				break;
			case 7:
				//	axiom 8a: retrieve(l,i) = if i>length(l) then error
				retrieve(l1, 9999999);//retrieve something way past the end of the list
				break;
			case 8:
				//	axiom 8b: retrieve(l,i) = if i < 0 then error
				retrieve(l1, -1);//retrieve something with a negative index
				break;
			default:
				printf("bad input. try again.\n");
		}
	}
	return 0;
}
