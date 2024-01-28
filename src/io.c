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
File io.c

This file implements the functions declared in io.h.	
************************************************************************************************/

#include "io.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>	
#include "def.h"
#include "vector.h"
#include "list.h"
#include "pgraph.h"
#include "cube.h"


#define LINELENGTH 1000 /*maximal length of a line in an input file*/


/*Function prototypes*/

static void ReadVariables(FILE *fp, list **varlist, programgraph *pg); /*Reads variables from file*/
static void ReadLocations(FILE *fp, programgraph *pg); /*Reads locations frome file*/
static void ReadActions(FILE *fp, programgraph *pg, int pid); /*Reads actions from file*/
static void ReadTransitions(FILE *fp, programgraph *pg, int pid); /*Reads transitions from file*/
static void ReadState(FILE *fp, programgraph *pg, int evals, location **loc, condition *cond); /*Reads initial or final state from file*/
static void PrintVariables(const list *varlist); /*Prints variables*/
static void PrintActions(const list *actlist); /*Prints actions*/
static void PrintAst(const ast *exp); /*Prints expression*/
static void PrintCondition(const condition *cond); /*Prints condition*/
static void PrintTransitions(const list *translist); /*Prints transitions*/
static void PrintPG(const programgraph *pg); /*Prints program graph*/
static void PrintDegree(const list *cubelist, int d); /*Prints cubes of degree d*/ 
static void PrintSageLabel(const cube *pc); /*Prints label of pc for use in Sage*/


/*Function implementations*/

/*Reads variables from file*/

static void ReadVariables(FILE *fp, list **varlist, programgraph *pg) {
	
	char line[LINELENGTH], str[STRL];
	int i, d, j;
	const list *node;
	intvar *var, *va;	

	for(i = 0; i < pg->no_vars; i++) {
		fscanf(fp, "%s", str);
		fgets(line, LINELENGTH, fp);
		fscanf(fp, "%i", &d);
		fgets(line, LINELENGTH, fp);
		var = NewVariable(d);
		strcpy(var->id, str);
		for (j = 0; j < d; j++) {
			fscanf(fp, "%i", &((int *) var->domain->coord)[j]);
			fgets(line, LINELENGTH, fp);
		}		
		if (*varlist) {
			node = *varlist;
			do {
				va = (intvar *) node->data;
				if (strcmp(va->id, var->id) == 0) {						
					if (Veccmp(va->domain, var->domain) != 0) {
						printf("Error: Variable %s is not well defined!\n", va->id);
						exit(EXIT_FAILURE);
					}					
					pg->varlist = InsertElement(va, pg->varlist);
					DeleteVariable(var);
					var = NULL;					
					break;
				}
				node = node->next;
			} while (node != *varlist);
		}
		if (var) {
			*varlist = InsertElement(var, *varlist);
			pg->varlist = InsertElement(var, pg->varlist);
		}
	}					
}


/*Reads locations from file*/

static void ReadLocations(FILE *fp, programgraph *pg) {
	
	char line[LINELENGTH];
	int  i, nolocs;		
	location *loc;
	
	fscanf(fp, "%i", &nolocs);
	fgets(line, LINELENGTH, fp);
	for (i = 0; i < nolocs; i++) {
		loc = NewLocation();
		loc->index = i;
		pg->loclist = InsertElement(loc, pg->loclist);
	}				
}


/*Reads actions from file*/

