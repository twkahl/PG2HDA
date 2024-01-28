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
File hda.c

This file implements the functions declared in hda.h.
************************************************************************************************/

#include "hda.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 
#include "def.h"
#include "vector.h"
#include "list.h"
#include "pgraph.h"
#include "cube.h"


/*Function prototypes*/

static list *InitialStates(const vector *pgvec, const list *varlist); /*Computes the initial global states of the system of program graphs in pgvec*/
static int IsFinal(const state *st, const vector *pgvec, const list *varlist); /*Returns 1 if the state is a final state and 0 otherwise*/
static int CubesOfStates(const list *statelist, list *cubelist[]); /*Produces cubes associated with states, returns the number of cubes*/
static state *NextState(const state *sta, const transition *trans, int pid, const list *varlist); /*Computes the state after the given transition*/
static int HandleState(const state *sta, list **statelist, list *cubelist[], const list *varlist); /*Computes edges and states incident with given state and adds all possible higher-dimensional cubes, returns the dimension of the highest-dimensional cube added, returns -1 if no cube is added*/


/*Function implementations*/

/*Creates new state for pgs program graphs and vars variables*/

state *NewState(int pgs, int vars) {

	state *st;	

	if ((st = malloc (sizeof(state))) == NULL) {
		printf("NewState: Out of memory!\n");
		exit(EXIT_FAILURE);
	}	
	st->locvec = NewVector(pgs, sizeof(location *)); 
	st->valvec = NewVector(vars, sizeof(int));
	st->cube = NewCube(0);
	return st;
}


/*Deletes state (but not its cube)*/

void DeleteState(void *st) {
	
	state *s;

	if (st) {
		s = (state *) st;
		DeleteVector(s->valvec);
		DeleteVector(s->locvec);
		free(s);
	}
} 


/*Computes the initial global states of the system of program graphs in pgvec*/

static list *InitialStates(const vector *pgvec, const list *varlist) {

	condition *cond, *merge, *initialcond;
	const programgraph *const *const pg = pgvec->coord;
	int i, *valvar;
	const list *vlist, *node;
	vector *vec;
	list *dejavulist = NULL, *statelist = NULL;
	state *st;	
	const int varcount = NumberOfElements(varlist);
	char s[STRL*(pgvec->dim + varcount)], str[STRL];
	intvar *var;
	
	if (inp == OPTION_old) {
		initialcond = NewCondition();
		if (pgvec->dim == 1)
			cond = pg[0]->cond_0;
		else {
			cond = MergeConditions(pg[0]->cond_0, pg[1]->cond_0);		
			for (i = 2; i < pgvec->dim && cond->evallist; i++) {
				merge = MergeConditions(cond, pg[i]->cond_0);
				DeleteCondition(cond);
				cond = merge;			
			}
		}	
		if (cond->evallist) {
			ExtendCondition(cond, initialcond, varlist);	
			vlist = initialcond->evallist;
			do {
				vec = (vector *) vlist->data;
				if (!IsElement(vec, dejavulist, Veccmp)) {				
					dejavulist = InsertElement(vec, dejavulist);
					st = NewState(pgvec->dim, varcount);	
					for (i = 0; i < pgvec->dim; i++) 
						((location **) st->locvec->coord)[i] = pg[i]->loc_0;	
					valvar = st->valvec->coord;		
					for (i = 0; i < varcount; i++) 
						valvar[i] = ((int *) vec->coord)[i];			
					strcpy(s, "(");
					for (i = 0; i < pgvec->dim; i++) {
						sprintf(str, "%d,", pg[i]->loc_0->index);
						strcat(s, str);
					}
					for (i = 0; i < varcount - 1; i++) {
						sprintf(str, "%d,", valvar[i]);
						strcat(s, str);
					}
					sprintf(str, "%d)", valvar[varcount - 1]);
					strcat(s, str);
					st->cube->lab = NewVector(1, sizeof(label *));
					((label **) st->cube->lab->coord)[0] = NewLabel(s, -1);
					st->cube->flags.ini = 1;
					statelist = InsertElement(st, statelist);
				}
				vlist = vlist->next;
			} while (vlist != initialcond->evallist);
		}
		DeleteList(&dejavulist, NULL);	
		if (pgvec->dim > 1)
			DeleteCondition(cond);
		DeleteCondition(initialcond);						
	}
	else {
		st = NewState(pgvec->dim, varcount);	
		for (i = 0; i < pgvec->dim; i++) 
			((location **) st->locvec->coord)[i] = pg[i]->loc_0;	
		valvar = (int *) st->valvec->coord;
		node = varlist;
		i = 0;
		do {
			var = (intvar *) node->data;
			valvar[i] = var->initialval;
			i++;
			node = node->next;
		} while (node != varlist);							
		strcpy(s, "(");
		for (i = 0; i < pgvec->dim; i++) {
			sprintf(str, "%d,", pg[i]->loc_0->index);
			strcat(s, str);
		}
		for (i = 0; i < varcount - 1; i++) {
			sprintf(str, "%d,", valvar[i]);
			strcat(s, str);
		} 
		sprintf(str, "%d)", valvar[varcount - 1]);
		strcat(s, str);
		st->cube->lab = NewVector(1, sizeof(label *));
		((label **) st->cube->lab->coord)[0] = NewLabel(s, -1);
		st->cube->flags.ini = 1;
		statelist = InsertElement(st, statelist);		
	}	
	return statelist;
} 


/*Returns 1 if the state is a final state and 0 otherwise*/

