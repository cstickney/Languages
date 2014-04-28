#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "list.h"

list* createList(){;// Create a list.
	list* l = malloc(sizeof(list));
	if(!l) return NULL;
	l->back= NULL;
	l->front= NULL;
	return l;
}
int isEmpty(list* l ){//returns 1 if empty, else 0
	if(!l->front) return 1;
	else return 0;
}
list* prepend(list* l, void* elem){//add element to front of list, returns ptr to new first element
	lnode* x = malloc(sizeof(lnode));
	x->elem = elem;
	if(l->front){//if there was an old front, link to it
		x->next =l->front;
	}
	l->front = x;//set elem as the new front
	if(!l->back){//if there was no back, make it the back too
		l->back = x;
	}
	return l;
}
list* append(list* l, void* elem){//add element to end of list
	lnode* x = malloc(sizeof(lnode));
	x->elem = elem;
	if(l->back){//if there is an old back, attach elem to it
		l->back->next = x;
	}
	l->back = x;//make elem the new back
	if(!l->front){//if there was no front, make it the front too
		l->front = x;
	}
	return l;
}
void* head(list* l){//gets the head of the list
	assert(l->front);//errors if list is empty
	return l->front->elem;//returns the head
}
list* tail(list* l){//gets the tail of the list
	assert(l->front);//errors if list is empty
	assert(l->back);
	list* cdr = createList();
	if(length(l)>1){//if the front is not the back
		cdr->back=l->back;
		cdr->front = l->front->next;
	}
	else{//the front is the back
		cdr->front=NULL;
		cdr->back=NULL;
	}
	return cdr;//returns the tail
}
int length(list* l){//gets the length of the list
	lnode* x;
	x = l->front;//points x at the first element of l
	int i=0;
	if(x != NULL){//if not empty
		if((int*)l->front == (int*)l->back){//if single element
			return 1;
		}
		else for(i=1;x->next != NULL; i++){//count the elements
			x =x->next;//if list isnt empty iterate until the end is found.
		}
	}
	return i;
}
list* insert(list* l, int index, void* elem){//inserts elem at index, or errors if out of range
	assert(index <= length(l));//index cannot be more than the list length, which is one more than the current max index
	assert(!(index <0));//index cannot be negative

	if(index == length(l)){//1 past end of list
		append(l, elem);
	}
	else if(index !=0 ){//middle of list

		lnode* x = malloc(sizeof(lnode));//node to insert
		x->elem = elem;

		lnode* y;//iterator
		y= l->front;//point it at the first node of l

		int i;
		for(i=0; i< index-1; i++) y = y->next;//get to one before the index.

		x->next = y->next;//attach the old index to x
		y->next = x;//attach x to the prior entry
	}
	else{//beginning of list
		prepend(l,elem);
	}
	return l;
}
list* remove_(list* l, int index){//removes elem at index, or errors if out of range
	assert(index < length(l));//index cannot be more outside of the list bounds
	assert(!(index <0));//index cannot be negative
	if(index !=0 ){//middle of array
		lnode* y = malloc(sizeof(lnode));
		y = l->front;
		int i;
		for(i=0; i< index-1; i++) y = y->next;//get to one before the index.
		y->next = y->next->next;
	}
	else{//beginning of array
		l->front = l->front->next;
	}
	return l;
}
void* retrieve(list* l, int index){//retrieves elem at index, or returns null if out of range
	assert(index < length(l));//index cannot be more outside of the list bounds
	assert(!(index <0));//index cannot be negative
	lnode* y = malloc(sizeof(lnode));
	int i;
	y=l->front;
	for(i=0; i< index; i++) y = y->next;//move to specified index
	return y->elem;//return elem at index
}