static void ReadActions(FILE *fp, programgraph *pg, int pid) {
	
	char line[LINELENGTH], str[STRL];
	int i, j, novars, noevals, k;
	const list *node;
	intvar *va;
	action *act;	

	for (i = 0; i < pg->no_acts; i++) {
		fscanf(fp, "%s", str);
		fgets(line, LINELENGTH, fp);
		fscanf(fp, "%i", &novars);
		fgets(line, LINELENGTH, fp);
		fscanf(fp, "%i", &noevals);		
		fgets(line, LINELENGTH, fp);
		act = NewAction(novars, noevals);
		strcpy(act->id, str);
		if (pid >= 0) {
			sprintf(str, "__%i", pid);
			strcat(act->id, str);
		}
		for (j = 0; j < act->no_vars; j++) {
			fscanf(fp, "%s", str);
			fgets(line, LINELENGTH, fp);
			if (pg->varlist) {
				node = pg->varlist;
				do {
					va = (intvar *) node->data;
					if (strcmp(va->id, str) == 0) {
						act->varlist = InsertElement(va, act->varlist);
						break;
					}
					node = node->next;	
				} while (node != pg->varlist);
			}
		}
		for (j = 0; j < act->no_evals; j++) {
			for (k = 0; k < act->no_vars; k++) {
				fscanf(fp, "%i", &act->map[0][j][k]);
				fgets(line, LINELENGTH, fp);
			}
			for (k = 0; k < act->no_vars; k++) {
				fscanf(fp, "%i", &act->map[1][j][k]);
				fgets(line, LINELENGTH, fp);
			}
		}
		pg->actlist = InsertElement(act, pg->actlist);
	}	

}


/*Reads transitions from file*/

static void ReadTransitions(FILE *fp, programgraph *pg, int pid) {
	
	char line[LINELENGTH], str[STRL], s[STRL];
	int i, j, novars, noevals, k, locindex;
	const list *node;
	intvar *va;
	location *loc;
	action *act;
	transition *trans;
	vector *ev;		

	for (i = 0; i < pg->no_trans; i++) {
		fscanf(fp, "%i", &novars);
		fgets(line, LINELENGTH, fp);
		fscanf(fp, "%i", &noevals);
		fgets(line, LINELENGTH, fp);
		trans = NewTransition();
		trans->cond->no_vars = novars;
		fscanf(fp, "%i", &locindex);
		fgets(line, LINELENGTH, fp);
		if (pg->loclist) {
			node = pg->loclist;
			do {
				loc = (location *) node->data;
				if (locindex == loc->index) {
					trans->loc[0] = loc;
					loc->outtranslist = InsertElement(trans, loc->outtranslist);
					break;
				}	
				node = node->next;
			} while (node != pg->loclist);
		}
		fscanf(fp, "%i", &locindex);
		fgets(line, LINELENGTH, fp);
		if (pg->loclist) {
			node = pg->loclist;
			do {
				loc = (location *) node->data;
				if (locindex == loc->index) {
					trans->loc[1] = loc;
					loc->intranslist = InsertElement(trans, loc->intranslist);
					break;
				}	
				node = node->next;
			} while (node != pg->loclist);
		}
		if (trans->cond->no_vars > 0) {
			for (j = 0; j < trans->cond->no_vars; j++) {
				fscanf(fp, "%s", str);
				fgets(line, LINELENGTH, fp);
				if (pg->varlist) {
					node = pg->varlist;
					do {
						va = (intvar *) node->data;
						if (strcmp(va->id, str) == 0) {
							trans->cond->varlist = InsertElement(va, trans->cond->varlist);
							break;
						}
					node = node->next;
					} while (node != pg->varlist);
				}
			}
			for (j = 0; j < noevals; j++) {
				ev = NewVector(trans->cond->no_vars, sizeof(int));
				for (k = 0; k < trans->cond->no_vars; k++) {
					fscanf(fp, "%i", &((int *) ev->coord)[k]);
					fgets(line, LINELENGTH, fp);
				}
				trans->cond->evallist = InsertElement(ev, trans->cond->evallist);
			}
			fscanf(fp, "%s", trans->cond->id);
			fgets(line, LINELENGTH, fp);
			if (noevals == 0) {		/*unsatisfiable conditions are not allowed*/
				DeleteCondition(trans->cond);
				trans->cond = NewCondition();
			}	
		}
		if (trans->cond->no_vars == 0) 
			trans->cond->evallist = InsertElement(NewVector(0,  sizeof(int)), trans->cond->evallist);		
		fscanf(fp, "%s", str);
		fgets(line, LINELENGTH, fp);
		if (pid >= 0) {
			sprintf(s, "__%i", pid);
			strcat(str, s);
		}
		if (pg->actlist) {
			node = pg->actlist;
			do {
				act = (action *) node->data;
				if (strcmp(act->id, str) == 0) {
					trans->act = act;
					break;
				}
				node = node->next;
			} while (node != pg->actlist);
		}
		pg->translist = InsertElement(trans, pg->translist);				
	}
}