static int IsFinal(const state *st, const vector *pgvec, const list *varlist) {

	int i, isfinal = 1;
	const programgraph *const *const pg = pgvec->coord;

	for (i = 0; i < pgvec->dim && isfinal; i++) {
		if (((location **) st->locvec->coord)[i] != pg[i]->loc_1) 
			isfinal = 0;
	}
	for (i = 0; i < pgvec->dim && isfinal; i++) 
		isfinal = CheckCondition(st->valvec, pg[i]->cond_1, varlist);
	return isfinal; 
}


/*Produces cubes associated with states, returns the number of cubes*/

static int CubesOfStates(const list *statelist, list *cubelist[]) {

	const list *slist;
	const state *st;
	int n = 0;

	if (statelist) {
		slist = statelist;
		do {
			st = (state *) slist->data;
			cubelist[0] = InsertElement(st->cube, cubelist[0]);
			st->cube->cl = cubelist[0]->prev;	
			n++;
			slist = slist->next;
		} while (slist != statelist);
	}
	return n; 
}


/*Computes the state after the given transition*/

static state *NextState(const state *sta, const transition *trans, int pid, const list *varlist) {

	state *st = NULL;
	const int pgs = sta->locvec->dim, *val, varcount = NumberOfElements(varlist);	
	const location **loc;
	int i;
	char s[STRL*(pgs + varcount)], str[STRL];

	if (CheckCondition(sta->valvec, trans->cond, varlist) == 1) {				
		st = NewState(pgs, NumberOfElements(varlist));
		loc = st->locvec->coord; 
		for (i = 0; i < pgs; i++)
			loc[i] = ((location **) sta->locvec->coord)[i];
		loc[pid] = trans->loc[1];		
		Effect(trans->act, sta->valvec, st->valvec, varlist);
		strcpy(s, "(");
		for (i = 0; i < pgs; i++) {
			sprintf(str, "%d,", loc[i]->index);
			strcat(s, str);
		}
		val = st->valvec->coord;
		for (i = 0; i < st->valvec->dim - 1; i++) {
			sprintf(str, "%d,", val[i]);
			strcat(s, str);
		}
		sprintf(str, "%d)", val[st->valvec->dim - 1]);
		strcat(s, str);
		st->cube->lab = NewVector(1, sizeof(label *));
		((label **) st->cube->lab->coord)[0] = NewLabel(s, -1);
	}	
	return st;
} 


/*Computes edges and states incident with given state and adds all possible higher-dimensional cubes, returns the dimension of the highest-dimensional cube added, returns -1 if no cube is added*/

static int HandleState(const state *sta, list **statelist, list *cubelist[], const list *varlist) {
		
	int i, isnew, dim = -1, d;	
	const location **loc = sta->locvec->coord;
	const list *tlist, *clist;
	const transition *trans;
	state *st;
	cube *cpc, *pc;	

	for (i = 0; i < sta->locvec->dim; i++) {		
		if (loc[i]->outtranslist) {				
			tlist = loc[i]->outtranslist; 
			do {
				trans = (transition *) tlist->data;															
				if ((st = NextState(sta, trans, i, varlist))) {							    
/*check whether state is new*/
					isnew = 1;											
					if (cubelist[0]) {					
						clist = cubelist[0];						
						do {						
							cpc = (cube *) clist->data;							
							if (VecCmp(cpc->lab, st->cube->lab, PTR, Labcmp) == 0) {							
								isnew = 0;
								DeleteCube(st->cube);
								st->cube = cpc;
								break;
							}
							clist = clist->next;
						} while (clist != cubelist[0]);
					}										
/*create edge for the transition*/
					pc = NewCube(1);										
					pc->d[0][0] = sta->cube;
					pc->d[1][0] = st->cube;					
					((cube **) pc->edges->coord)[0] = pc;
					pc->lab = NewVector(1, sizeof(label *));
					((label **) pc->lab->coord)[0] = NewLabel(trans->act->id, i);					
					cubelist[1] = InsertElement(pc, cubelist[1]);
					pc->cl = cubelist[1]->prev;					
					sta->cube->s[0][0] = InsertElement(pc, sta->cube->s[0][0]);
					st->cube->s[1][0] = InsertElement(pc, st->cube->s[1][0]);									
					dim = 1 > dim ? 1 : dim;												
/*if state is new, insert state and its cube, otherwise fill cubes at new edge*/
					if (isnew) {
						*statelist = InsertElement(st, *statelist);						
						cubelist[0] = InsertElement(st->cube, cubelist[0]);
						st->cube->cl = cubelist[0]->prev;																		
					}				
					else {							
						DeleteState(st);
						d = FillCubes(pc, cubelist);
						dim = d > dim ? d : dim;
					}					
				}			
				tlist = tlist->next;
			} while (tlist != loc[i]->outtranslist);
		}
	}
	return dim;	
} 


/*Transforms system of program graphs (pgvec) over the variables in varlist into HDA cubelist, returns the dimension of the HDA*/

int MakeHDA(const vector *pgvec, list *cubelist[], const list *varlist) {
	
	list *queue = NULL; 
	const state *sta;
	int d, dim = 0;		

	queue = InitialStates(pgvec, varlist);			
	CubesOfStates(queue, cubelist);	
	while (queue) {	
		sta = (state *) queue->data;
		if (IsFinal(sta, pgvec, varlist))
			sta->cube->flags.fin = 1; 								
		d = HandleState(sta, &queue, cubelist, varlist);										
		dim = (d > dim) ? d : dim;
		Pop(&queue, DeleteState);
	}
	return dim;	
}
