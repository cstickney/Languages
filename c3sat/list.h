#ifndef LIST_H
#define LIST_H

typedef struct lnode{
    void* elem;
    struct lnode *next;
}lnode;

typedef struct list{
    lnode *back;
    lnode *front;
}list;

list* createList();
int isEmpty(list* l );//returns 1 if empty, else 0
list* prepend(list* l, void* elem);//add element to front of list, returns ptr to new first element
list* append(list* l, void* elem);//add element to end of list
void* head(list* l);//gets the head of the list
list* tail(list* l);//gets the tail of the list
int length(list* l);//gets the length of the list
list* insert(list* l, int index, void* elem);//inserts elem at index, or returns null if out of range
list* remove_(list* l, int index);//removes elem at index, or returns null if out of range
void* retrieve(list* l, int index);//retrieves elem at index, or returns null if out of range
list* deepCopy(list* original);
#endif
