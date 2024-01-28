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
File pgraph.c

This file implements the functions declared in pgraph.h.
************************************************************************************************/

#include "pgraph.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vector.h"
#include "list.h"	


/*Creates new variable of domainsize d*/

intvar *NewVariable(int d) {

	intvar *var;

	if ((var = malloc(sizeof(intvar))) == NULL) {
		printf("NewVariable: Out of memory!\n");
		exit(EXIT_FAILURE);
	}	
	var->domain = NewVector(d, sizeof(int));
	var->initialval = 0;
	strcpy(var->id, "");
	return var;
} 


/*Deletes variable*/

void DeleteVariable(void *var) {

	intvar *v;	

	if (var) {
		v = (intvar *) var;
		DeleteVector(v->domain);
		free(v);			
	}
}


/*Creates new location*/

location *NewLocation() {

	location *loc;

	if ((loc = malloc(sizeof(location))) == NULL) {
		printf("NewLocation: Out of memory!\n");
		exit(EXIT_FAILURE);
	}
	strcpy(loc->label, "");	
	loc->intranslist = NULL;
	loc->outtranslist = NULL;
	return loc;
}


/*Deletes location*/

void DeleteLocation(void *loc) {

	location *l;

	if (loc) {
		l = (location *) loc;	
		DeleteList(&l->outtranslist, NULL);
		DeleteList(&l->intranslist, NULL);
		free(l);		
	}
}


/*Creates new ast*/

ast *NewAst(int type, int nop, intvar *var, char par, ast *l, ast *r) {

	ast *exp;
	
	if ((exp = malloc(sizeof(ast))) == NULL) {
		printf("NewAst: Out of memory!\n");
		exit(EXIT_FAILURE);
	}
	exp->type = type;
	switch (type) {
		case VAR:
			exp->node.var = var;
			break;
		case NUM:
			exp->node.num = nop;
			break;
		default:
			exp->node.op = nop;
			break;		
	}
	exp->par = par;	
	exp->l = l;
	exp->r = r; 
	return exp;
}


/*Deletes ast*/

void DeleteAst(void *exp) {

	ast *e;	

	if (exp) {
		e = (ast *) exp;		
		DeleteAst(e->l);
		DeleteAst(e->r);		
		free(e);			
	}
}


/*Creates new condition*/

condition *NewCondition() {
	
	condition *cond;
	ast *null0 = NewAst(NUM, 0, NULL, 0, NULL, NULL), *null1 = NewAst(NUM, 0, NULL, 0, NULL, NULL); 			

	if ((cond = malloc(sizeof(condition))) == NULL) {
		printf("NewCondition: Out of memory!\n");
		exit(EXIT_FAILURE);
	}
	cond->exp = NewAst(EXP, EQ, NULL, 0, null0, null1);	/*0 == 0, true*/
	strcpy(cond->id, "");
	cond->varlist = NULL;
	cond->evallist = NULL;
	cond->no_vars = 0;
	return cond;
} 


/*Deletes given condition*/

void DeleteCondition(void *cond) {

	condition *c;

	if (cond) {
		c = (condition *) cond;
		DeleteAst(c->exp);
		DeleteList(&c->varlist, NULL);
		DeleteList(&c->evallist, DeleteVector);	
		free(c);		
	}
}


/*Creates new assignment*/

assignment *NewAssignment() {

	assignment *ass;
	
	if ((ass = malloc(sizeof(assignment))) == NULL) {
		printf("NewAssignment: Out of memory!\n");
		exit(EXIT_FAILURE);
	}
	ass->var = NULL;
	ass->exp = NULL;
	return ass;
}


/*Deletes assignment*/

void DeleteAssignment(void *ass) {

	assignment *a;
	
	if (ass) {
		a = (assignment *) ass;	
		DeleteAst(a->exp);	
		free(a);
	}
}


/*Creates new action for v variables and e evaluations*/

action *NewAction(int v, int e) {
	
	action *act;
	int i, j;

	if ((act = malloc(sizeof(action))) == NULL) {
		printf("NewAction: Out of memory!\n");
		exit(EXIT_FAILURE);
	}	
	for (i = 0; i < 2; i++) {
		if ((act->map[i] = malloc(sizeof(int *) * e)) == NULL) {
			printf("NewAction: Out of memory!\n");
			exit(EXIT_FAILURE);
		}		
		for (j = 0; j < e; j++) {
			if ((act->map[i][j] = calloc(v, sizeof(int))) == NULL) {
				printf("NewAction: Out of memory!\n");
				exit(EXIT_FAILURE);
			}			
		}
	}
	strcpy(act->id, "");
	act->varlist = NULL;
	act->assignments = NULL;
	act->no_vars = v;
	act->no_evals = e;
	return act;
} 


