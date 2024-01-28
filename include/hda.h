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
File hda.h

This file declares states, functions to create and delete states, and the MakeHDA function for 
the computation of HDAs from program graphs.
************************************************************************************************/

#ifndef HDA_H
#define HDA_H

struct vector;
struct list;
struct location;
struct programgraph;
struct cube;

typedef struct state state;

struct state {
	struct vector *locvec;	/*locations of the differnt processes*/
	struct vector *valvec;	/*values of the different variables*/	
	struct cube *cube;	/*associated vertex*/
};

state *NewState(int pgs, int vars); /*Creates new state for pgs program graphs and vars variables*/
void DeleteState(void *st); /*Deletes state (but not its cube)*/
int MakeHDA(const struct vector *pgvec, struct list *cubelist[], const struct list *varlist); /*Transforms system of program graphs (pgvec) over the variables in varlist into HDA cubelist, returns the dimension of the HDA*/

#endif