/*Reads initial or final state from file*/

static void ReadState(FILE *fp, programgraph *pg, int evals, location **loc, condition *cond) {
	
	char line[LINELENGTH], str[STRL];
	int i, j, locindex;
	const list *node;
	intvar *va;	
	vector *ev;	
				
	fscanf(fp, "%i", &locindex);
	fgets(line, LINELENGTH, fp);
	if (pg->loclist) {
		node = pg->loclist;
		while (((location *) node->data)->index != locindex) 
			node = node->next;
		*loc = (location *) node->data;
	}
	if (cond->no_vars > 0) {
		for (i = 0; i < cond->no_vars; i++) {
			fscanf(fp, "%s", str);
			fgets(line, LINELENGTH, fp);
			if (pg->varlist) {
				node = pg->varlist;
				do {
					va = (intvar *) node->data;
					if (strcmp(va->id, str) == 0) {
						cond->varlist = InsertElement(va, cond->varlist);
						break;
					}
					node = node->next;
				} while (node != pg->varlist);
			}				
		}
		for (i = 0; i < evals; i++) {
			ev = NewVector(cond->no_vars, sizeof(int));
			for(j = 0; j < cond->no_vars; j++) {
				fscanf(fp, "%i", &((int *) ev->coord)[j]);
				fgets(line, LINELENGTH, fp);
			}
			cond->evallist = InsertElement(ev, cond->evallist);			
		}
		fscanf(fp, "%s", cond->id);
		fgets(line, LINELENGTH, fp);
		if (evals == 0) {
			DeleteCondition(cond);
			cond = NewCondition();
		}	
	}
	if (cond->no_vars == 0) 
		cond->evallist = InsertElement(NewVector(0, sizeof(int)), cond->evallist);
} 


/*Reads input from file*/

void ReadPG(FILE *fp, list **varlist, programgraph *pg, int pid) {
	
	char line[LINELENGTH];
	int cond0noevals, final, cond1noevals;
						
	fscanf(fp, "%i", &pg->no_vars);
	fgets(line, LINELENGTH, fp);
	fscanf(fp, "%i", &pg->no_acts);
	fgets(line, LINELENGTH, fp);
	fscanf(fp, "%i", &pg->no_trans);
	fgets(line, LINELENGTH, fp);
	fscanf(fp, "%i", &pg->cond_0->no_vars);
	fgets(line, LINELENGTH, fp);
	fscanf(fp, "%i", &cond0noevals);
	fgets(line, LINELENGTH, fp);
	fscanf(fp, "%i", &final);
	fgets(line, LINELENGTH, fp);
	fscanf(fp, "%i", &pg->cond_1->no_vars);
	fgets(line, LINELENGTH, fp);
	fscanf(fp, "%i", &cond1noevals);
	fgets(line, LINELENGTH, fp);
	fscanf(fp, "%s", pg->id);
	fgets(line, LINELENGTH, fp);
	ReadVariables(fp, varlist, pg);
	ReadLocations(fp, pg);
	ReadActions(fp, pg, pid);
	ReadTransitions(fp, pg, pid);	
	ReadState(fp, pg, cond0noevals, &pg->loc_0, pg->cond_0);
	if (final) 
		ReadState(fp, pg, cond1noevals, &pg->loc_1, pg->cond_1);

} 


/*Prints variables*/

static void PrintVariables(const list *varlist) {

	const list *vlist;
	const intvar *var;
	int i;
	
	if (varlist) {
		vlist = varlist;
		do {		
			if ((var = (intvar *) vlist->data)) {
				if (inp == OPTION_old) {
					printf("%s\tdomain:\t\t", var->id);
					if (var->domain->coord) {
						for (i = 0; i < var->domain->dim; i++)
							printf("%i\t", ((int *) var->domain->coord)[i]);
						printf("\n");
					}
				}
				else 
					printf("int %s\n", var->id);				 									
			}
			vlist = vlist->next;
		} while (vlist != varlist);
	}
}


/*Prints actions*/

