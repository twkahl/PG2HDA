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
File pml2pg.y

This file implements a Bison parser to read Promela files.
************************************************************************************************/

%{
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vector.h"
#include "list.h"
#include "pgraph.h"
#include "cube.h"


#define OPT 0	/*option*/
#define DOOD 1	/*repetition structure*/
#define IFFI 2	/*selection structure*/


/*Global variables*/

static int phase, startline, endpreamble, rep, pid, endline, wait;
static programgraph *pg;
extern int yylineno;
static list *variables, *defacts, *danglinglocs, *blks;
static vector *startlines, *times, *endlines;
static location *currentloc, *previousloc;
static ast *initialexp;


/*Function prototypes*/

static void MakeVariables(const char *name, const vector *vals); /*Constructs variables and inserts them in the list of variables. The number of variables to be constructed is given by the dimension of the vector vals, which contains the initial values. The variables have the given name, with index in the case of more than one variable. The initial values are used to extend the initialexp.*/ 
static void DefTransition(condition *guard, action *act); /*Creates a new transition with the given guard condition and the given action and inserts it in the defines list*/
static action *MakeAssignAction(intvar *var, ast *exp); /*Creates an action that assigns the given expression to the given variable*/
static action *ComposeActions(const action *seq, const action *act); /*Composes the given actions*/
static condition *MakeCondition(ast *exp); /*Creates a condition representing the given expression*/
static void MergeCurrentLoc(location *mergeloc); /*Merges the current location into mergeloc and deletes the current location (if both locations exist and are different)*/
static action *GetAction(action *act); /*Returns the registered version of the given action*/
static void CreateTransition(const condition *guard, action *act); /*Sets the previous and the current location and creates a new transition between them with the given guard condition and action*/
static void Open(int type); /*Opens block of the indicated type*/
static void Close(int type); /*Closes block or option according to the indicated type*/
void ParsePML(FILE *fp, list **varlist, programgraph *pgraph, list **sections, const int create); /*Parses the given file in two modes: if create == 0, the sections containing process definitions are located; if create == 1, the programgraph described in the section at the head of the section list is created and this section is then removed from the list; the variables are only constructed when the very first programgraph is created*/
void yyrestart(FILE *fp); /*Called by ParsePML*/
int yylex(); /*For yyparse*/
void yylex_destroy(); /*Called by ParsePML*/
void yyerror(char const *s); /*Reports errors in the input file*/
%}


%define parse.lac full
%define parse.error verbose

%union {char *str; long long ll; vector *vec; action *act; struct ast *exp; condition *cond;} 

%token INTEGER 
%token INCREMENT DECREMENT SKIP
%token DEF 
%token ATOMIC
%token ARROW
%token IS_EQUAL NOT_EQUAL IS_G IS_GEQ IS_L IS_LEQ  
%token ACTIVE PROCTYPE
%token DO OD IF FI GOTO 
%token DOUBLECOLON
%token PID
%token ASSERT

%token <str> STRING /*string starting with a letter and consisting otherwise of letters, digits, and underscores*/
%token <ll> NAT_NUMBER /*natural number in [0, INT_MAX + 1]*/

%left LOGICAL_OR
%left LOGICAL_AND
%left '+' '-'
%left '*' '/' '%'
%precedence LOGICAL_NEG UMINUS

%type <str> varid loclabel
%type <vec> values
%type <act> assignment sequence 
%type <exp> ariexp boolexp
%type <cond> condition
 

%%
pml			: 	vardecls optionaldefs procs {;}
			;
        
vardecls    :	vardecl {;}
            |   vardecls vardecl {;}
            ;

vardecl		: 	INTEGER vardefs ';' {;}
			;	

vardefs		: 	vardef {;}
			| 	vardefs ',' vardef {;}
			;
 
vardef      :  	STRING '=' NAT_NUMBER {
            		vector *vals;
            		
					if (phase == 1 && startline == endpreamble + 1 && rep == 0) { /*variables are only constructed when the very first programgraph is created*/
						if ($3 > (long long) INT_MAX) 
							yyerror("value out of range"); 
						vals = NewVector(1, sizeof(int));
						((int *) vals->coord)[0] = (int) $3; 												
		        		MakeVariables($1, vals);
		        		DeleteVector(vals);
		        	}	                                                                                                	
                    free($1);                                                                                                                                     
                }
            |	STRING '=' '-' NAT_NUMBER {
            		vector *vals;
					
					if (phase == 1 && startline == endpreamble + 1 && rep == 0) {
						if (- $4 < (long long) INT_MIN) 
							yyerror("value out of range");																
						vals = NewVector(1, sizeof(int));
						((int *) vals->coord)[0] = (int) - $4; 											
		        		MakeVariables($1, vals);
		        		DeleteVector(vals);
		        	}	                                                                                                	
                    free($1);                                                                                                                                     
                }        
            |   STRING '[' NAT_NUMBER ']' '=' NAT_NUMBER { 
            		vector *vals;
            		unsigned int dim, i;

					if (phase == 1 && startline == endpreamble + 1 && rep == 0) {
						if ($6 > (long long) INT_MAX) 
							yyerror("value out of range");						
						dim = (unsigned int) $3;											
						if (dim == 0) 
							yyerror("array not well defined");	
						if (dim > 100) 
							yyerror("array dimension > 100");	
						vals = NewVector(dim, sizeof(int));						
						for (i = 0; i < dim; i++)
							((int *) vals->coord)[i] = (int) $6;																							 
		        		MakeVariables($1, vals);
		        		DeleteVector(vals);
		        	}	
        			free($1);                                                                                                                                     
                }
            |   STRING '[' NAT_NUMBER ']' '=' '-' NAT_NUMBER { 
            		vector *vals;
            		unsigned int dim, i;

					if (phase == 1 && startline == endpreamble + 1 && rep == 0) {
						if (- $7 < (long long) INT_MIN) 
							yyerror("value out of range");
						dim = (unsigned int) $3;	
						if (dim == 0) 
							yyerror("array not well defined");
						if (dim > 100) 
							yyerror("array dimension > 100");								
						vals = NewVector(dim, sizeof(int));
						for (i = 0; i < dim; i++) 
							((int *) vals->coord)[i] = (int) - $7;  
		        		MakeVariables($1, vals);
		        		DeleteVector(vals);
		        	}	
        			free($1);                                                                                                                                     
                }    
            |   STRING '[' NAT_NUMBER ']' '=' '{' values '}' { 
            		unsigned int dim;
                       					
					if (phase == 1 && startline == endpreamble + 1 && rep == 0) {						
						dim = (unsigned int) $3;
						if (dim > 100) 
							yyerror("array dimension > 100");		
						if ((dim == 0) || (dim != $7->dim)) 
							yyerror("array not well defined");
		        		MakeVariables($1, $7);
		        		DeleteVector($7);
		        	}	
            		free($1);                                                                                                                                     
                }                                                
            ;                                            
            
