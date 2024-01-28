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
File def.h

This file declares global variables and defines macros used in various modules.
************************************************************************************************/

#ifndef DEF_H
#define DEF_H   

/*Macros for command-line options, see README*/

#define OPTION_old 1
#define OPTION_s 1
#define OPTION_i 2
#define OPTION_c 3
#define OPTION_t 4

/*Maximal string length*/

#define STRL 1000

/*Macros for types*/

#define INT 0	 		/*integer*/
#define PTR	1			/*pointer*/
	
/*Global variables for command-line options, initialized in main.c*/

extern unsigned int out, inp;

#endif	