/*Deletes given action*/

void DeleteAction(void *act) {

	action *a;
	int j, i;

	if (act) {
		a = (action *) act;
		DeleteList(&a->varlist, NULL);
		DeleteList(&a->assignments, DeleteAssignment);
		for(j = 0; j < 2; j++) {
			for(i = 0; i < a->no_evals; i++) {
				free(a->map[j][i]);				
			}
		}
		for(j = 0; j < 2; j++) {
			free(a->map[j]);			
		}
		free(a);		
	}
}


/*Creates new transition*/

transition *NewTransition() {
	
	transition *trans;

	if ((trans = malloc(sizeof(transition))) == NULL) {
		printf("NewTransition: Out of memory!\n");
		exit(EXIT_FAILURE);
	}	
	trans->cond = NewCondition();
	trans->loc[0] = NULL;
	trans->loc[1] = NULL;			
	trans->act = NULL;
	return trans;
} 


/*Deletes given transition*/

void DeleteTransition(void *trans) {

	transition *t;

	if (trans) {
		t = (transition *) trans;
		DeleteCondition(t->cond);
		free(t);				
	}
}


/*Creates new program graph*/

programgraph *NewPG() {

	programgraph *pg;

	if ((pg = malloc(sizeof(programgraph))) == NULL) {
		printf("NewPG: Out of memory!\n");
		exit(EXIT_FAILURE);
	}	
	pg->varlist = NULL;
	pg->loclist = NULL;
	pg->loc_0 = NULL;
	pg->loc_1 = NULL;
	pg->actlist = NULL;
	pg->translist = NULL;
	pg->cond_0 = NewCondition();
	pg->cond_1 = NewCondition();
	strcpy(pg->id, "");
	pg->no_vars = 0;
	pg->no_acts = 0;
	pg->no_trans = 0;	
	return pg;
} 


/*Deletes given program graph */

void DeletePG(void *pg) {

	programgraph *p;
		
	if (pg) {
		p = (programgraph *) pg;
		DeleteCondition(p->cond_1);
		DeleteCondition(p->cond_0);
		DeleteList(&p->translist, DeleteTransition);
		DeleteList(&p->actlist, DeleteAction);
		DeleteList(&p->loclist, DeleteLocation);		
		DeleteList(&p->varlist, NULL);		
		free(p);		
	}
}


/*Copies ast*/

ast *CopyAst(const ast *exp) {

	ast *copy = NULL;
	
	if (exp) {
		switch (exp->type) {
			case VAR:
				copy = NewAst(exp->type, 0, exp->node.var, exp->par, NULL, NULL);
				break;
			case NUM: 
				copy = NewAst(exp->type, exp->node.num, NULL, exp->par, NULL, NULL);
				break;			
			default:
				copy = NewAst(exp->type, exp->node.op, NULL, exp->par, CopyAst(exp->l), CopyAst(exp->r));
				break;	 	
		}
	}			
	return copy;
}


/*Evaluates expression*/

