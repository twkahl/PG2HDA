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
File list.h

For convenience, doubly linked circular lists are systematically used in this project to 
implement structures such as sets, queues, etc.
************************************************************************************************/

#ifndef LIST_H
#define LIST_H
	
typedef struct list list;

struct list {	
	void *data;		/*content of the current node*/
	list *next, *prev;	/*pointers to the next and the previous node*/
};

list *InsertElement(void *p, list *l); /*Inserts element with data p in list l, returns head of the list*/
void Pop(list **l, void (*del)(void *)); /*Pops the head of the list, deletes data using the del function*/
void DeleteList(list **l, void (*del)(void *)); /*Deletes list, deletes data using the del function*/
int NumberOfElements(const list *l); /*Returns the number of elements in the list*/
int IsElement(const void *object, const list *l, int (*cmp)(const void *, const void *)); /*Returns 1 if object belongs to list and 0 otherwise*/
list *Intersection(const list *l1, const list *l2, int (*cmp)(const void *, const void *)); /*Computes the intersection of the two lists (sets), the elements of which are compared by the cmp function*/
void MergeLists(list **list1, const list *list2, int (*cmp)(const void *, const void *)); /*Merges list2 into list1 by appending the complement at the end, the elements of the lists are compared by the cmp function*/
void ConcatLists(list **list1, const list *list2); /*Appends list2 to list1*/
list *Product(const list *list1, const list *list2, int type); /*Produces the cartesian product of two lists of vectors of elements of the given type*/
list *Transpose(const list *veclist); /*veclist is seen as a matrix with vectors as lines, computes the transposite of this matrix, all vectors must be integer vectors of the same dimension*/
list *AsVectorList(const list *l); /*Transforms list into list of 1-dimensional vectors*/

#endif