static void PrintActions(const list *actlist) {

	const int noacts = NumberOfElements(actlist);
	const list *alist, *vlist;
	const action *act;
	const intvar *var;	
	int i, j;
	char name[STRL*noacts];
	
	if (actlist) {
		alist = actlist;
		do {			
			if ((act = (action *) alist->data)) {
				printf("%s", act->id);
				if (inp == OPTION_old) {
					printf("\n");
					if (act->varlist) {
						vlist = act->varlist;
						do {
							if ((var = (intvar *) vlist->data))						
								printf("\t%s", var->id);
							vlist = vlist->next;
						} while (vlist != act->varlist);
						printf("\n");
					}
					for (i = 0; i < act->no_evals; i++) {
						for(j = 0; j < act->no_vars; j++) 
							printf("\t%i", act->map[0][i][j]);				
						printf("\t->");
						for(j = 0; j < act->no_vars; j++) {
							printf("\t%i", act->map[1][i][j]);
						}
						printf("\n");
					}
				}
				else {
					strcpy(name, ""); /*redundant*/
					ActionName(name, act);					
					printf("\n   %s\n", name);	
				}	
				printf("\n");
			}
			alist = alist->next;
		} while (alist != actlist);
	}
}


/*Prints expression*/

static void PrintAst(const ast *exp) {

	char *ops[15] = { "", "+", "-", "*", "/", "%", "==", "!=", "<", "<=", ">", ">=", "||", "&&", "!" };

	if (exp) {
		if (exp->par) {		
			switch (exp->type) {
				case VAR:
					printf("(%s)", exp->node.var->id);
					break;
				case NUM:
					printf("(%d)", exp->node.num);
					break;
				case NUMEXP:
					printf("%d", Evaluate(exp, NULL, NULL));
					break;	
				default:
					printf("(");														
					PrintAst(exp->l);
					printf("%s", ops[exp->node.op]);
					PrintAst(exp->r);
					printf(")");							
					break;						
			}		
		}
		else {
			switch (exp->type) {
				case VAR:
					printf("%s", exp->node.var->id);
					break;
				case NUM:
					printf("%d", exp->node.num);
					break;
				case NUMEXP:
					printf("%d", Evaluate(exp, NULL, NULL));
					break;	
				default:
					if ((exp->node.op == EQ) && (exp->l->type == NUM) && (exp->r->type == NUM) && (exp->l->node.num == 0) && (exp->r->node.num == 0))
						printf("true");
					else {																
						PrintAst(exp->l);
						printf("%s", ops[exp->node.op]);
						PrintAst(exp->r);
					}							
					break;						
			}
		}
	}		
}


/*Prints condition*/

static void PrintCondition(const condition *cond) {
	
	const list *node; 
	const intvar *var;
	const vector *vec;
	int i;
		
	if (inp == OPTION_old) {
		if (cond && cond->no_vars > 0) {
			printf("%s\n", cond->id);
			if (cond->varlist) {
				node = cond->varlist;
				do {
					if ((var = (intvar *) node->data))							
						printf("\t%s", var->id);
					node = node->next;
				} while (node != cond->varlist);
			}
			printf("\n");
			if (cond->evallist) {
				node = cond->evallist;
				do {
					if ((vec = (vector *) node->data)) {
						if (vec->coord) {
							for (i = 0; i < cond->no_vars; i++)
								printf("\t%i", ((int *) vec->coord)[i]);
							printf("\n");
						}
					}
					node = node->next;
				} while (node != cond->evallist);
			}
		}
		else
			printf("true\n");
	}
	else 
		PrintAst(cond->exp);
}


/*Prints transitions*/

static void PrintTransitions(const list *translist) {

	const list *tlist;
	const transition *trans;

	if (translist) {
		tlist = translist;
		do {
			if ((trans = (transition *) tlist->data)) {			
				printf("start location:");
				if (trans->loc[0])
					printf("\t%i", trans->loc[0]->index);				
				printf("\nend location:");
				if (trans->loc[1])
					printf("\t%i\n", trans->loc[1]->index);				
				if (trans->act)
					printf("action:\t%s\n", trans->act->id);	
				printf("guard condition:\t");
				PrintCondition(trans->cond);
				printf("\n");
			}
			tlist = tlist->next;
		} while (tlist != translist);
	}
}


