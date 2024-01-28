/*
	Copyright (c) 2018-2024 Thomas Kahl

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.
*/

/***********************************************************************************************
pg2hda - Compute higher-dimensional automata modeling concurrent systems given by program graphs
File list.c

This file implements the functions declared in list.h.
************************************************************************************************/

#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "def.h"
#include "vector.h"


/*Inserts element with data p in list l, returns head of the list*/

list *InsertElement(void *p, list *l) {

	list *newnode;

	if ((newnode = malloc(sizeof(list))) == NULL) {
		printf("InsertElement: Out of memory!\n");
		exit(EXIT_FAILURE);
	}	
	newnode->data = p;
	if (l) {	
		newnode->next = l;
		newnode->prev = l->prev;
		l->prev = newnode;
		newnode->prev->next = newnode;
	}
	else
		newnode->next = newnode->prev = newnode;
	return newnode->next;
}


/*Pops the head of the list, deletes data using the del function*/

void Pop(list **l, void (*del)(void *)) {

	list *help;	

	if (l) {	
		if (*l) {
			if (del) 
				del((*l)->data);
			(*l)->prev->next = (*l)->next;
			(*l)->next->prev = (*l)->prev;	
			if (*l != (*l)->next) {
				help = *l;
				*l = help->next;
				free(help);				
			}
			else {
				free(*l);				
				*l = NULL;
			}		
		}
	}	
} 


/*Deletes list, deletes data using the del function*/

void DeleteList(list **l, void (*del)(void *)) {

	if (l) {
		while (*l) {			
			Pop(l, del);			
		}		
	}
}


/*Returns the number of elements in the list*/

int NumberOfElements(const list *l) {

	int n = 0;
	const list *l2;
	
	if(l) {
		l2 = l;
		do {
			n = n+1;
			l2 = l2->next;
		} while (l2 != l);
	}
	return n;
}


/*Returns 1 if object belongs to list and 0 otherwise*/

int IsElement(const void *object, const list *l, int (*cmp)(const void *, const void *)) {

	const list *node;	
	int iselement = 0;
	
	if (l) {
		node = l;	
		do {	
			if (cmp) {			
				if (cmp(node->data, object) == 0) { 			 
					iselement = 1;
					break;
				}
			}
			else {
				if (node->data == object) {
					iselement = 1;
					break;
				}
			}
			node = node->next;
		} while (node != l);
	}
	return iselement;
}


/*Computes the intersection of the two lists (sets), the elements of which are compared by the cmp function*/

list *Intersection(const list *l1, const list *l2, int (*cmp)(const void *, const void *)) {

	list *caplist = NULL;
	const list *list1;
	
	if (l1 && l2) {
		list1 = l1;
		do {
			if (IsElement(list1->data, l2, cmp)) 
				caplist = InsertElement(list1->data, caplist);				
			list1 = list1->next;
		} while (list1 != l1);
	}
	return caplist;
} 


/*Merges list2 into list1 by appending the complement at the end, the elements of the lists are compared by the cmp function*/

void MergeLists(list **list1, const list *list2, int (*cmp)(const void *, const void *)) {

	const list *l1, *l2;	
	int isnew;	

	if (list2) {	
		l2 = list2;
		do {
			isnew = 1;
			if (*list1) {	
				l1 = *list1;
				do {
					if (cmp) {
						if (cmp(l2->data, l1->data) == 0) {
							isnew = 0;
							break;
						}
					}
					else {
						if (l2->data == l1->data) {
							isnew = 0;
							break;
						}
					}
					l1 = l1->next;
				} while (l1 != *list1);
			}
			if (isnew)	
				*list1 = InsertElement(l2->data, *list1);
			l2 = l2->next;
		} while (l2 != list2);
	}
}


/*Appends list2 to list1*/

void ConcatLists(list **list1, const list *list2) {
	
	const list *l;	
	
	if (list2) {	
		l = list2;
		do {		
			*list1 = InsertElement(l->data, *list1);
			l = l->next; 
		} while (l != list2);
	}
}


/*Produces the cartesian product of two lists of vectors of elements of the given type*/

list *Product(const list *list1, const list *list2, int type) {
	
	int i;
	const list *l1, *l2;
	list *prodlist = NULL;
	const vector *vec1, *vec2;
	vector *vec;
		
	if(list1 && list2) {
		l1 = list1;
		do {
			vec1 = (vector *) l1->data;
			l2 = list2;
			do {				
				vec2 = (vector *) l2->data;	
				switch (type) {
					case INT: 
						vec = NewVector(vec1->dim + vec2->dim, sizeof(int));
						for (i = 0; i < vec1->dim; i++)
							((int *) vec->coord)[i] = ((int *) vec1->coord)[i];
						for (i = 0; i < vec2->dim; i++)
							((int *) vec->coord)[i + vec1->dim] = ((int *) vec2->coord)[i];
						break;
					default:
						vec = NewVector(vec1->dim + vec2->dim, sizeof(void *));
						for (i = 0; i < vec1->dim; i++)
							((void **) vec->coord)[i] = ((void **) vec1->coord)[i];
						for (i = 0; i < vec2->dim; i++)
							((void **) vec->coord)[i + vec1->dim] = ((void **) vec2->coord)[i];
						break;
				} 
				prodlist = InsertElement(vec, prodlist);
				l2 = l2->next;
			} while (l2 != list2);
			l1 = l1->next;
		} while (l1 != list1);
	}
	return prodlist;
}

 
/*veclist is seen as a matrix with vectors as lines, computes the transposite of this matrix, all vectors must be integer vectors of the same dimension*/

list *Transpose(const list *veclist) {

	int i, j;
	const list *vlist;
	list *transpose = NULL;	
	vector *vec, *v;
	 
	if (veclist) {		 
		for (j = 0; j < ((vector *) veclist->data)->dim; j++) { 
			vec = NewVector(NumberOfElements(veclist), sizeof(int));
			i = 0;
			vlist = veclist;
			do {
				v = (vector *) vlist->data;
				((int *) vec->coord)[i++] = ((int *) v->coord)[j];
				vlist = vlist->next;
			} while (vlist != veclist);
			transpose = InsertElement(vec, transpose);
		}
	}
	return transpose;
} 


/*Transforms list into list of 1-dimensional vectors*/

list *AsVectorList(const list *l) {

	list *veclist = NULL;
	const list *vlist;
	vector *vec;
	
	if (l) {
		vlist = l;
		do {
			vec = NewVector(1, sizeof(void *));
			((void **) vec->coord)[0] = vlist->data;	
			veclist = InsertElement(vec, veclist);
			vlist = vlist->next;
		} while (vlist != l);
	}
	return veclist;
} 