values		: 	NAT_NUMBER {
					if (phase == 1 && startline == endpreamble + 1 && rep == 0) {						
						if ($1 > (long long) INT_MAX) 
							yyerror("value out of range");	
						$$ = NewVector(1, sizeof(int));
						((int *) $$->coord)[0] = (int) $1;
					}						
				}
			| 	'-' NAT_NUMBER {
					if (phase == 1 && startline == endpreamble + 1 && rep == 0) {
						if (- $2 < (long long) INT_MIN) 
							yyerror("value out of range");					
						$$ = NewVector(1, sizeof(int));
						((int *) $$->coord)[0] = (int) - $2;
					}						
				}				
			|	values ',' NAT_NUMBER {
					int i; 
					
					if (phase == 1 && startline == endpreamble + 1 && rep == 0) {
						if ($3 > (long long) INT_MAX) 
							yyerror("value out of range");	
						$$ = NewVector($1->dim + 1, sizeof(int));
						for (i = 0; i < $1->dim; i++)
							((int *) $$->coord)[i] = ((int *) $1->coord)[i];
						((int *) $$->coord)[$1->dim] = (int) $3;
						DeleteVector($1);
					}		
				}
			|	values ',' '-' NAT_NUMBER {
					int i; 
					
					if (phase == 1 && startline == endpreamble + 1 && rep == 0) {
						if (- $4 < (long long) INT_MIN) 
							yyerror("value out of range");						
						$$ = NewVector($1->dim + 1, sizeof(int));
						for (i = 0; i < $1->dim; i++)
							((int *) $$->coord)[i] = ((int *) $1->coord)[i];
						((int *) $$->coord)[$1->dim] = (int) - $4;
						DeleteVector($1);
					}		
				}				
			;                                                                                                      

optionaldefs:   %empty {;}
            |   defs {;}
            ;

defs	    :	def {;}
            |   defs def {;}
            ;

def     	:  	DEF STRING assignment {
					char str[STRL];

					if (phase == 1) { /*creation of some programgraph*/
						strcpy($3->id, $2);						
		                sprintf(str, "__%d", pid);
		                strcat($3->id, str);		                                         
		                DefTransition(NULL, $3);                    
		                pg->actlist = InsertElement($3, pg->actlist);
		            }    
                    free($2);                    
                }				               			               
            |   DEF STRING ATOMIC '{' sequence '}' {
            		char str[STRL];
            		
            		if (phase == 1) {           
		        		strcpy($5->id, $2);					
		                sprintf(str, "__%d", pid);
		                strcat($5->id, str);		                                       
		                DefTransition(NULL, $5);
		                pg->actlist = InsertElement($5, pg->actlist); 
		            }    
                    free($2);                           
                }
            |   DEF STRING ATOMIC '{' condition ARROW sequence '}' {
            		char str[STRL];
                    
                    if (phase == 1) {     
		        		strcpy($7->id, $2);						
		                sprintf(str, "__%d", pid);
		                strcat($7->id, str);		                                                
		                DefTransition($5, $7);
		                pg->actlist = InsertElement($7, pg->actlist);
		            }    
                    free($2);                                                  
                }             
            |	DEF STRING boolexp { /*ignored*/
            		if (phase == 1)
            			DeleteAst($3);
            		free($2);	
            	}	
            ;             

