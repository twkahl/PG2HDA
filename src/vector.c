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
File vector.c

This file implements the functions declared in vector.h.
************************************************************************************************/

#include "vector.h"
#include <stdio.h>
#include <stdlib.h>
#include "def.h"


/*Creates new vector of given dimension and element size*/

vector *NewVector(unsigned int dim, unsigned int elsize) {

	vector *v;
	
	if ((v = malloc(sizeof(vector))) == NULL) {
		printf("NewVector: Out of memory!\n");
		exit(EXIT_FAILURE);
	}	
	if (dim > 0) {											
		if ((v->coord = malloc(dim * elsize)) == NULL) {
			printf("NewVector: Out of memory!\n");
			exit(EXIT_FAILURE);
		}				
	}	 
	else 
		v->coord = NULL;
	v->dim = dim;
	return v;
}


/*Deletes given vector*/

void DeleteVector(void *vec) {
	
	vector *v;
			
	if (vec) {
		v = (vector *) vec;
		if (v->coord) {
			free(v->coord);						
		}
		free(v);		
	}
}


/*Returns 0 iff the vectors of the given type are equal according to the coordinate comparison function cmp*/

int VecCmp(const vector *vec1, const vector *vec2, int type, int (*cmp)(const void *, const void *)) {

	int diff = 1, i;

	if (!vec1 && !vec2)
		diff = 0;
	else if (vec1 && vec2 && vec1->dim == vec2->dim) {
		diff = 0;	
		switch (type) {
			case INT: 
				for (i = 0; i < vec1->dim; i++) {
					if (((int *) vec1->coord)[i] != ((int *) vec2->coord)[i]) {
						diff = 1;
						break;
					}
				}
				break;			
			default: 
				if (cmp) {
					for (i = 0; i < vec1->dim; i++) {
						if (cmp(((void **) vec1->coord)[i], ((void **) vec2->coord)[i]) != 0) {
							diff = 1;
							break;
						}
					}					
				}
				else {
					for (i = 0; i < vec1->dim; i++) {
						if (((void **) vec1->coord)[i] != ((void **) vec2->coord)[i]) {
							diff = 1;
							break;
						}
					}
				}
				break;
		}
	}
	return diff;
}


/*Same as VecCmp((vector *) vec1, (vector *) vec2, INT, NULL)*/

int Veccmp(const void *vec1, const void *vec2) {

	return VecCmp((vector *) vec1, (vector *) vec2, INT, NULL);

}
