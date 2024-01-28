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
File vector.h

A vector is an array whose dimension may be defined dynamically. This file defines vectors and 
declares basic functions for them.
************************************************************************************************/

#ifndef VECTOR_H
#define VECTOR_H

typedef struct vector vector;

struct vector {
	unsigned int dim;	/*dimension of the vector*/		
	void *coord;	/*The ith coordinate of a vector v of elements of type t is ((t *) v->coord)[i]*/
};

vector *NewVector(unsigned int dim, unsigned int elsize); /*Creates new vector of given dimension and element size*/
void DeleteVector(void *vec); /*Deletes given vector*/
int VecCmp(const vector *vec1, const vector *vec2, int type, int (*cmp)(const void *, const void *)); /*Returns 0 iff the vectors of the given type are equal according to the coordinate comparison function cmp*/
int Veccmp(const void *vec1, const void *vec2); /*Same as VecCmp((vector *) vec1, (vector *) vec2, INT, NULL)*/

#endif