assignment  : 	varid '=' ariexp {
					list *node;
            		intvar *var = NULL;
            		int found = 0;            		          		
            		char str[STRL];
					
					if (phase == 1 && (yylineno <= endpreamble || (yylineno >= startline && yylineno <= endline))) { /*defs section or current programgraph*/
						if (variables) {		
							node = variables;
							do {
								var = (intvar *) node->data;
								if (strcmp(var->id, $1) == 0) {
									found = 1;
									break;
								}	
								node = node->next;
							} while (node != variables);							
						}
						if (!found)
							yyerror("undeclared variable");																			 
						$$ = MakeAssignAction(var, $3);						
		                sprintf(str, "__%d", pid);
		                strcat($$->id, str);		                		               
		            }    		
					free($1);
			  	}						  		
            |   varid INCREMENT {            		
            		list *node;
            		intvar *var = NULL;
            		int found = 0;
            		ast *l, *r, *exp;
            		char str[STRL];
            		
            		if (phase == 1 && (yylineno <= endpreamble || (yylineno >= startline && yylineno <= endline))) {          		
		        		if (variables) {		
							node = variables;
							do {
								var = (intvar *) node->data;
								if (strcmp(var->id, $1) == 0) {
									found = 1;
									break;
								}	
								node = node->next;
							} while (node != variables);							
						}
						if (!found)
							yyerror("undeclared variable"); 		           		
		        		l = NewAst(VAR, 0, var, 0, NULL, NULL);
		        		r = NewAst(NUM, 1, NULL, 0, NULL, NULL);
		        		exp = NewAst(EXP, PLUS, NULL, 0, l, r);
		        		$$ = MakeAssignAction(var, exp);
		        		sprintf($$->id, "%s++", $1);		        		
		                sprintf(str, "__%d", pid);
		                strcat($$->id, str);		                		                
		            }                     
            		free($1); 
            	} 
            |   varid DECREMENT {            		
            		list *node;
            		intvar *var = NULL;
            		int found = 0;
            		ast *l, *r, *exp;
            		char str[STRL];
            		
            		if (phase == 1 && (yylineno <= endpreamble || (yylineno >= startline && yylineno <= endline))) {		        		 
		        		if (variables) {		
							node = variables;
							do {
								var = (intvar *) node->data;
								if (strcmp(var->id, $1) == 0) {
									found = 1;
									break;
								}	
								node = node->next;
							} while (node != variables);							
						}
						if (!found)
							yyerror("undeclared variable");           		
		        		l = NewAst(VAR, 0, var, 0, NULL, NULL);
		        		r = NewAst(NUM, 1, NULL, 0, NULL, NULL);
		        		exp = NewAst(EXP, MINUS, NULL, 0, l, r);            		
		        		$$ = MakeAssignAction(var, exp);
		        		sprintf($$->id, "%s--", $1);		        		
		                sprintf(str, "__%d", pid);
		                strcat($$->id, str);		                		                
		            }                      
            		free($1);
            	}             	              
            |	SKIP {
            		intvar *var;
            		ast *exp;
            		char str[STRL];
            		
            		if (phase == 1 && (yylineno <= endpreamble || (yylineno >= startline && yylineno <= endline))) {
		        		var = (intvar *) variables->data;
		        		exp = NewAst(VAR, 0, var, 0, NULL, NULL);
		        		$$ = MakeAssignAction(var, exp);
		        		sprintf($$->id, "skip"); 		                
		                sprintf(str, "__%d", pid);
		                strcat($$->id, str);                    	
                    }	       
            	}                              
            ; 
            
varid		:	STRING {;}						
			|	STRING '[' ariexp ']' {										
					if (($$ = malloc(sizeof(char) * (STRL + 1))) == NULL) {
			  			printf ("Varid: Out of memory!\n");
			  			exit(EXIT_FAILURE);		
					}
					if (phase == 1 && (yylineno <= endpreamble || (yylineno >= startline && yylineno <= endline))) {
						if ($3->type != NUM && $3->type != NUMEXP)
							yyerror("invalid expression"); 											
						sprintf($$, "%s[%d]", $1, Evaluate($3, NULL, NULL));					
						DeleteAst($3);
					}	
					free($1);					
				}
			;            
            