/*Prints program graph*/

void PrintPG(const programgraph *pg) {

	const int nolocs = NumberOfElements(pg->loclist);
	const list *llist;
	const location *loc;
	
	if (pg) {
		if (pg->no_vars == 1) 
			printf("\n\n1 variable\n\n");
		else 
			printf("\n\n%i variables\n\n", pg->no_vars);
		PrintVariables(pg->varlist);
		if (nolocs == 1) 
			printf("\n\n1 location\n\n");
		else 
			printf("\n\n%i locations\n\n", nolocs);
		if (pg->loclist) {
			llist = pg->loclist;
			do {
				if ((loc = (location *) llist->data)) {
					printf("%i\t", loc->index);				
					llist = llist->next;
				}
			} while (llist != pg->loclist);
		}
		if (pg->no_acts == 1) 
			printf("\n\n1 action\n\n");
		else 
			printf("\n\n\n%i actions\n\n", pg->no_acts);
		PrintActions(pg->actlist);
		if (pg->no_trans == 1) 
			printf("\n\n1 transition\n\n");
		else 
			printf("\n%i transitions\n\n", pg->no_trans);
		PrintTransitions(pg->translist);
		printf("\ninitial location:");
		if (pg->loc_0)
			printf("\t%i", pg->loc_0->index);
		printf("\n\ninitial condition:\t");
		PrintCondition(pg->cond_0);
		printf("\n\nfinal location:");
		if (pg->loc_1) {
			printf("\t\t%i", pg->loc_1->index);
			printf("\n\nfinal condition:\t");
			PrintCondition(pg->cond_1);
		}
		else
			printf("\t\tnone\n");			
	}
	printf("\n\n\n");	
}
 

/*Prints cubes of degree d*/

static void PrintDegree(const list *cubelist, int d) {

	const list *cubesd; 
	int elcount = 0, j, i;
	cube *pc; 
	cube **edge;
			
	if ((cubesd = cubelist)) {			
		do {
			pc = (cube *) cubesd->data;				
			printf("cube %i.%i: ", d, ++elcount);
			if (d == 0) {
				printf("%s", ((label **) pc->lab->coord)[0]->str);
				if (pc->flags.ini == 1)
					printf("  initial");
				if (pc->flags.fin == 1)
					printf("  final");
				else if (!pc->s[0][0]) 
					printf("  deadlock");								
				printf("\n\n");
			}
			else {
				edge = pc->edges->coord;
				printf("%s  ", ((label **) edge[0]->d[0][0]->lab->coord)[0]->str);
				printf("(");				
				printf("%s", ((label **) edge[0]->lab->coord)[0]->str);
				for (j = 1; j < edge[0]->lab->dim; j++) {
					printf(";");
					printf("%s", ((label **) edge[0]->lab->coord)[j]->str);
				}
				for (i = 1; i < d; i++) {
					printf(",  ");
					printf("%s", ((label **) edge[i]->lab->coord)[0]->str);
					for (j = 1; j < edge[i]->lab->dim; j++) {
						printf(";");
						printf("%s", ((label **) edge[i]->lab->coord)[j]->str);
					}
				}
				printf(")\n\n");
			}									
			cubesd = cubesd->next;
		} while (cubesd != cubelist);				
	}		
}


/*Prints the system and its HDA model*/

