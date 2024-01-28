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
File pgraph.h

This file declares program graphs, their components, and functions for these structures, in 
particular, functions to allocate and free memory. 
************************************************************************************************/

#ifndef PGRAPH_H
#define PGRAPH_H

#include "def.h"	/*for STRL*/

#define VAR 1	/*ast node types*/
#define NUM 2
#define EXP 3
#define NUMEXP 4

#define PLUS 1	/*ast operators*/
#define MINUS 2
#define TIMES 3
#define DIV 4 
#define MOD 5
#define EQ 6
#define NEQ 7
#define L 8
#define LEQ 9
#define G 10
#define GEQ 11
#define OR 12
#define AND 13
#define NOT 14

struct vector;
struct list;

typedef struct intvar intvar;	/*integer variable*/
typedef struct location location;
typedef struct ast ast;		/*abstract syntax tree*/
typedef struct condition condition;	
typedef struct assignment assignment;
typedef struct action action;	
typedef struct transition transition;	
typedef struct programgraph programgraph;

union astnode {intvar *var; int num, op;};

struct intvar {   	
	struct vector *domain;	/*values of the variable, must be integers*/
	int initialval;	
	char id[STRL];	/*name*/	
};
struct location {
	int index;
	char label[STRL];		
	struct list *intranslist, *outtranslist;	/*incoming and outgoing transitions*/	
};
struct ast {
	int type;
	union astnode node;
	char par;	/*parentheses*/
	ast *l, *r;
};
struct condition {
	ast *exp;	/*boolean expression*/
	struct list *varlist;	/*variables*/
	struct list *evallist;	/*list of those evaluations of the variables under which the condition is true*/		
	int no_vars;	/*number of variables*/ 
	char id[STRL];	/*name*/
};
struct assignment {
	intvar *var;
	ast *exp;
};
struct action {		
	struct list *varlist;	/*variables*/
	struct list *assignments;	/*assignments that compose the action*/
	int no_vars, no_evals, **map[2]; 	/*number of variables, number of evaluations, map: effect function for evaluations*/	
	char id[STRL];	/*name*/			
};
struct transition {
   	location *loc[2];	/*start and end location*/
	condition *cond;	/*guard condition*/
	action *act;		
};
struct programgraph {	
	struct list *varlist;	/*variables*/
	struct list *loclist;	/*locations*/
	location *loc_0, *loc_1;	/*initial and final location*/	
	struct list *actlist;	/*actions*/
	struct list *translist;	/*transitions*/
	condition *cond_0, *cond_1;	/*initial and final condition*/
	char id[STRL];	/*name*/	
	int no_vars, no_acts, no_trans; /*numbers of variables, actions, transitions*/
};

intvar *NewVariable(int d);	/*Creates new variable of domain size d*/
void DeleteVariable(void *var);	/*Deletes given variable*/
location *NewLocation(); /*Creates new location*/
void DeleteLocation(void *loc);	/*Deletes location*/
ast *NewAst(int type, int nop, intvar *var, char par, ast *l, ast *r); /*Creates new ast*/
void DeleteAst(void *exp); /*Deletes ast*/
condition *NewCondition(); /*Creates new condition*/
void DeleteCondition(void *cond); /*Deletes given condition*/
assignment *NewAssignment(); /*Creates new assignment*/
void DeleteAssignment(void *ass); /*Deletes assignment*/
action *NewAction(int v, int e); /*Creates new action for v variables and e evaluations*/
void DeleteAction(void *act); /*Deletes given action*/
transition *NewTransition(); /*Creates new transition*/
void DeleteTransition(void *trans);	/*Deletes given transition*/
programgraph *NewPG(); /*Creates new program graph*/
void DeletePG(void *pg); /*Deletes given program graph*/
ast *CopyAst(const ast *exp); /*Copies ast*/
int Evaluate(const ast *exp, const struct list *varlist, const struct vector *vals); /*Evaluates expression*/
void AstName(char *name, const ast *exp);	/*Computes a string representation of an expression*/
void ActionName(char *name, const action *act);	/*Computes the name of an action*/
int CheckCondition(const struct vector *vec, const condition *cond, const struct list *varlist); /*Returns 1 if vector satisfies condition and 0 otherwise*/
condition *MergeConditions(const condition *cond1, const condition *cond2); /*Merges conditions*/ 
void ExtendCondition(const condition *cond, condition *newcond, const struct list *varlist); /*Extends condition cond to larger list of variables varlist, result is in newcond*/
void Effect(const action *act, const struct vector *invec, struct vector *outvec, const struct list *varlist); /*Computes the effect of the action on invec, result is in outvec*/
#endif	