ariexp		:   ariexp '+' ariexp {
					if (phase == 1 && (yylineno <= endpreamble || (yylineno >= startline && yylineno <= endline))) {
						if (($1->type == NUM || $1->type == NUMEXP)	&& ($3->type == NUM || $3->type == NUMEXP))
							$$ = NewAst(NUMEXP, PLUS, NULL, 0, $1, $3);
						else 												
							$$ = NewAst(EXP, PLUS, NULL, 0, $1, $3);		
					}	
				}
			|	ariexp '-' ariexp {
					if (phase == 1 && (yylineno <= endpreamble || (yylineno >= startline && yylineno <= endline))) {
						if (($1->type == NUM || $1->type == NUMEXP)	&& ($3->type == NUM || $3->type == NUMEXP))
							$$ = NewAst(NUMEXP, MINUS, NULL, 0, $1, $3);
						else 	
							$$ = NewAst(EXP, MINUS, NULL, 0, $1, $3);
					}										
				}	
			| 	ariexp '*' ariexp {
					if (phase == 1 && (yylineno <= endpreamble || (yylineno >= startline && yylineno <= endline))) {
						if (($1->type == NUM || $1->type == NUMEXP)	&& ($3->type == NUM || $3->type == NUMEXP))
							$$ = NewAst(NUMEXP, TIMES, NULL, 0, $1, $3);
						else 												 
							$$ = NewAst(EXP, TIMES, NULL, 0, $1, $3);
					}		
				}
			|	ariexp '/' ariexp {
					if (phase == 1 && (yylineno <= endpreamble || (yylineno >= startline && yylineno <= endline))) {
						if (($1->type == NUM || $1->type == NUMEXP)	&& ($3->type == NUM || $3->type == NUMEXP))
							$$ = NewAst(NUMEXP, DIV, NULL, 0, $1, $3);
						else 												
							$$ = NewAst(EXP, DIV, NULL, 0, $1, $3);
					}		
				}
			|	ariexp '%' ariexp {
					if (phase == 1 && (yylineno <= endpreamble || (yylineno >= startline && yylineno <= endline))) {
						if (($1->type == NUM || $1->type == NUMEXP)	&& ($3->type == NUM || $3->type == NUMEXP))
							$$ = NewAst(NUMEXP, MOD, NULL, 0, $1, $3);
						else 												
							$$ = NewAst(EXP, MOD, NULL, 0, $1, $3);
					}		
				}	
			|	'(' ariexp ')' {
					if (phase == 1 && (yylineno <= endpreamble || (yylineno >= startline && yylineno <= endline))) {										
						if ($2->type == VAR) 
							$$ = NewAst(VAR, 0, $2->node.var, 1, $2->l, $2->r);
						else if ($2->type == NUM)
							$$ = NewAst(NUM, $2->node.num, NULL, 1, $2->l, $2->r);				
						else 
							$$ = NewAst($2->type, $2->node.op, NULL, 1, $2->l, $2->r);
						free($2);		
					}		 																
				}
			|	'-' ariexp %prec UMINUS {										
					if (phase == 1 && (yylineno <= endpreamble || (yylineno >= startline && yylineno <= endline))) {									
						if ($2->type == NUM || $2->type == NUMEXP)
							$$ = NewAst(NUMEXP, MINUS, NULL, 0, NULL, $2);
						else 																	
							$$ = NewAst(EXP, MINUS, NULL, 0, NULL, $2);	
					}																	
				}	
			|	varid {
					list *node;
            		intvar *var = NULL;
            		int found = 0;
            		
            		if (phase == 1 && (yylineno <= endpreamble || (yylineno >= startline && yylineno <= endline))) {            				        		
		        		if (variables) {		
							node = variables;
							do {
								var = (intvar *) node->data;
								if (strcmp(var->id, $1) == 0) {
									found = 1;
									break;
								}	
								node = node->next;
							} while (node != variables);							
						}
						if (!found)
							yyerror("undeclared variable");           						
						$$ = NewAst(VAR, 0, var, 0, NULL, NULL);						 
					}	
					free($1);
				}
			|	NAT_NUMBER {
					if (phase == 1 && (yylineno <= endpreamble || (yylineno >= startline && yylineno <= endline))) 						
						$$ = NewAst(NUM, (int) $1, NULL, 0, NULL, NULL);						
				}
			|	PID {
					if (phase == 1 && (yylineno <= endpreamble || (yylineno >= startline && yylineno <= endline))) 
						$$ = NewAst(NUM, pid, NULL, 0, NULL, NULL);	
				}		
          	;          	
            
sequence    :   assignment {
					if (phase == 1 && (yylineno <= endpreamble || (yylineno >= startline && yylineno <= endline)))
						$$ = $1;
				}			
            |   sequence ';' assignment { 
            		if (phase == 1 && (yylineno <= endpreamble || (yylineno >= startline && yylineno <= endline))) {                     
		                $$ = ComposeActions($1, $3); 
		                DeleteList(&$1->assignments, NULL); 
		                DeleteList(&$3->assignments, NULL);                  
		                DeleteAction($1); 
		                DeleteAction($3);
		            }    
                }                                   	      
            ;

condition	: 	boolexp {
					if (phase == 1 && (yylineno <= endpreamble || (yylineno >= startline && yylineno <= endline))) {
						$$ = MakeCondition(CopyAst($1));
						DeleteAst($1);
					}	
				}
			;	                       
            
boolexp		:	ariexp IS_EQUAL ariexp {
					if (phase == 1 && (yylineno <= endpreamble || (yylineno >= startline && yylineno <= endline))) 
						$$ = NewAst(EXP, EQ, NULL, 0, $1, $3);						
				}
			|   ariexp IS_L ariexp {
					if (phase == 1 && (yylineno <= endpreamble || (yylineno >= startline && yylineno <= endline))) 
						$$ = NewAst(EXP, L, NULL, 0, $1, $3);						
				}
			|	ariexp IS_LEQ ariexp {
					if (phase == 1 && (yylineno <= endpreamble || (yylineno >= startline && yylineno <= endline)))	
						$$ = NewAst(EXP, LEQ, NULL, 0, $1, $3);
				} 
			|	ariexp IS_G ariexp {
					if (phase == 1 && (yylineno <= endpreamble || (yylineno >= startline && yylineno <= endline))) 
						$$ = NewAst(EXP, G, NULL, 0, $1, $3);
				} 
			|	ariexp IS_GEQ ariexp {
					if (phase == 1 && (yylineno <= endpreamble || (yylineno >= startline && yylineno <= endline))) 
						$$ = NewAst(EXP, GEQ, NULL, 0, $1, $3);
				} 
			|	ariexp NOT_EQUAL ariexp {
					if (phase == 1 && (yylineno <= endpreamble || (yylineno >= startline && yylineno <= endline)))
						$$ = NewAst(EXP, NEQ, NULL, 0, $1, $3);
				} 			
			|	boolexp LOGICAL_AND boolexp {
					if (phase == 1 && (yylineno <= endpreamble || (yylineno >= startline && yylineno <= endline))) 
						$$ = NewAst(EXP, AND, NULL, 0, $1, $3);
				}
			|	boolexp LOGICAL_OR boolexp {
					if (phase == 1 && (yylineno <= endpreamble || (yylineno >= startline && yylineno <= endline))) 
						$$ = NewAst(EXP, OR, NULL, 0, $1, $3);
				}
			| 	'(' boolexp ')' {
					if (phase == 1 && (yylineno <= endpreamble || (yylineno >= startline && yylineno <= endline))) {
						$$ = NewAst(EXP, $2->node.op, NULL, 1, $2->l, $2->r);
						free($2);
					}	
				}	
			| 	LOGICAL_NEG boolexp {															
					if (phase == 1 && (yylineno <= endpreamble || (yylineno >= startline && yylineno <= endline))) 
						$$ = NewAst(EXP, NOT, NULL, 0, NULL, $2);																								
				}
			;        			
			