void PrintSystemHDA(const vector* pgvec, const list *varlist, list *const cubes[], int dim) {

	const int novars = NumberOfElements(varlist);
	int i, rk[dim + 1], sum = 0, bd = 0, deadlocks = 0, euler = 0;	
	const programgraph *const *const pg = pgvec->coord;
	const list *node;
	const cube *pc;
		
	if (out != OPTION_i) {
		if (pgvec->dim > 1) 
			printf("\nSystem of %i processes\n", pgvec->dim);
		else 
			printf("\nSystem of 1 process\n");
	}		
	if (out != OPTION_i && out != OPTION_s) {
		if (novars == 1)
			printf("\n\n\n1 variable\n\n");
		else 
			printf("\n\n\n%i variables\n\n", NumberOfElements(varlist));
		PrintVariables(varlist);
		printf("\n\n");
	}	
	printf("\n");
	for (i = 0; i < pgvec->dim; i++) {
		if (pgvec->dim > 1) 
			printf("Process %i: %s\n", i, pg[i]->id);
		else
			printf("Process: %s\n", pg[0]->id);
		if (out != OPTION_s)
			PrintPG(pg[i]);
	}		
	if (out != OPTION_i) {		
		for (i = 0; i <= dim; i++) {
			rk[i] = NumberOfElements(cubes[i]);
			sum = sum + rk[i];
			bd = bd + 2 * i * rk[i];  
		}
		printf("\nHDA model of dimension %i with %i elements and %i boundaries\n\n", dim, sum, bd);
		if (out != OPTION_s) 
			printf("\n\n");
		if (rk[0] == 1) 
			printf("Degree 0: 1 element\n");
		else 
			printf("Degree 0: %i elements\n", rk[0]);
		if (out != OPTION_s) {
			printf("\n\n");
			PrintDegree(cubes[0], 0);
			printf("\n");
		}
		for (i = 1; i <= dim; i++) {
			if (rk[i] == 1) 
				printf("Degree %i: 1 element (2 boundaries)\n", i); 
			else 
				printf("Degree %i: %i elements (%i boundaries)\n", i, rk[i], 2 * i * rk[i]); 
			if (out != OPTION_s) {
				printf("\n\n");
				PrintDegree(cubes[i], i);
				printf("\n");
			}
		}							
		if (out != OPTION_s)
			printf("\n\n");
		if ((node = cubes[0])) {
			do {
				pc = (cube *) node->data;
				if (!pc->flags.fin && !pc->s[0][0])
					deadlocks++;
				node = node->next;
			} while (node != cubes[0]);
		}	
		if (deadlocks == 1)
			printf("\n1 deadlock\n\n");
		else 
			printf("\n%i deadlocks\n\n", deadlocks);
		if (out != OPTION_s)
			printf("\n\n");		
		for (i = 0; i <= dim; i++)
			euler = euler + (int) pow(-1, i) * rk[i];	
		printf("Euler characteristic: %i\n\n", euler);		
	}		
}


/*Computes label of pc for use in Sage*/

static void PrintSageLabel(const cube *pc){

	int i, j;
	const cube **edge;

	if (pc->degree == 0) 
		printf("1");		
	else {
		edge = pc->edges->coord;						
		printf("(%s", ((label **) edge[0]->lab->coord)[0]->str);
		for (j = 1; j < edge[0]->lab->dim; j++) {
			printf("+");
			printf("%s", ((label **) edge[0]->lab->coord)[j]->str);
		}
		for (i = 1; i < pc->degree; i++) {
			printf(")");
			printf("*(%s", ((label **) edge[i]->lab->coord)[0]->str);
			for (j = 1; j < edge[i]->lab->dim; j++) {
				printf("+");
				printf("%s", ((label **) edge[i]->lab->coord)[j]->str);
			}
		}
		printf(")");
	}			

}


/*Prints chain complex of HDA in Z_2 chomp format*/

void PrintChainComplex(list *const cubes[], int dim) {

	int d, i, rk[dim + 1];	
	list *cubesd;	
	cube *pc;
	
	for (d = 0; d <= dim; d++) {			
		i = 0;				
		if ((cubesd = cubes[d])) {
			do {
				pc = (cube *) cubesd->data;				
				pc->furtherdata = NewInt(++i);
				cubesd = cubesd->next;
			} while (cubesd != cubes[d]);
		}
		rk[d] = i;		
	}
	printf("chain complex\n\n");
	printf("max dimension = %i\n\n", dim);
	for (d = 0; d <= dim; d++) {
		printf("dimension %i: %i\n\n", d, rk[d]);	
		if ((cubesd = cubes[d])){
			do {
				pc = (cube *) cubesd->data;
				printf("boundary %i.%i:", d, *(int *) pc->furtherdata);	
				PrintSageLabel(pc);
				printf(" = ");
				for (i = 0; i < d; i++) {
					printf("+ %i.%i:", d - 1, *(int *) pc->d[0][i]->furtherdata);
					PrintSageLabel(pc->d[0][i]);
					printf(" ");
				}
				for (i = 0; i < d; i++) {
					printf("+ %i.%i:", d - 1, *(int *) pc->d[1][i]->furtherdata);
					PrintSageLabel(pc->d[1][i]);
					printf(" ");
				}				
				printf("\n");
				cubesd = cubesd->next;
			} while (cubesd != cubes[d]);
		}
		printf("\n");
	}	
	for (d = 0; d <= dim; d++) {						
		if ((cubesd = cubes[d])) {
			do {
				pc = (cube *) cubesd->data;				
				DeleteInt(pc->furtherdata);
				pc->furtherdata = NULL;
				cubesd = cubesd->next;
			} while (cubesd != cubes[d]);
		}		
	}
}


