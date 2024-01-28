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
File io.h

This file declares functions for in and output.
************************************************************************************************/

#ifndef IO_H
#define IO_H

#include <stdio.h>

struct vector;
struct list;
struct programgraph;

void ReadPG(FILE *fp, struct list **varlist, struct programgraph *pg, int pid); /*Reads input from file*/
void PrintSystemHDA(const struct vector* pgvec, const struct list *varlist, struct list *const cubes[], int dim); /*Prints the system and its HDA model*/
void PrintChainComplex(struct list *const cubes[], int dim); /*Prints chain complex of HDA in Z_2 chomp format*/
void PrintHDA(struct list *const cubes[], int dim); /*Prints HDA in tsv format*/

#endif