int Evaluate(const ast *exp, const list *varlist, const vector *vals) {

	const list *node;
	intvar *var;
	int i = 0;
  
  	if (exp) {
	 	switch (exp->type) {
	 		case VAR:
	 			if (varlist) {
					node = varlist;
					do {
						var = (intvar *) node->data;
						if (var == exp->node.var) 							
							return ((int *) vals->coord)[i];										
						i++;
						node = node->next;
					} while (node != varlist);	
				}	
	 			break;
			case NUM:
				return exp->node.num;
				break;
			default:
				if (exp->node.op == PLUS) 
					return Evaluate(exp->l, varlist, vals) + Evaluate(exp->r, varlist, vals);
				else if (exp->node.op == MINUS) {
					if (exp->l)
						return Evaluate(exp->l, varlist, vals) - Evaluate(exp->r, varlist, vals);
					else 
						return - Evaluate(exp->r, varlist, vals);	
				}		
				else if (exp->node.op == TIMES) 
					return Evaluate(exp->l, varlist, vals) * Evaluate(exp->r, varlist, vals);	
				else if (exp->node.op == DIV) 
					return Evaluate(exp->l, varlist, vals) / Evaluate(exp->r, varlist, vals);	
				else if (exp->node.op == MOD) 
					return Evaluate(exp->l, varlist, vals) % Evaluate(exp->r, varlist, vals);	
				else if (exp->node.op == EQ) 
					return Evaluate(exp->l, varlist, vals) == Evaluate(exp->r, varlist, vals);
				else if (exp->node.op == NEQ) 
					return Evaluate(exp->l, varlist, vals) != Evaluate(exp->r, varlist, vals);	
				else if (exp->node.op == L) 
					return Evaluate(exp->l, varlist, vals) < Evaluate(exp->r, varlist, vals);				
				else if (exp->node.op == LEQ) 
					return Evaluate(exp->l, varlist, vals) <= Evaluate(exp->r, varlist, vals);	
				else if (exp->node.op == G) 
					return Evaluate(exp->l, varlist, vals) > Evaluate(exp->r, varlist, vals);	
				else if (exp->node.op == GEQ) 
					return Evaluate(exp->l, varlist, vals) >= Evaluate(exp->r, varlist, vals);									
				else if (exp->node.op == OR) 
					return Evaluate(exp->l, varlist, vals) || Evaluate(exp->r, varlist, vals);
				else if (exp->node.op == AND) 
					return Evaluate(exp->l, varlist, vals) && Evaluate(exp->r, varlist, vals);		
				else 
					return !Evaluate(exp->r, varlist, vals);	
				break;			 			
	 	}
	} 		 	
 	printf("Evaluate: Error!\n");
	exit(EXIT_FAILURE); 
}


/*Computes a string representation of an expression*/

void AstName(char *name, const ast *exp) {
	
	char nm[STRL], *ops[15] = { "", "+", "-", "*", "/", "%", "==", "!=", "<", "<=", ">", ">=", "||", "&&", "!" };

	strcpy(name, ""); 
	if (exp) {
		if (exp->par) {		
			switch (exp->type) {
				case VAR:
					sprintf(name, "(%s)", exp->node.var->id);
					break;
				case NUM:
					sprintf(name, "(%d)", exp->node.num);
					break;
				case NUMEXP:					
					sprintf(name, "%d", Evaluate(exp, NULL, NULL));						
					break;	
				default:
					strcpy(name, "(");
					strcpy(nm, "");	/*redundant*/														
					AstName(nm, exp->l);
					strcat(name, nm);
					sprintf(nm, "%s", ops[exp->node.op]);
					strcat(name, nm);
					strcpy(nm, "");
					AstName(nm, exp->r);
					strcat(name, nm);
					strcat(name, ")");							
					break;						
			}		
		}
		else {
			switch (exp->type) {
				case VAR:
					sprintf(name, "%s", exp->node.var->id);
					break;
				case NUM:
					sprintf(name, "%d", exp->node.num);
					break;
				case NUMEXP:
					sprintf(name, "%d", Evaluate(exp, NULL, NULL));				
					break;	
				default:
					strcpy(nm, "");																								
					AstName(nm, exp->l);
					strcpy(name, nm);					
					sprintf(nm, "%s", ops[exp->node.op]);
					strcat(name, nm);
					strcpy(nm, "");
					AstName(nm, exp->r);
					strcat(name, nm);												
					break;						
			}
		}
	}														
}


/*Computes the name of an action*/

void ActionName(char *name, const action *act) {
	
	assignment *ass;
	char nm[STRL];
	list *node;

	strcpy(name, "");
	if (act && act->assignments) {
		ass = (assignment *) act->assignments->data;
		strcpy(name, ass->var->id);
		strcat(name, "=");
		strcpy(nm, "");	/*redundant*/	
		AstName(nm, ass->exp);	
		strcat(name, nm);
		if (NumberOfElements(act->assignments) > 1) {		
			node = act->assignments->next;
			do {
				ass = (assignment *) node->data;
				strcat(name, ";");
				strcat(name, ass->var->id);
				strcat(name, "=");
				strcpy(nm, "");			
				AstName(nm, ass->exp);
				strcat(name, nm);
				node = node->next;
			} while (node != act->assignments);
		}
	}	
}


/*Returns 1 if vector satisfies condition and 0 otherwise*/