/*Prints HDA in tsv format*/

void PrintHDA(list *const cubes[], int dim) {

	int i, d, j;	
	list *cubesd;	
	cube *pc, **edge;
	
	printf("\"degree\"\t\"id\"");
	for (i = 1; i <= dim; i++) 
		printf("\t\"d^0_%i\"", i);
	for (i = 1; i <= dim; i++) 
		printf("\t\"d^1_%i\"", i);
	printf("\t\"label\"\t\"initial\"\t\"final\"\t\"deadlock\"\t\"origin\"\n");
	for (d = 0; d <= dim; d++) {			
		i = 0;				
		if ((cubesd = cubes[d])) {
			do {
				pc = (cube *) cubesd->data;				
				pc->furtherdata = NewInt(++i);
				cubesd = cubesd->next;
			} while (cubesd != cubes[d]);
		}		
	}	
	for (d = 0; d <= dim; d++) {		
		if ((cubesd = cubes[d])){
			do {
				pc = (cube *) cubesd->data;
				printf("\"%i\"\t\"%i.%i\"", d, d, *(int *) pc->furtherdata);	
				for (i = 0; i < d; i++) 
					printf("\t\"%i.%i\"", d - 1, *(int *) pc->d[0][i]->furtherdata);
				for (i = d; i < dim; i++)
					printf("\t\"\"");
				for (i = 0; i < d; i++) 
					printf("\t\"%i.%i\"", d - 1, *(int *) pc->d[1][i]->furtherdata);
				for (i = d; i < dim; i++)
					printf("\t\"\"");
				if (d == 0)
					printf("\t\"()\"");						
				else {
					edge = pc->edges->coord;						
					printf("\t\"(%s", ((label **) edge[0]->lab->coord)[0]->str);
					for (j = 1; j < edge[0]->lab->dim; j++) {
						printf(";");
						printf("%s", ((label **) edge[0]->lab->coord)[j]->str);
					}
					for (i = 1; i < d; i++) {	
						printf(", %s", ((label **) edge[i]->lab->coord)[0]->str);	
						for (j = 1; j < edge[i]->lab->dim; j++) {
							printf(";");
							printf("%s", ((label **) edge[i]->lab->coord)[j]->str);
						}
					}
					printf(")\"");
				}		
				if (d == 0 && pc->flags.ini == 1) 
					printf("\t\"y\"");	
				else
					printf("\t\"\"");
				if (d == 0 && pc->flags.fin == 1) 
					printf("\t\"y\"");	
				else
					printf("\t\"\"");	
				if (d == 0 && pc->flags.fin != 1 && !pc->s[0][0]) 
					printf("\t\"y\"");	
				else
					printf("\t\"\"");
				if (d == 0)
					printf("\t\"%s\"", ((label **) pc->lab->coord)[0]->str);
				else {
					edge = pc->edges->coord;
					printf("\t\"%s\"", ((label **) edge[0]->d[0][0]->lab->coord)[0]->str);					
				}
				printf("\n");								
				cubesd = cubesd->next;
			} while (cubesd != cubes[d]);
		}		
	}	
	for (d = 0; d <= dim; d++) {					
		if ((cubesd = cubes[d])) {
			do {
				pc = (cube *) cubesd->data;				
				DeleteInt(pc->furtherdata);
				pc->furtherdata = NULL;
				cubesd = cubesd->next;
			} while (cubesd != cubes[d]);
		}		
	}
}
