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
File pml2pg.l

This file provides a Flex scanner for the parser implemented in pml2pg.y.
************************************************************************************************/

%{
#include <ctype.h>
#include <limits.h>
#include "vector.h"
#include "pgraph.h"
#include "pml2pg.tab.h"


void yyerror(char const *s);
%}

%option noinput
%option nounput
%option noyywrap
%option yylineno


%%
[/][*]([^*]|[*]+[^/*])*[*]+[/]	 	; /*ignore comments*/		
"::"                            	{return DOUBLECOLON;}
"=="                            	{return IS_EQUAL;}
"!="                            	{return NOT_EQUAL;}
">="                            	{return IS_GEQ;}
">"                             	{return IS_G;}
"<="                            	{return IS_LEQ;}
"<"                             	{return IS_L;}
"&&"                            	{return LOGICAL_AND;}
"||"                            	{return LOGICAL_OR;}
"!"                             	{return LOGICAL_NEG;}
"->"                            	{return ARROW;}
"++"                           		{return INCREMENT;}
"--"                            	{return DECREMENT;}
"#define"                       	{return DEF;}
"_pid" 								{return PID;}
"active"                			{return ACTIVE;}
"assert"							{return ASSERT;}
"atomic"                        	{return ATOMIC;}
"do"                            	{return DO;}
"fi"                            	{return FI;}
"goto"                          	{return GOTO;}
"if"                            	{return IF;}
"int"								{return INTEGER;}
"ltl"								{yyterminate();} /*stop when ltl formula is found*/
"never"								{yyterminate();} /*stop when never claim is found*/
"od"                            	{return OD;} 
"proctype"							{return PROCTYPE;} 
"skip"                          	{return SKIP;} 
[a-zA-Z][_0-9a-zA-Z]*				{yylval.str = strdup(yytext); return STRING;}	
[0-9]*		           		       	{
										if (strlen(yytext) > 1 && yytext[0] == '0') 
											yyerror("leading 0");																				
										if (strlen(yytext) > 10 || strtoll(yytext, NULL, 10) >  (long long) INT_MAX + 1) 	
											yyerror("value out of range"); 	/*rejects numbers outside [0, INT_MAX + 1]*/					
										yylval.ll = strtoll(yytext, NULL, 10); 
										return NAT_NUMBER;
									}						
[;,:\[\](){}=+\-*/%]               	{return yytext[0];} 
[ \t\n]                         	; /*ignore white space*/
.                               	{
										char s[STRL]; 
										if (isprint(yytext[0])) 
											sprintf(s, "unexpected '%c'", yytext[0]); 
										else 
											strcpy(s, "unrecognized character");		
										yyerror(s);
									}
%%