int CheckCondition(const vector *vec, const condition *cond, const list *varlist) {

	int i, j, result = 0;
	const list *clist, *vlist;
	const intvar *var, *va;
	const int vars = NumberOfElements(varlist);
	vector *v;
	const vector *ev;

	if (inp == OPTION_old) {
		v = NewVector(cond->no_vars, sizeof(int));
		if (cond->varlist) {
			clist = cond->varlist;
			for (i = 0; i < cond->no_vars; i++) {
				var = (intvar *) clist->data;
				if (varlist) {
					vlist = varlist;
					for (j = 0; j < vars; j++) {
						va = (intvar *) vlist->data;
						if (strcmp(var->id, va->id) == 0) {
							((int *) v->coord)[i] = ((int *) vec->coord)[j];
							break;		
						}	
						vlist = vlist->next;
					}
				}
				clist = clist->next;
			}
		}
		if (cond->evallist) {
			clist = cond->evallist;
			do {
				ev = (vector*) clist->data;	
				if (Veccmp(ev, v) == 0) {
					result = 1;
					break;	
				}
				clist = clist-> next;
			} while (clist != cond->evallist);
		}
		DeleteVector(v);
		return result;
	}
	else 		
		return Evaluate(cond->exp, varlist, vec);		
}


/*Merges conditions*/

condition *MergeConditions(const condition *cond1, const condition *cond2) { /*only needed when inp == OPTION_old*/

	condition *cond = NewCondition();
	int i, j, index[cond2->no_vars], *evc, *ev2c;
	const list *vlist, *wlist;
	const intvar *var, *va;
	const vector *ev1, *ev2;
	vector *ev;
	
	ConcatLists(&cond->varlist, cond1->varlist);
	MergeLists(&cond->varlist, cond2->varlist, NULL);
	cond->no_vars = NumberOfElements(cond->varlist);
	strcpy(cond->id, "merged");
	if (cond2->varlist) {
		vlist = cond2->varlist;
		for (i = 0; i < cond2->no_vars; i++) {				
			var = (intvar *) vlist->data;
			wlist = cond->varlist;
			for (j = 0; j < cond->no_vars; j++) {							
				va = (intvar *) wlist->data;
				if (strcmp(var->id, va->id) == 0) {
					index[i] = j;
					break;
				}
				wlist = wlist->next;				
			}
			vlist = vlist->next;			
		}
	}
	if (cond1->evallist) {
		vlist = cond1->evallist;
		do {
			ev1 = (vector *) vlist->data;				
			if (cond2->evallist) {
				wlist = cond2->evallist;
				do {
					ev2 = (vector *) wlist->data;
					ev2c = ev2->coord;
					ev = NewVector(cond->no_vars, sizeof(int));
					evc = ev->coord;
					for (i = 0; i < cond1->no_vars; i++) 	
						evc[i] = ((int *) ev1->coord)[i];
					for (i = 0; i < cond2->no_vars; i++) {
						if (index[i] < cond1->no_vars && evc[index[i]] != ev2c[i]) {
							DeleteVector(ev);
							ev = NULL;
							break;
						}
						evc[index[i]] = ev2c[i];
					}
					if (ev)
						cond->evallist = InsertElement(ev, cond->evallist);
					wlist = wlist->next;
				} while (wlist != cond2->evallist);
			}
			vlist = vlist->next;
		} while (vlist != cond1->evallist);
	}
	return cond;
}


/*Extends condition cond to larger list of variables varlist, result is in newcond*/

