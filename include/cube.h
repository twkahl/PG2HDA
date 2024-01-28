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
File cube.h

This file declares cubes and functions for cubes.
************************************************************************************************/

#ifndef CUBE_H
#define CUBE_H

struct vector;
struct list;

typedef struct label label;
typedef struct flagfield flagfield;
typedef struct cube cube;

struct label {
	char *str;	/*label, e.g., of an action or a state*/
	int num;	/*some number, e.g., a process ID*/
};
struct flagfield {
	unsigned int ini : 1;	/*initial state*/
	unsigned int fin : 1; 	/*final state*/
};
struct cube {
	unsigned int degree; 
	flagfield flags;
	cube **d[2];	/*boundary operators*/
	struct list **s[2];	/*cofaces*/	
	struct vector *edges; /*the edges starting at the origin of the cube, which correspond to the actions whose independence is represented by the cube*/
	struct vector *lab; /*label*/ 
	struct list *cl;	/*pointer to cube list position*/	
	void *furtherdata; /*further data, e.g. a pointer to a cube used for reduction*/		
};

label *NewLabel(const char *str, int num); /*Creates new label*/
void DeleteLabel(void *lab); /*Deletes given label*/
int Labcmp(const void *l1, const void *l2); /*Returns 0 if the two labels exist and are equal*/
struct vector *MultiplyLabelVectors(const struct vector *lv1, const struct vector *lv2); /*Concatenates two label words*/
int *NewInt(int n); /*Creates new int with value n*/
void DeleteInt(void *n); /*Deletes given int*/
cube *NewCube(unsigned int d); /*Creates new cube of degree d*/
void DeleteCube(void *pc); /*Deletes the given cube*/
struct vector *Vertices(cube *pc); /*Computes the vertices of a cube, the initial and the final vertex come first*/
struct vector *Edges(cube *pc); /*Computes the edges of pc starting in the initial vertex of pc*/
int FillCubes(cube *edge, struct list *cubelist[]);	/*Completes HDA cubelist at given edge, returns the dimension of the highest cube added, returns -1 if no cube is added*/ 

#endif