procs		: 	procdef '(' ')' '{' stmnts endproc {;}
			|	procs procdef '(' ')' '{' stmnts endproc {;}
			;		            
            
procdef		: 	ACTIVE PROCTYPE STRING {
					vector *vec;
					int i;	
									
					if (phase == 0) { /*determine the number of programgraphs and where they are defined*/
						vec = NewVector(startlines->dim + 1, sizeof(int));
						for (i = 0; i < startlines->dim; i++)
							((int *) vec->coord)[i] = ((int *) startlines->coord)[i];
						((int *) vec->coord)[startlines->dim] = yylineno;
						DeleteVector(startlines);
						startlines = vec;
						vec = NewVector(times->dim + 1, sizeof(int));
						for (i = 0; i < times->dim; i++)
							((int *) vec->coord)[i] = ((int *) times->coord)[i];
						((int *) vec->coord)[times->dim] = 1;
						DeleteVector(times);
						times = vec;
					}
					else if (yylineno >= startline && yylineno <= endline) /*current programgraph*/										
						strcpy(pg->id, $3);						
                    free($3); 
				}
			| 	ACTIVE '[' NAT_NUMBER ']' PROCTYPE STRING {					
					vector *vec;
					int i;
																			
					if (phase == 0) {
						if ($3 == 0) 
							yyerror("array not well defined");
						if ($3 > 100) 
							yyerror("array dimension > 100");
						vec = NewVector(startlines->dim + 1, sizeof(int));
						for (i = 0; i < startlines->dim; i++)
							((int *) vec->coord)[i] = ((int *) startlines->coord)[i];
						((int *) vec->coord)[startlines->dim] = yylineno;
						DeleteVector(startlines);
						startlines = vec;
						vec = NewVector(times->dim + 1, sizeof(int));
						for (i = 0; i < times->dim; i++)
							((int *) vec->coord)[i] = ((int *) times->coord)[i];
						((int *) vec->coord)[times->dim] = (int) $3;
						DeleteVector(times);
						times = vec;
					}
					else if (yylineno >= startline && yylineno <= endline) 						
						strcpy(pg->id, $6);
                    free($6);
				}			
			;			                                                    
                                                   			
stmnts      :	stmnt {;}																
			|   stmnts ';' stmnt {;}				   		       
            ;            
            
stmnt      	:	transition {;} 			
            |   locmarker transition {;}              
            |	doopen options doclose {;}				
			|	locmarker doopen options doclose {;}			                                             								
            |   ifopen options ifclose {;}
			| 	locmarker ifopen options ifclose {;} 			            	 	                                  
            |   GOTO loclabel { /*GOTO is supported after a transition or a FI but not at the beginning of the process, after another GOTO, after a DO loop, or at double colons*/               
                    list *node;
                    location *loc, *markedloc = NULL;                                        
                    label *dang;
                    
                    if (phase == 1 && yylineno >= startline && yylineno <= endline) {                   
                    	if (!currentloc)
                    		yyerror("unsupported GOTO"); 
/*check whether a location with the given label already exists*/
                    	if (pg->loclist) {                
		                    node = pg->loclist;
		                    do {
		                        loc = (location *) node->data;                            
		                        if (strcmp($2, loc->label) == 0) {
		                        	markedloc = loc;		                            
		                            break;
		                        }     
		                        node = node->next;  
		                    } while (node != pg->loclist);
		                } 
/*if such a location exists, substitute the current location by the one with the given label*/
		                if (markedloc) 
		                	MergeCurrentLoc(markedloc);               			          
/*if no such location exists, label the current location with the given label and mark this label dangling*/	            
		                else {		            	   	
		                    strcpy(currentloc->label, $2);
		                    dang = NewLabel($2, yylineno);
		                    danglinglocs = InsertElement(dang, danglinglocs);		                    	                    
		                }
/*in all cases*/		                
		                currentloc = NULL;    		               
		            }      
                    free($2);                 
                }
            |	ASSERT '(' boolexp ')' { /*ignored*/ 
            		if (phase == 1 && yylineno >= startline && yylineno <= endline)             		
            			DeleteAst($3);            			
            	}             
            |	loclabel ':'  ASSERT '(' boolexp ')' { /*ignored*/            	
            		if (phase == 1 && yylineno >= startline && yylineno <= endline)	            		
            			DeleteAst($5);
            		free($1);	            		
            	}         
            ;                        
                        
transition  :   assignment {														
					if (phase == 1 && yylineno >= startline && yylineno <= endline) 	                             		
			            CreateTransition(NULL, GetAction($1));		            		                                              
                }	              			               			               
            |   ATOMIC '{' sequence '}' {            		
					if (phase == 1 && yylineno >= startline && yylineno <= endline) 					                                 
		                CreateTransition(NULL, GetAction($3));		            		
                }
            |   ATOMIC '{' condition ARROW sequence '}' {                       		
					if (phase == 1 && yylineno >= startline && yylineno <= endline) {                             		              		              
			            CreateTransition($3, GetAction($5));
		                DeleteCondition($3);
		            }		                                                                                 
                } 
			|	STRING {
					char s[STRL], str[STRL];
					list *node;
					transition *trans;
					int exists = 0;
					
					if (phase == 1 && yylineno >= startline && yylineno <= endline) {														
						strcpy(s, $1);						
		                sprintf(str, "__%d", pid);
		                strcat(s, str);		                 
						node = defacts;
						do {
							trans = (transition *) node->data;
							if (strcmp(trans->act->id, s) == 0) {
								exists = 1;
								break;
							}	
							node = node->next;
						} while (node != defacts);
						if (!exists)
							yyerror("undefined transition");						
						CreateTransition(trans->cond, trans->act);				
					}				
					free($1);
				} 			              
            ;                           

