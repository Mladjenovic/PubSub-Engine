#ifndef _LIST_H
#define _LIST_H


struct listitem {
	int data;
	struct listitem *next;
};

typedef struct listitem Listitem;
struct list {
	Listitem *head;
};


typedef struct list List;
void initlist(List *); /* initialize an empty list */
void insertfront(List *, int val); /* insert val at front */
void insertback(List *, int val); /* insert val at back */
int length(List); /* returns list length */
void destroy(List *); /* deletes list */
void setitem(List *, int n, int val);/* modifies item at n to val*/
int getitem(List, int n); /* returns value at n*/
void displaylist(List*); /* displays the entire list*/

#endif /* _LIST_H */