void ExtendCondition(const condition *cond, condition *newcond, const list *varlist) { /*only needed when inp == OPTION_old*/

	int i, deletedata = 0;	
	const intvar *var, *var2;
	const list *vlist, *vlist2;
	list *helpvarlist = NULL, *helpevallist = NULL, *veclist, *product, *transpose;
	vector *vec;
			
	strcpy(newcond->id, cond->id);
	newcond->no_vars = NumberOfElements(varlist);
	DeleteList(&newcond->varlist, NULL);
	ConcatLists(&newcond->varlist, varlist);
	DeleteList(&newcond->evallist, NULL);
	if (cond->evallist) { 
		if (!varlist) /*in this case, cond->varlist should be NULL, too*/
			ConcatLists(&newcond->evallist, cond->evallist);
		else { 
			ConcatLists(&helpvarlist, cond->varlist);
			MergeLists(&helpvarlist, varlist, NULL);
			ConcatLists(&helpevallist, cond->evallist);			
			if (newcond->no_vars != cond->no_vars) {
				vlist = helpvarlist;
				for (i = 0; i < cond->no_vars; i++)
					vlist = vlist->next;				
				do {
					var = (intvar *) vlist->data;
					veclist = NULL;
					for (i = 0; i < var->domain->dim; i++) {
						vec = NewVector(1, sizeof(int));
						((int *) vec->coord)[0] = ((int *) var->domain->coord)[i];
						veclist = InsertElement(vec, veclist);	
					}
					product = Product(helpevallist, veclist, INT);
					if (deletedata)
						DeleteList(&helpevallist, DeleteVector); 						
					else {
						DeleteList(&helpevallist, NULL);
						deletedata = 1;
					}
					DeleteList(&veclist, DeleteVector);
					helpevallist = product;
					vlist = vlist->next;			
				} while (vlist != helpvarlist);
			}								
			transpose = Transpose(helpevallist);
			if (deletedata) 
				DeleteList(&helpevallist, DeleteVector);
			else 
				DeleteList(&helpevallist, NULL);
			helpevallist = transpose;	
			vlist = varlist;
			do {
				var = (intvar *) vlist->data;
				vlist2 = helpvarlist;
				veclist = helpevallist;
				do {
					var2 = (intvar *) vlist2->data;
					vec = (vector *) veclist->data;
					if (var == var2) {
						newcond->evallist = InsertElement(vec, newcond->evallist);
						break;
					}
					veclist = veclist->next;
					vlist2 = vlist2->next;
				} while (vlist2 != helpvarlist);
				vlist = vlist->next;
			} while (vlist != varlist);
			transpose = Transpose(newcond->evallist);
			DeleteList(&newcond->evallist, NULL);
			newcond->evallist = transpose;
			DeleteList(&helpevallist, DeleteVector);
			DeleteList(&helpvarlist, NULL);
		}
	}
} 


/*Computes the effect of the action on invec, result is in outvec*/ 

void Effect(const action *act, const vector *invec, vector *outvec, const list *varlist) {
	
	const int novars = NumberOfElements(varlist);
	int i, j, k, ind[novars], arg[act->no_vars]; 
	const list *vlist1, *vlist2, *node;
	const intvar *var1, *var2;
	assignment *ass;
	
	if (inp == OPTION_old) {
		if (varlist && act->varlist) {
			i = 0;
			vlist1 = varlist;
			do {
				var1 = (intvar *) vlist1->data;
				ind[i] = -1;
				j = 0;	
				vlist2 = act->varlist;
				do {
					var2 = (intvar *) vlist2->data;
					if (var1 == var2) {
						ind[i] = j;
						break;
					}
					j = j + 1;
					vlist2 = vlist2->next;
				} while (vlist2 != act->varlist);
				i = i + 1;
				vlist1 = vlist1->next;
			} while (vlist1 != varlist);	
		}
		for (i = 0; i < novars; i++) { 
			if (ind[i] >= 0)
				arg[ind[i]] = ((int *) invec->coord)[i];
		}
		for (i = 0; i < act->no_evals; i++) {
			k = i;
			for (j = 0; j < act->no_vars && k >= 0; j++) {
				if (act->map[0][i][j] != arg[j]) 
					k = -1;							
			}
			if (k == i)
				break;
		}
		for (i = 0; i < novars; i++) {
			if (ind[i] >= 0)
				((int *) outvec->coord)[i] = act->map[1][k][ind[i]];
			else 
				((int *) outvec->coord)[i] = ((int *) invec->coord)[i];
		}
	}
	else {
		for (i = 0; i < outvec->dim; i++)
			((int *) outvec->coord)[i] = ((int *) invec->coord)[i];			
		if (act->assignments) {
			node = act->assignments;
			do {
				ass = (assignment *) node->data;				
				if (varlist) {
					i = 0;
					vlist1 = varlist;
					do {
						var1 = (intvar *) vlist1->data;
						if (var1 == ass->var)
							break;
						i++;	
						vlist1 = vlist1->next;
					} while (vlist1 != varlist);
				}										
				((int *) outvec->coord)[i] = Evaluate(ass->exp, varlist, outvec);				
				node = node->next;
			} while (node != act->assignments);
		}
	}	
} 	