locmarker   :   loclabel ':' {                    
                    list *node;
                    location *loc, *markedloc = NULL;
                    label *dang;
                    int dangling = 0;                                                 
                                        
                    if (phase == 1 && yylineno >= startline && yylineno <= endline) {
/*check whether a location with the given label already exists*/                                                             	                
		                if (pg->loclist) {                
		                    node = pg->loclist;
		                    do {
		                        loc = (location *) node->data;
		                        if (strcmp($1, loc->label) == 0) {		                            
		                            markedloc = loc;
		                            break;
		                        }     
		                        node = node->next;  
		                    } while (node != pg->loclist);
		                }
/*if such a location exists, check whether it is dangling or already used; if it is already used, yyerror that the label is already used, else make the location  the current one*/ 		                 
		                if (markedloc) {
		                	if (danglinglocs) {		                	
				            	node = danglinglocs;
				                do {
				                    dang = (label *) node->data;
				                    if (strcmp($1, dang->str) == 0)	{
				                    	dangling = 1;                            
				                        break;
				                    }    		                            
				                    node = node->next;  
				                } while (node != danglinglocs);
		                    }
		                    if (!dangling) 
		                		yyerror("label already used");		                    
		                    danglinglocs = node;
		                	Pop(&danglinglocs, DeleteLabel);			                	
			            	MergeCurrentLoc(markedloc); /*does nothing if there is no currentloc*/		                				       
			                currentloc = markedloc;				                                      				            	
		                }
/*if no location with the given label exists, label the current location*/		                		               			                                  
		                else {		                
		                	if (!currentloc) {				                     
				                currentloc = NewLocation();
				                if (!pg->loc_0)
									pg->loc_0 = currentloc;
				                pg->loclist = InsertElement(currentloc, pg->loclist);				         		                				      
				            }				                               
			                strcat(currentloc->label, $1);
		                }    
		            }         
                    free($1);                                                                  
                }                   
            ;
      
loclabel    :   STRING {;}
            ;                                                          

doopen      :   DO {
					if (phase == 1 && yylineno >= startline && yylineno <= endline) 
						Open(DOOD);									                		                                  
	            } 		                
            ;  
            
options		: 	option {;}
			|	options option {;}
			;                     
            
option      :  	doublecolon transition {;}					
			|	doublecolon transition ';' stmnts {;}			
			|	doublecolon ifopen options ifclose {;}			
			|	doublecolon ifopen options ifclose ';' stmnts {;}				
            ;
            			               
doublecolon :   DOUBLECOLON {                					
					if (phase == 1 && yylineno >= startline && yylineno <= endline) 					
						Close(OPT); /*does nothing at the first double colon in a block (except enabling action for the next double colon)*/	
				}         
			;                       
						
ifopen      :   IF {
					if (phase == 1 && yylineno >= startline && yylineno <= endline) 							
						Open(IFFI);	     		               
                }                                                               
            ;             
            
ifclose     :   FI {                					
					if (phase == 1 && yylineno >= startline && yylineno <= endline) 						
						Close(IFFI); 												                          
                }
            ;                                                      

doclose     :   OD {                                 
                    if (phase == 1 && yylineno >= startline && yylineno <= endline)                     
						Close(DOOD);                   	                                                    
                }           
            ;                       
            
endproc		:	'}' {						 				
					vector *vec;
					int i;
																				
					if (phase == 0) {
						vec = NewVector(endlines->dim + 1, sizeof(int));
						for (i = 0; i < endlines->dim; i++)
							((int *) vec->coord)[i] = ((int *) endlines->coord)[i];
						((int *) vec->coord)[endlines->dim] = yylineno;
						DeleteVector(endlines);
						endlines = vec;
					}
					else {						
						if (danglinglocs) {
							printf("Error near line %d: undefined location \"%s\"\n", ((label *) danglinglocs->data)->num, ((label *) danglinglocs->data)->str);
							exit(EXIT_FAILURE);
						}	
					}																										
				}	
		    ;                                            
%%


/*Function implementations*/

/*Constructs variables and inserts them in the list of variables. The number of variables to be constructed is given by the dimension of the vector vals, which contains the initial values. The variables have the given name, with index in the case of more than one variable. The initial values are used to extend the initialexp.*/ 

