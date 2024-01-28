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
File main.c

This file contains the main function.
************************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "def.h"
#include "vector.h"
#include "list.h"
#include "pgraph.h"
#include "cube.h"
#include "io.h"
#include "hda.h"
#include "pml2pg.tab.h" 


extern void ParsePML(FILE *fp, list **varlist, programgraph *pgraph, list **sections, const int create);
unsigned int out = 0, inp = 0; 
 

int main(int argc, char *argv[]) {
	
	int i, filecount = 0, n, dim = 0;
	char inputfile[argc][STRL];	
	vector *pgvec, *hda;
	programgraph **pg;
	FILE *fp;
	list *varlist = NULL, *sections = NULL, **cubes;	
	
	for (i = 0; i < argc; i++) 
		strcpy(inputfile[i], "");								
/*options and input files*/
	for (i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-s") == 0) 
			out = OPTION_s;
		else if (strcmp(argv[i], "-i") == 0) 
			out = OPTION_i;
		else if (strcmp(argv[i], "-c") == 0) 
			out = OPTION_c;
		else if (strcmp(argv[i], "-t") == 0) 
			out = OPTION_t;		
		else if (strcmp(argv[i], "--old") == 0) 
			inp = OPTION_old;		
		else 
			strcpy(inputfile[filecount++], argv[i]);
	}
	if (filecount < 1) {
		printf("Error: no input file\n");
		exit(EXIT_FAILURE);
	}				
/*program graph input*/ 
    if (inp == OPTION_old) {    		    
	    n = filecount;
	    pgvec = NewVector(n, sizeof(programgraph *));
		pg = pgvec->coord;
		for (i = 0; i < n; i++) {
			pg[i] = NewPG();
			fp = fopen(inputfile[i], "r");
			if (!fp) {
				printf("Error: Unknown option or file: %s\n", inputfile[i]);
				exit(EXIT_FAILURE);
			}
			ReadPG(fp, &varlist, pg[i], n > 1 ? i : -1);
	 		fclose(fp); 	
		}	    	    	  	     
	}
	else {
		fp = fopen(inputfile[0], "r");
		if (!fp) {
			printf("Error: unknown option or file \"%s\"\n", inputfile[0]);
			exit(EXIT_FAILURE);
		}
		ParsePML(fp, &varlist, NULL, &sections, 0);
		fclose(fp); 	
		n = NumberOfElements(sections);
		pgvec = NewVector(n, sizeof(programgraph *));
		pg = pgvec->coord;
		for (i = 0; i < n; i++) {
			pg[i] = NewPG();				
			fp = fopen(inputfile[0], "r");				
			ParsePML(fp, &varlist, pg[i], &sections, 1);				
			fclose(fp);							
		}								
	} 	
/*HDA construction and output*/		
	hda = NewVector(n + 1, sizeof(list *));	
	cubes = (list **) hda->coord;		
	for (i = 0; i <= n; i++) 		
		cubes[i] = NULL;												
	if (out != OPTION_i) 
		dim = MakeHDA(pgvec, cubes, varlist);
	if (out == OPTION_c)
		PrintChainComplex(cubes, dim);																							 
	else if (out == OPTION_t)
		PrintHDA(cubes, dim);
	else 		    			
		PrintSystemHDA(pgvec, varlist, cubes, dim);												
/*clear memory*/
	for (i = 0; i < n; i++) 
		DeletePG(pg[i]);
	DeleteVector(pgvec);	
	DeleteList(&varlist, DeleteVariable);		
	for (i = 0; i <= dim; i++) 
		DeleteList(&cubes[i], DeleteCube);		
	DeleteVector(hda);			
	return 0;
}