static void MakeVariables(const char *name, const vector *vals) {

	char s[strlen(name) + 4];
	list *vlist;
	intvar *va, *var;
	int k, val;	                                                  
    ast *varexp, *numexp, *eqexp;
    
    strcpy(s, name);
    strcat(s, "[0]");        
    if (variables) {
        vlist = variables;
        do {
            va = (intvar *) vlist->data;
            if (strcmp(va->id, name) == 0 || strcmp(va->id, s) == 0) 						                
                yyerror("variable already defined");	                	            
            vlist = vlist->next;
        } while (vlist != variables);
    }                                 
    for (k = 0; k < vals->dim; k++) {
    	val = ((int *) vals->coord)[k];
        var = NewVariable(0);     
        var->initialval = val;    
        if (vals->dim > 1)           
	        sprintf(var->id, "%s[%d]", name, k); 
	    else 
	        strcpy(var->id, name);                                                       
        variables = InsertElement(var, variables);
        varexp = NewAst(VAR, 0, var, 0, NULL, NULL);                                  
        numexp = NewAst(NUM, val, NULL, 0, NULL, NULL);
        eqexp = NewAst(EXP, EQ, NULL, 1, varexp, numexp); 
        if (initialexp) 
        	initialexp = NewAst(EXP, AND, NULL, 0, initialexp, eqexp);           	                    		
        else 
        	initialexp = eqexp;        	
    }	                            
}


/*Creates a new transition with the given guard condition and the given action and inserts it in the defines list*/

static void DefTransition(condition *guard, action *act) { 
    
    list *node;    
    transition *t, *trans;      
            
    if (defacts) {
		node = defacts;
		do {	
			t = (transition *) node->data;
			if (strcmp(t->act->id, act->id) == 0) 
				yyerror("transition already defined");											
			node = node->next;
		} while (node != defacts);							
    }
    trans = NewTransition();
    trans->act = act;                
    if (guard) {  
    	DeleteCondition(trans->cond);
        trans->cond = guard; 
	}          
	defacts = InsertElement(trans, defacts);
}


/*Creates an action that assigns the given expression to the given variable*/

static action *MakeAssignAction(intvar *var, ast *exp) {

	action *act;
	assignment *ass;
		
	act = NewAction(1, 1);	
	ass = NewAssignment();
	ass->var = var;
	ass->exp = exp;		
	act->assignments = InsertElement(ass, act->assignments);			
	ActionName(act->id, act);
	return act;
}


/*Composes the given actions*/

static action *ComposeActions(const action *seq, const action *act) {

	action *composite;
	
	composite = NewAction(1, 1);
	ConcatLists(&composite->assignments, seq->assignments);
	ConcatLists(&composite->assignments, act->assignments);
	strcpy(composite->id,seq->id);
	strcat(composite->id,";");
	strcat(composite->id,act->id);
	return composite;
}


/*Creates a condition representing the given expression*/

static condition *MakeCondition(ast *exp) {

	condition *cond;
	
	cond = NewCondition();
	DeleteAst(cond->exp);
	cond->exp = exp;
	return cond;
}


/*Merges the current location into mergeloc and deletes the current location (if both locations exist and are different)*/

static void MergeCurrentLoc(location *mergeloc) {

	list *node;
	vector *vec;
	transition *trans;
    location *loc;
           
    if (mergeloc && currentloc && currentloc != mergeloc) { 
		if (strcmp(currentloc->label, "")  != 0) /*precaution, should not be necessary*/
			yyerror("confusing location labels");
		if (blks) { /*precaution, should not be necessary*/
			node = blks;
			do {
				vec = (vector *) node->data;
				if (currentloc == ((location **) vec->coord)[0] || currentloc == ((location **) vec->coord)[1])
					yyerror("confusing control structure");	
				node = node->next;
			} while (node != blks);
		}	
		if (currentloc->intranslist) {
			node = currentloc->intranslist;
			do { 
				trans = (transition *) node->data;
				trans->loc[1] = mergeloc;
				mergeloc->intranslist = InsertElement(trans, mergeloc->intranslist); 	
				node = node->next;
			} while (node != currentloc->intranslist);			
		}
		if (currentloc->outtranslist) {
			node = currentloc->outtranslist;
			do { 
				trans = (transition *) node->data;
				trans->loc[0] = mergeloc;
				mergeloc->outtranslist = InsertElement(trans, mergeloc->outtranslist); 	
				node = node->next;
			} while (node != currentloc->outtranslist);			
		}
		if (pg->loc_0 == currentloc) 
			pg->loc_0 = mergeloc;								
		node = pg->loclist;
		do {
		    loc = (location *) node->data;                            
		    if (loc == currentloc) 
		        break;              
		    node = node->next;  
		} while (node != pg->loclist);			
		pg->loclist = node;                        
		Pop(&pg->loclist, DeleteLocation);			
    }		 				                				           			
}


/*Returns the registered version of the given action*/

static action *GetAction(action *act) { 
    
    list *node;
	action *a = NULL;
	int exists = 0;	            
                                       
    if (pg->actlist) {
    	node = pg->actlist;
    	do {
    		a = (action *) node->data;                		
    		if (strcmp(a->id, act->id) == 0) {                                        		                    			
    			exists = 1;
    			break;
    		}
    		node = node->next;
    	} while (node != pg->actlist);
    }
    if (exists) 
    	DeleteAction(act);		                	    	
    else {
    	a = act;
    	pg->actlist = InsertElement(a, pg->actlist); 
    } 
    return a;    		                                          
}


/*Sets the previous and the current location and creates a new transition between them with the given guard condition and action*/

static void CreateTransition(const condition *guard, action *act) { 
        
    transition *trans = NewTransition();
    
    if (!currentloc) {
    	currentloc = NewLocation();
    	if (!pg->loc_0)
			pg->loc_0 = currentloc;
		pg->loclist = InsertElement(currentloc, pg->loclist);			
    }             
    previousloc = currentloc;          
    currentloc = NewLocation();
    if (!pg->loc_0)
		pg->loc_0 = currentloc;
    pg->loclist = InsertElement(currentloc, pg->loclist);                            
    trans->act = act;
    if (guard) { 
  		DeleteAst(trans->cond->exp);  
    	trans->cond->exp = CopyAst(guard->exp);
	}                 
    trans->loc[0] = previousloc;
    trans->loc[1] = currentloc;
    trans->loc[0]->outtranslist = InsertElement(trans, trans->loc[0]->outtranslist);
    trans->loc[1]->intranslist = InsertElement(trans, trans->loc[1]->intranslist);
    pg->translist = InsertElement(trans, pg->translist);            
}


/*Opens block of the indicated type*/

static void Open(int type) {

	vector *locs = NewVector(2, sizeof(location *));

    if (!currentloc) {
        currentloc = NewLocation();
        if (!pg->loc_0)
			pg->loc_0 = currentloc;
        pg->loclist = InsertElement(currentloc, pg->loclist);		                        
    } 		                     		       
    ((location **) locs->coord)[0] = currentloc;
    if (type == DOOD) 
    	((location **) locs->coord)[1] = currentloc;
    else /*IFFI*/
    	((location **) locs->coord)[1] = NULL;		                
    blks = InsertElement(locs, blks);
    wait = 1; /*do nothing at the first double colon of the block*/
}


/*Closes block or option according to the indicated type*/

static void Close(int type) {

	vector *locs = (vector *) blks->prev->data;
	
	if (wait) /*only at the first double colon in a block*/
		wait = 0;	
	else { 
		if (!currentloc) {
			currentloc = NewLocation();
			if (!pg->loc_0) 
				pg->loc_0 = currentloc;
			pg->loclist = InsertElement(currentloc, pg->loclist);		                        									
		}
		if (!((location **) locs->coord)[1])
			((location **) locs->coord)[1] = currentloc;
		MergeCurrentLoc(((location **) locs->coord)[1]);				
		if (type == OPT) 																																		
			currentloc = ((location **) locs->coord)[0];
		else if (type == DOOD)
			currentloc = NULL;
		else /*IFFI*/ 
			currentloc = ((location **) locs->coord)[1];
		if (type == DOOD || type == IFFI) {				
			blks = blks->prev;
		    Pop(&blks, DeleteVector);
		} 
	}				
}


/*Parses the given file in two modes: if create == 0, the sections containing process definitions are located; if create == 1, the programgraph described in the section at the head of the section list is created and this section is then removed from the list; the variables are only constructed when the very first programgraph is created*/

void ParsePML(FILE *fp, list **varlist, programgraph *pgraph, list **sections, const int create) {
    	    
	int i, j, locindex = 0;
	vector *sec;
   	list *node;
   	location *loc;
     
	if (!create) {
 		phase = 0;
 		startlines = NewVector(0, sizeof(int));
 		times = NewVector(0, sizeof(int));
 		endlines = NewVector(0, sizeof(int));  		
 		yyrestart(fp); 	  	 	
   		yyparse();   		
   		for (i = 0; i < endlines->dim; i++) {
   			for (j = 0; j < ((int *) times->coord)[i]; j++) {
				sec = NewVector(3, sizeof(int));
				((int *) sec->coord)[0] = ((int *) startlines->coord)[i];
				((int *) sec->coord)[1] = ((int *) endlines->coord)[i];				
				((int *) sec->coord)[2] = j > 0 ? 1 : 0;
				*sections = InsertElement(sec, *sections);
			}	     			
   		} 
   		endpreamble = ((int *) startlines->coord)[0] - 1;
   		DeleteVector(startlines);
   		DeleteVector(times);
   		DeleteVector(endlines);   		
   		pid = -1; 
   		variables = NULL;
   		initialexp = NULL;  					 		
 	}
 	else {
		phase = 1;
		pid++;
		pg = pgraph;
		defacts = NULL; 
		blks = NULL; 		
		currentloc = NULL;
		previousloc = NULL;
		danglinglocs = NULL;
		sec = (vector *) (*sections)->data;			
		startline = ((int *) sec->coord)[0];
		endline = ((int *) sec->coord)[1];
		rep = ((int *) sec->coord)[2];									 								
		yyrestart(fp); 	 
		yyparse();  		
  		if (!*varlist) 
 			ConcatLists(varlist, variables); 			 			
 		ConcatLists(&pg->varlist, variables);	
		if (pg->loclist) {
   			node = pg->loclist;
   			do {
   				loc = (location *) node->data;
   				if (loc == pg->loc_0) 
   					break;   					
   				node = node->next;													
			} while (node != pg->loclist);
	 		pg->loclist = node;
   			do {
   				loc = (location *) node->data;
   				loc->index = locindex++;
   				if (strcmp(loc->label, "end") == 0) 
   					pg->loc_1 = loc;   				
   				node = node->next;													
			} while (node != pg->loclist);
	 	}											
		DeleteCondition(pg->cond_0);			
		pg->cond_0 = MakeCondition(CopyAst(initialexp));
		pg->no_vars = NumberOfElements(pg->varlist);
		pg->no_acts = NumberOfElements(pg->actlist);
		pg->no_trans = NumberOfElements(pg->translist);	
		DeleteList(&defacts, DeleteTransition);
		Pop(sections, DeleteVector);											 	   	
	   	if (!*sections) {  
		   	DeleteList(&variables, NULL);
		   	DeleteAst(initialexp);	   	
		}   		   		
 	} 
 	yylex_destroy();    
}


/*Reports errors in the input file*/

void yyerror(char const *s) {
    
	fprintf(stderr, "Error near line %d: %s\n", yylineno, s);  
    exit(EXIT_FAILURE);      
}

