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
File cube.c

This file implements the functions declared in cube.h.
************************************************************************************************/

#include "cube.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "def.h"
#include "vector.h"
#include "list.h"


/*Function prototypes*/

static vector *MergeVertices(const vector *v0, const vector *v1, int dim); /*Computes the vertices of a cube of degree dim >= 1 by joining the vertices of a front face (v0) and those of the corresponding back face (v1), the initial and the final vertex come first*/	
static int FillSquares(cube *edge, list *cubelist[]); /*Completes HDA by introducing squares at given 1-cube, returns the number of squares added*/	
static list *ExtendConfig(const vector *v2d, int edgeindex, int degree, const int pid[], const list *veclist, cube *const faces[]); /*See below*/	
static list *Config(const cube *edge, int degree, const list *veclist, cube *const faces[]); /*See below*/	
static list *CompleteUpperindices(int edgeindex, cube *const faces[], const vector *v, const int upperindex[]);	/*See below*/
static list *Upperindices(int edgeindex, cube *const faces[], const vector *v);	/*See below*/
static int BdIdsOK(const cube *pc); /*Returns 1 if cube satisfies the boundary identities and 0 else*/
static int FillHDCubes(cube *edge, list *cubelist[], int degree, int facecount);	


/*Function implementations*/

/*Creates new label*/

label *NewLabel(const char *str, int num) {

	label *lab;	

	if ((lab = malloc(sizeof(label))) == NULL) {
      		printf ("NewLabel: Out of memory!\n");
      		exit(EXIT_FAILURE);
   	}	
	if (str) {
		if ((lab->str = malloc(sizeof(char) * (strlen(str) + 1))) == NULL) {
      			printf ("NewLabel: Out of memory!\n");
      			exit(EXIT_FAILURE);		
		}
		strcpy(lab->str, str);
	}
	else
		lab->str = NULL;	
	lab->num = num;
	return lab;
}


/*Deletes given label*/

void DeleteLabel(void *lab) {

	label *l; 

	if (lab) {
		l = (label *) lab;
		if (l->str) {
			free(l->str);			
		}
		free(l);		
	}
}


/*Returns 0 if the two labels exist and are equal*/

int Labcmp(const void *l1, const void *l2) {

	int diff = -1;

	if (l1 && l2) {
		diff = 0;
		if (strcmp(((label *) l1)->str, ((label *) l2)->str) != 0 || ((label *) l1)->num != ((label *) l2)->num) 
			diff = 1;
	}
	return diff;
}


/*Concatenates two label words*/

vector *MultiplyLabelVectors(const vector *lv1, const vector *lv2) {

	vector *lv = NULL; 
	label **lv1c, **lv2c, **lvc;
	int i;

	if (lv1 && lv2) {
		lv1c = lv1->coord;
		lv2c = lv2->coord;
		lv = NewVector(lv1->dim + lv2->dim, sizeof(label *));
		lvc = lv->coord;
		for (i = 0; i < lv1->dim; i++) 
			lvc[i] = NewLabel(lv1c[i]->str, lv1c[i]->num);
		for (i = 0; i < lv2->dim; i++) 
			lvc[i + lv1->dim] = NewLabel(lv2c[i]->str, lv2c[i]->num);
	}
	return lv;	
}


/*Creates new int with value n*/

int *NewInt(int n) {

	int *m;

	if ((m = malloc(sizeof(int))) == NULL) {
      		printf ("NewInt: Out of memory!\n");
      		exit(EXIT_FAILURE);
   	}	
	*m = n;
	return m;	
}


/*Deletes given int*/

void DeleteInt(void *n) {

	int *m;

	if (n) {
		m = (int *) n;
		free(m);
	}
}


/*Creates new cube of degree d*/

cube *NewCube(unsigned int d) { 
   
	cube  *pc;
	int i;

	if ((pc = malloc(sizeof(cube))) == NULL) {
      		printf ("NewCube: Out of memory!\n");
      		exit(EXIT_FAILURE);
   	}
	pc->degree = d;	
	pc->flags.ini = 0;
	pc->flags.fin = 0;	
	if (d > 0) {
  		if ((pc->d[0] = malloc(sizeof(cube *) * d)) == NULL) {
      			printf ("NewCube: Out of memory!\n");
      			exit(EXIT_FAILURE);
   		}	
   		if ((pc->d[1] = malloc(sizeof(cube *) * d)) == NULL) {
      			printf ("NewCube: Out of memory!\n");
      			exit(EXIT_FAILURE);
   		}
		for (i = 0; i < d; i++) {
			pc->d[0][i] = NULL;
			pc->d[1][i] = NULL;	
		}	
	}
	else {
		pc->d[0] = NULL;
		pc->d[1] = NULL;		
	}		
   	if ((pc->s[0] = malloc(sizeof(list *) * (d + 1))) == NULL) {
      		printf ("NewCube: Out of memory!\n");
      		exit(EXIT_FAILURE);
   	}	
	if ((pc->s[1] = malloc(sizeof(list *) * (d + 1))) == NULL) {
      		printf ("NewCube: Out of memory!\n");
      		exit(EXIT_FAILURE);
   	}
	for (i = 0; i < d + 1; i++) {
		pc->s[0][i] = NULL;
		pc->s[1][i] = NULL;
	}
	pc->edges = NewVector(d, sizeof(cube *));
   	if (d == 1) 
		((cube **) pc->edges->coord)[0] = pc;
	pc->lab = NULL;
	pc->cl = NULL;
	pc->furtherdata = NULL;		
	return pc;
}


/*Deletes the given cube*/

void DeleteCube(void *pc) {

	int i;
	cube *c;

	if (pc) {
		c = (cube *) pc;
		if (c->lab) {
			for (i = 0; i < c->lab->dim; i++) 
				DeleteLabel(((label **) c->lab->coord)[i]); 
			DeleteVector(c->lab);
		}	
		DeleteVector(c->edges);	
		for (i = 0; i < c->degree + 1; i++) {
			DeleteList(&c->s[0][i], NULL);
			DeleteList(&c->s[1][i], NULL);
		}
		free(c->s[0]);
		free(c->s[1]);
		if(c->degree > 0) {
			free(c->d[0]);
			free(c->d[1]);			
		}			
		free(c);			
	}
}


/*Computes the vertices of a cube of degree dim >= 1 by joining the vertices of a front face (v0) and those of the corresponding back face (v1), the initial and the final vertex come first*/

static vector *MergeVertices(const vector *v0, const vector *v1, int dim) {

	vector *verts;
	cube **vertsc, **v0c = v0->coord, **v1c = v1->coord;
	unsigned int i, n;
	
	n = (unsigned int) pow(2, dim);
	verts = NewVector(n, sizeof(cube *));
	vertsc = verts->coord;
	if (dim == 1) {
		vertsc[0] = v0c[0];
		vertsc[1] = v1c[0];
    }
	else 
		for(i = 0; i < n / 4; i++) {
			vertsc[4 * i] = v0c[2 * i];
			vertsc[4 * i + 1] = v1c[2 * i + 1];
			vertsc[4 * i + 2] = v0c[2 * i + 1];
			vertsc[4 * i + 3] = v1c[2 * i];
		}	
	return verts;
}


/*Computes the vertices of a cube, the initial and the final vertex come first*/

vector *Vertices(cube *pc) {

	vector *verts, *verts0, *verts1;
	
	if (pc->degree == 0) { 
		verts = NewVector(1, sizeof(cube *));
		((cube **) verts->coord)[0] = pc;
	}
	else { 
		verts0 = Vertices(pc->d[0][0]);
		verts1 = Vertices(pc->d[1][0]);
		verts = MergeVertices(verts0, verts1, pc->degree);
		DeleteVector(verts0);
		DeleteVector(verts1);
	}
	return verts;
}


/*Computes the edges of pc starting in the initial vertex of pc*/

vector *Edges(cube *pc) {

	vector *edges = NewVector(pc->degree, sizeof(cube *)), *e, *f;
	cube **edgesc = edges->coord;
	int i;

	if (pc->degree == 1) 		
		edgesc[0] = pc;	
	else if (pc->degree == 2) {		
		edgesc[0] = pc->d[0][1];
		edgesc[1] = pc->d[0][0];	
	}
	else if (pc->degree > 2) {
		e = Edges(pc->d[0][pc->degree - 1]);
		f = Edges(pc->d[0][pc->degree - 3]);
		for (i = 0; i < pc->degree - 1; i++) 
			edgesc[i] = ((cube **) e->coord)[i];
		edgesc[pc->degree - 1] = ((cube **) f->coord)[pc->degree - 2];
		DeleteVector(e);
		DeleteVector(f);			 			
	}
	return edges;
}


/*Completes HDA by introducing squares at given 1-cube, returns the number of squares added*/

static int FillSquares(cube *edge, list *cubelist[]) {
	
	int squarecount = 0;
	list *clist, *clist2, *clist3;
	cube *pc, *pc2, *pc3, *newcube;
	label **pclabc, **edgelabc = (label **) edge->lab->coord;

/*squares with edge as a d[0]-boundary*/
	if (edge->d[0][0]->s[0][0]) {
		clist = edge->d[0][0]->s[0][0];
		do {
			pc = (cube *) clist->data;
			pclabc = (label **) pc->lab->coord; 
			if (edge->d[1][0]->s[0][0] && pclabc[0]->num != edgelabc[0]->num) {
				clist2 = edge->d[1][0]->s[0][0];
				do {
					pc2 = (cube *) clist2->data;
					if (pc->d[1][0]->s[0][0] && VecCmp(pc2->lab ,pc->lab, PTR, Labcmp) == 0) {
						clist3 = pc->d[1][0]->s[0][0];
						do{
							pc3 = (cube *) clist3->data;
							if (pc2->d[1][0] == pc3->d[1][0] && VecCmp(edge->lab, pc3->lab, PTR, Labcmp) == 0) {
								newcube = NewCube(2);								
								if (pclabc[0]->num < edgelabc[0]->num) {
									newcube->d[0][0] = edge;
									edge->s[0][0] = InsertElement(newcube, edge->s[0][0]);
									newcube->d[0][1] = pc;
									pc->s[0][1] = InsertElement(newcube, pc->s[0][1]);
									newcube->d[1][0] = pc3;
									pc3->s[1][0] = InsertElement(newcube, pc3->s[1][0]);
									newcube->d[1][1] = pc2;
									pc2->s[1][1] = InsertElement(newcube, pc2->s[1][1]);
								}
								else {							
									newcube->d[0][0] = pc;
									pc->s[0][0] = InsertElement(newcube, pc->s[0][0]);
									newcube->d[0][1] = edge;
									edge->s[0][1] = InsertElement(newcube, edge->s[0][1]);
									newcube->d[1][0] = pc2;
									pc2->s[1][0] = InsertElement(newcube, pc2->s[1][0]);
									newcube->d[1][1] = pc3;
									pc3->s[1][1] = InsertElement(newcube, pc3->s[1][1]);	
								}								
								((cube **) newcube->edges->coord)[0] = newcube->d[0][1];
								((cube **) newcube->edges->coord)[1] = newcube->d[0][0];																
								cubelist[2] = InsertElement(newcube, cubelist[2]);								
								newcube->cl = cubelist[2]->prev;
								squarecount++;					
							}
							clist3 = clist3->next;
						} while (clist3 != pc->d[1][0]->s[0][0]);
					}
					clist2 = clist2->next;
				} while (clist2 != edge->d[1][0]->s[0][0]);
			}
			clist = clist->next;
		} while (clist != edge->d[0][0]->s[0][0]);
	}	 	
/*squares with edge as a d[1]-boundary*/
	if (edge->d[1][0]->s[1][0]) {
		clist = edge->d[1][0]->s[1][0];
		do {
			pc = (cube *) clist->data;
			pclabc = (label **) pc->lab->coord;
			if (edge->d[0][0]->s[1][0] && pclabc[0]->num != edgelabc[0]->num) {
				clist2 = edge->d[0][0]->s[1][0];
				do{
					pc2 = (cube *) clist2->data;
					if (pc->d[0][0]->s[1][0] && VecCmp(pc2->lab, pc->lab, PTR, Labcmp) == 0) {
						clist3 = pc->d[0][0]->s[1][0];
						do{
							pc3 = (cube *) clist3->data;
							if (pc3 != edge && pc2->d[0][0] == pc3->d[0][0] && VecCmp(edge->lab, pc3->lab, PTR, Labcmp) == 0) {
								newcube = NewCube(2);								
								if (pclabc[0]->num < edgelabc[0]->num) {
									newcube->d[0][0] = pc3;
									pc3->s[0][0] = InsertElement(newcube, pc3->s[0][0]);
									newcube->d[0][1] = pc2;
									pc2->s[0][1] = InsertElement(newcube, pc2->s[0][1]);
									newcube->d[1][0] = edge;
									edge->s[1][0] = InsertElement(newcube, edge->s[1][0]);
									newcube->d[1][1] = pc;
									pc->s[1][1] = InsertElement(newcube, pc->s[1][1]);	
								}
								else{								
									newcube->d[0][0] = pc2;
									pc2->s[0][0] = InsertElement(newcube, pc2->s[0][0]);
									newcube->d[0][1] = pc3;
									pc3->s[0][1] = InsertElement(newcube, pc3->s[0][1]);
									newcube->d[1][0] = pc;
									pc->s[1][0] = InsertElement(newcube, pc->s[1][0]);
									newcube->d[1][1] = edge;
									edge->s[1][1] = InsertElement(newcube, edge->s[1][1]);	
								}
								((cube **) newcube->edges->coord)[0] = newcube->d[0][1];
								((cube **) newcube->edges->coord)[1] = newcube->d[0][0];								
								cubelist[2] = InsertElement(newcube, cubelist[2]);								
								newcube->cl = cubelist[2]->prev;
								squarecount++;					
							}
							clist3 = clist3->next;
						} while (clist3 != pc->d[0][0]->s[1][0]);
					}
					clist2 = clist2->next;
				} while (clist2 != edge->d[0][0]->s[1][0]);
			}
			clist = clist->next;
		} while (clist != edge->d[1][0]->s[1][0]);
	}
	return squarecount;	 	
} 


/*At a fixed edge, suppose that all cubes of dimension degree-1 (all containing the edge) have been constructed, that they have been put in the faces array, and that veclist represents the indexes of the faces (as 1D vectors). Suppose that the 2D vector v2d contains the indexes of two faces that from the point of view of process IDs could be part of the boundary of a cube of dimension degree. Suppose that edgeindex is the lower index of the starting edge of that cube that is parallel to the edge under consideration, and suppose that the cube's pid is given by pid. ExtendConfig() returns the list of all face index vectors beginning with v2d representing degree-1 faces that from the point of view of process IDs could be part of the boundary of the cube.*/

static list *ExtendConfig(const vector *v2d, int edgeindex, int degree, const int pid[], const list *veclist, cube *const faces[]) {

	list *config = NULL, *product, *dlist = NULL;
	const list *vlist;
	int d, i, j;
	vector *v, *copy = NewVector(2, sizeof(int)), *vec = NewVector(degree - 1, sizeof(int)), *hat = NewVector(degree - 1, sizeof(int));

	((int *) copy->coord)[0] = ((int *) v2d->coord)[0];
	((int *) copy->coord)[1] = ((int *) v2d->coord)[1];
	config = InsertElement(copy, config);
	for (d = 2; d < degree - 1 && config; d++) {
		if (veclist) {
			vlist = veclist;
			do {
				v = (vector *) vlist->data;		
				for (j = 0; j < degree - 1; j++)
					((int *) vec->coord)[j] = ((label **) ((cube **) faces[((int *) v->coord)[0]]->edges->coord)[j]->lab->coord)[0]->num;
				i = d < edgeindex ? d : d + 1;
				for (j = 0; j < i; j++)
					((int *) hat->coord)[j] = pid[j];
				for (j = i; j < degree - 1; j++)
					((int *) hat->coord)[j] = pid[j+1];				
				if (Veccmp(hat, vec) == 0) 
					dlist = InsertElement(v, dlist);				
				vlist = vlist->next;
			} while (vlist != veclist);
		}
		product = Product(config, dlist, INT);
		DeleteList(&config, DeleteVector);
		DeleteList(&dlist, NULL);
		config = product;			
	}
	DeleteVector(hat);
	DeleteVector(vec);
	return config;	
}


/*At the given edge, suppose that all cubes of dimension degree-1 (all containing edge) have been constructed, that they have been put in the faces array, and that veclist represents the indexes of the faces (as 1D vectors). Config() returns the list of all face index vectors representing degree-1 faces that from the point of view of process IDs could be part of the boundary of a cube of dimension degree.*/

static list *Config(const cube *edge, int degree, const list *veclist, cube *const faces[]) {

	vector *v, *vec = NewVector(degree - 1, sizeof(int)), *hat = NewVector(degree - 1, sizeof(int));
	int i, j, d, maybe, edgeindex, pid[degree], *vc;	
	list *config = NULL, *product, *helpconfig;
	const list *vlist;
		
	product = Product(veclist, veclist, INT);
	if (product) {
		vlist = product;
		do {	
			v = (vector *) vlist->data;
			vc = v-> coord;	
			maybe = 1;
			for (i = 0; i < degree - 1; i++) {
				if (((label **) ((cube **) faces[vc[0]]->edges->coord)[i]->lab->coord)[0]->num == ((label **) edge->lab->coord)[0]->num) {
					edgeindex = i;
					break;
				}
			}	
			if (edgeindex == 0 && ((label **) ((cube **) faces[vc[1]]->edges->coord)[0]->lab->coord)[0]->num == ((label **) edge->lab->coord)[0]->num) {	
				if (((label **) ((cube **) faces[vc[1]]->edges->coord)[1]->lab->coord)[0]->num == ((label **) ((cube **) faces[vc[0]]->edges->coord)[1]->lab->coord)[0]->num) 
					maybe = 0;
				else {
					pid[0] = ((label **) edge->lab->coord)[0]->num;				
					pid[1] = ((label **) ((cube **) faces[vc[1]]->edges->coord)[1]->lab->coord)[0]->num;	
					for (i = 2; i < degree; i++) 
						pid[i] = ((label **) ((cube **) faces[vc[0]]->edges->coord)[i - 1]->lab->coord)[0]->num;					
				}		
			}	
			else {
				if (((label **) ((cube **) faces[vc[1]]->edges->coord)[0]->lab->coord)[0]->num >= ((label **) ((cube **) faces[vc[0]]->edges->coord)[0]->lab->coord)[0]->num) 
					maybe = 0;						
				else {	
					edgeindex += 1;
						pid[0] = ((label **) ((cube **) faces[vc[1]]->edges->coord)[0]->lab->coord)[0]->num;
					for (i = 1; i < degree; i++) 
						pid[i] = ((label **) ((cube **) faces[vc[0]]->edges->coord)[i - 1]->lab->coord)[0]->num;
				}			
			}
			for (d = 0; d <= 1 && maybe; d++) {
				for (j = 0; j < degree - 1; j++)
					((int *) vec->coord)[j] = ((label **) ((cube **) faces[vc[d]]->edges->coord)[j]->lab->coord)[0]->num;
				i = d < edgeindex ? d : d + 1;					
				for (j = 0; j < i; j++)
					((int *) hat->coord)[j] = pid[j];
				for (j = i; j < degree - 1; j++)
					((int *) hat->coord)[j] = pid[j + 1];
				if (Veccmp(hat, vec) != 0) 
					maybe = 0;						
			}																						
			if (maybe) {			
				helpconfig = ExtendConfig(v, edgeindex, degree, pid, veclist, faces);
				ConcatLists(&config, helpconfig); 
				DeleteList(&helpconfig, NULL);
			}
			vlist = vlist->next;
		} while (vlist != product);
		DeleteList(&product, DeleteVector);
	}
	DeleteVector(hat);
	DeleteVector(vec);	
	return config;
}


/*At a fixed edge, suppose that all cubes of dimension degree-1 (all containing the edge) have been constructed and that they have been put in the faces array. Suppose that the vector v contains the indexes of degree-1 faces that from the point of view of process IDs could be part of the boundary of a cube of dimension degree. Suppose that edgeindex is the lower index of the starting edge of that cube that is parallel to the edge under consideration, and suppose that the upper indexes corresponding to the two first faces are given by upperindex. CompleteUpperindices() computes the list of all possible upper index vectors for the given configuration of faces that begin with upperindex.*/

static list *CompleteUpperindices(int edgeindex, cube *const faces[], const vector *v, const int upperindex[]) {

	int  i, j, *vc = v->coord;
	const int degree = v->dim + 1;	
	list *veclist = NULL, *vlist = NULL, *product;
	vector *vec, *vec2;

	vec = NewVector(2, sizeof(int));
	((int *) vec->coord)[0] = upperindex[0];
	((int *) vec->coord)[1] = upperindex[1];
	veclist = InsertElement(vec, veclist);
	if (edgeindex == 0) {
		for (i = 2; i < degree - 1 && veclist; i++) {
			for (j = 0; j <= 1; j++) {					
				if (faces[vc[0]]->d[j][i] == faces[vc[i]]->d[upperindex[0]][1]) {				
					vec2 = NewVector(1, sizeof(int));
					((int *) vec2->coord)[0] = j;
					vlist = InsertElement(vec2, vlist);
				}
			}
			product = Product(veclist, vlist, INT);
			DeleteList(&veclist, DeleteVector);
			DeleteList(&vlist, DeleteVector);
			veclist = product;								
		}
	}
	else if (edgeindex == 1) {
		for (i = 2; i < degree - 1 && veclist; i++) {
			for (j = 0; j<= 1; j++) {			
				if (faces[vc[0]]->d[j][i] == faces[vc[i]]->d[upperindex[0]][0]) {				
					vec2 = NewVector(1, sizeof(int));
					((int *) vec2->coord)[0] = j;
					vlist = InsertElement(vec2, vlist);
				}
			}					
			product = Product(veclist, vlist, INT);
			DeleteList(&veclist, DeleteVector);
			DeleteList(&vlist, DeleteVector);
			veclist = product;								
		}
	}
	else {
		for (i = 2; i < edgeindex && veclist; i++) {
			for (j = 0; j<= 1; j++) {			
				if (faces[vc[0]]->d[j][i - 1] == faces[vc[i]]->d[upperindex[0]][0]) {				
					vec2 = NewVector(1, sizeof(int));
					((int *) vec2->coord)[0] = j;
					vlist = InsertElement(vec2, vlist);
				}
			}					
			product = Product(veclist, vlist, INT);
			DeleteList(&veclist, DeleteVector);
			DeleteList(&vlist, DeleteVector);
			veclist = product;								
		}		
		for (i = edgeindex; i < degree - 1 && veclist; i++) {
			for (j = 0; j<= 1; j++) {							
				if (faces[vc[0]]->d[j][i] == faces[vc[i]]->d[upperindex[0]][0]) {				
					vec2 = NewVector(1, sizeof(int));
					((int *) vec2->coord)[0] = j;
					vlist = InsertElement(vec2, vlist);
				}
			}						
			product = Product(veclist, vlist, INT);
			DeleteList(&veclist, DeleteVector);
			DeleteList(&vlist, DeleteVector);
			veclist = product;								
		}		
	}
	return veclist;
}


/*At a fixed edge, suppose that all cubes of dimension degree-1 (all containing the edge) have been constructed and that they have been put in the faces array. Suppose that the vector v contains the indexes of degree-1 faces that from the point of view of process IDs could be part of the boundary of a cube of dimension degree, and suppose that edgeindex is the lower index of the starting edge of that cube that is parallel to the given edge. Upperindices() computes the list of all possible upper index vectors for the given configuration of faces.*/

static list *Upperindices(int edgeindex, cube *const faces[], const vector *v) {

	int  upperindex[2], *vc = v->coord;		
	list *veclist, *upperindices = NULL;

	if (edgeindex == 0) {
		if (faces[vc[0]]->d[0][1] == faces[vc[1]]->d[0][1]) {
			upperindex[0] = 0;
			upperindex[1] = 0;
			veclist = CompleteUpperindices(edgeindex, faces, v, upperindex);
			ConcatLists(&upperindices, veclist);
			DeleteList(&veclist, NULL);
		}
		if (faces[vc[0]]->d[0][1] == faces[vc[1]]->d[1][1]) {
			upperindex[0] = 1;
			upperindex[1] = 0;
			veclist = CompleteUpperindices(edgeindex, faces, v, upperindex);
			ConcatLists(&upperindices, veclist);
			DeleteList(&veclist, NULL);
		}
		if (faces[vc[0]]->d[1][1] == faces[vc[1]]->d[0][1]) {
			upperindex[0] = 0;
			upperindex[1] = 1;
			veclist = CompleteUpperindices(edgeindex, faces, v, upperindex);
			ConcatLists(&upperindices, veclist);
			DeleteList(&veclist, NULL);
		}
		if (faces[vc[0]]->d[1][1] == faces[vc[1]]->d[1][1]) {
			upperindex[0] = 1;
			upperindex[1] = 1;
			veclist = CompleteUpperindices(edgeindex, faces, v, upperindex);
			ConcatLists(&upperindices, veclist);
			DeleteList(&veclist, NULL);
		}
	}
	else if (edgeindex == 1) {
		if (faces[vc[0]]->d[0][1] == faces[vc[1]]->d[0][0]) {
			upperindex[0] = 0;
			upperindex[1] = 0;
			veclist = CompleteUpperindices(edgeindex, faces, v, upperindex);
			ConcatLists(&upperindices, veclist);
			DeleteList(&veclist, NULL);
		}
		if (faces[vc[0]]->d[0][1] == faces[vc[1]]->d[1][0]) {
			upperindex[0] = 1;
			upperindex[1] = 0;
			veclist = CompleteUpperindices(edgeindex, faces, v, upperindex);
			ConcatLists(&upperindices, veclist);
			DeleteList(&veclist, NULL);
		}
		if (faces[vc[0]]->d[1][1] == faces[vc[1]]->d[0][0]) {
			upperindex[0] = 0;
			upperindex[1] = 1;
			veclist = CompleteUpperindices(edgeindex, faces, v, upperindex);
			ConcatLists(&upperindices, veclist);
			DeleteList(&veclist, NULL);
		}
		if (faces[vc[0]]->d[1][1] == faces[vc[1]]->d[1][0]) {
			upperindex[0] = 1;
			upperindex[1] = 1;
			veclist = CompleteUpperindices(edgeindex, faces, v, upperindex);
			ConcatLists(&upperindices, veclist);
			DeleteList(&veclist, NULL);
		}
	}
	else {
		if (faces[vc[0]]->d[0][0] == faces[vc[1]]->d[0][0]) {
			upperindex[0] = 0;
			upperindex[1] = 0;
			veclist = CompleteUpperindices(edgeindex, faces, v, upperindex);
			ConcatLists(&upperindices, veclist);
			DeleteList(&veclist, NULL);
		}
		if (faces[vc[0]]->d[0][0] == faces[vc[1]]->d[1][0]) {
			upperindex[0] = 1;
			upperindex[1] = 0;
			veclist = CompleteUpperindices(edgeindex, faces, v, upperindex);
			ConcatLists(&upperindices, veclist);
			DeleteList(&veclist, NULL);
		}
		if (faces[vc[0]]->d[1][0] == faces[vc[1]]->d[0][0]) {
			upperindex[0] = 0;
			upperindex[1] = 1;
			veclist = CompleteUpperindices(edgeindex, faces, v, upperindex);
			ConcatLists(&upperindices, veclist);
			DeleteList(&veclist, NULL);
		}
		if (faces[vc[0]]->d[1][0] == faces[vc[1]]->d[1][0]) {
			upperindex[0] = 1;
			upperindex[1] = 1;
			veclist = CompleteUpperindices(edgeindex, faces, v, upperindex);
			ConcatLists(&upperindices, veclist);
			DeleteList(&veclist, NULL);
		}		
	}		
	return upperindices;
}


/*Returns 1 if cube satisfies the boundary identities and 0 else*/

static int BdIdsOK(const cube *pc) {

	int ok = 1, i, j, k, l;
	
	for (i = 0; i < pc->degree - 1 && ok; i++) {
		for (j = i + 1; j < pc->degree && ok; j++) {
			for (k = 0; k < 2 && ok; k++) {
				for (l = 0; l < 2 && ok ; l++) {
					if (pc->d[k][i]->d[l][j-1] != pc->d[l][j]->d[k][i]) 
						ok = 0;															
				} 				
			}				
		}	
	}
	return ok;
} 


/*Completes HDA by introducing cubes of given degree at given 1-cube, returns the number of cubes added*/

static int FillHDCubes(cube *const edge, list *cubelist[], int degree, int facecount) {

	int  newcount = 0, i, j, *upperindex, edgeindex, *vc;
	const int edgepid = ((label **) edge->lab->coord)[0]->num;
	vector *v, *vec, *cubevec, *upvec, *facevec = NewVector(facecount, sizeof(cube *));	
	cube **faces = facevec->coord, *top[2], *cub, **cubevecc;
	list *clist, *toplist[2] = {NULL, NULL}, *tlist[2], *slist, *veclist = NULL, *config, *cubeveclist, *cveclist, *product, *intersection, *upperindices, *ulist, *others = NULL;
	
	if (degree > 2) {
/*determine faces containing edge and their configurations with compatible pids*/
		clist = cubelist[degree - 1]->prev;
		for (i = 0; i < facecount; i++) {
			faces[i] = (cube *) clist->data;
			vec = NewVector(1, sizeof(int));
			((int *) vec->coord)[0] = i;
			veclist = InsertElement(vec, veclist);
			clist = clist->prev;
		}
		config = Config(edge, degree, veclist, faces);
		DeleteList(&veclist, DeleteVector);
/*for each configuration of faces with compatible pids*/
		if (config) {
			veclist = config;
			do {
				v = (vector *) veclist->data;
				vc = v->coord;
/*determine edgeindex*/
				for (i = 0; i < degree - 1; i++) {
					if (((label **) ((cube **) faces[vc[0]]->edges->coord)[i]->lab->coord)[0]->num == edgepid) {	
						edgeindex = i;
						break;
					}
				}	
				if (edgeindex > 0 || ((label **) ((cube **) faces[vc[1]]->edges->coord)[0]->lab->coord)[0]->num != edgepid) 
					edgeindex += 1;	
				upperindices = Upperindices(edgeindex, faces, v);
/*for each vector of compatible upper indexes*/				
				if (upperindices) {
					ulist = upperindices; 
					do {
						upvec = (vector *) upperindices->data;
						upperindex = upvec->coord;
/*determine all possible top and bottom faces (wrt edge)*/ 
						if (edgeindex > 0) {
							for (i = 0; i < 2; i++) {
								MergeLists(&toplist[i], faces[vc[0]]->d[i][edgeindex - 1]->s[upperindex[0]][0], NULL);
								for (j = 1; j < edgeindex && toplist[i]; j++) {
									intersection = Intersection(faces[vc[j]]->d[i][edgeindex - 1]->s[upperindex[j]][j], toplist[i], NULL);
									DeleteList(&toplist[i], NULL);
									toplist[i] = intersection;											
								}
								for (j = edgeindex; j < degree - 1 && toplist[i]; j++) {
									intersection = Intersection(faces[vc[j]]->d[i][edgeindex]->s[upperindex[j]][j], toplist[i], NULL);
									DeleteList(&toplist[i], NULL);
									toplist[i] = intersection;									
								}
								if (!toplist[i]) 
									break;
							}								
						}
						else {
							for (i = 0; i < 2; i++) {
								MergeLists(&toplist[i], faces[vc[0]]->d[i][0]->s[upperindex[0]][0], NULL);
								for (j = 1; j < degree - 1 && toplist[i]; j++) {
									intersection = Intersection(faces[vc[j]]->d[i][0]->s[upperindex[j]][j], toplist[i], NULL);
									DeleteList(&toplist[i], NULL);
									toplist[i] = intersection;									
								}
								if (!toplist[i]) 
									break;
							}								
						}
/*for all possible top and bottom faces*/ 
						if (toplist[0] && toplist[1]) {
							tlist[0] = toplist[0];
							do {
								top[0] = (cube *) tlist[0]->data;
								tlist[1] = toplist[1];
								do {
									top[1] = (cube *) tlist[1]->data;
/*determine all vectors of possible opposite faces of the ones in the current configuration*/
									if (edgeindex > 0) {
										slist = Intersection(top[0]->d[1 - upperindex[0]][0]->s[0][edgeindex - 1], top[1]->d[1 - upperindex[0]][0]->s[1][edgeindex - 1], NULL);										
										cubeveclist = AsVectorList(slist);
										DeleteList(&slist, NULL);
										for (j = 1; j < edgeindex && cubeveclist; j++) {
											slist = Intersection(top[0]->d[1 - upperindex[j]][j]->s[0][edgeindex - 1], top[1]->d[1 - upperindex[j]][j]->s[1][edgeindex - 1], NULL);											
											cveclist = AsVectorList(slist);
											DeleteList(&slist, NULL);
											product = Product(cubeveclist, cveclist, PTR);
											DeleteList(&cubeveclist, DeleteVector);
											DeleteList(&cveclist, DeleteVector);
											cubeveclist = product;						
										}										
										for (j = edgeindex; j< degree - 1 && cubeveclist; j++) {
											slist = Intersection(top[0]->d[1 - upperindex[j]][j]->s[0][edgeindex], top[1]->d[1 - upperindex[j]][j]->s[1][edgeindex], NULL);													
											cveclist = AsVectorList(slist);
											DeleteList(&slist, NULL);
											product = Product(cubeveclist, cveclist, PTR);
											DeleteList(&cubeveclist, DeleteVector);
											DeleteList(&cveclist, DeleteVector);
											cubeveclist = product;	
										}										
									}
									else {
										slist = Intersection(top[0]->d[1 - upperindex[0]][0]->s[0][0], top[1]->d[1 - upperindex[0]][0]->s[1][0], NULL);	
										cubeveclist = AsVectorList(slist);
										DeleteList(&slist, NULL);
										for (j = 1; j < degree - 1 && cubeveclist; j++) {
											slist = Intersection(top[0]->d[1 - upperindex[j]][j]->s[0][0], top[1]->d[1 - upperindex[j]][j]->s[1][0], NULL);			
											cveclist = AsVectorList(slist);
											DeleteList(&slist, NULL);
											product = Product(cubeveclist, cveclist, PTR);
											DeleteList(&cubeveclist, DeleteVector);
											DeleteList(&cveclist, DeleteVector);
											cubeveclist = product;	
										}										
									}
/*for each vector of possible opposite faces of the ones in the current configuration*/
									if (cubeveclist) {
										cveclist = cubeveclist;
										do {
											cubevec = (vector *) cveclist->data;
											cubevecc = cubevec->coord;
/*create a new cube*/
											cub = NewCube(degree);
											cub->d[0][edgeindex] = top[0];			
											cub->d[1][edgeindex] = top[1];
											others = Intersection(top[0]->s[0][edgeindex], top[1]->s[1][edgeindex], NULL);
											for (i = 0; i < edgeindex; i++) {
												cub->d[upperindex[i]][i] = faces[vc[i]];
												intersection = Intersection(others, faces[vc[i]]->s[upperindex[i]][i], NULL);
												DeleteList(&others, NULL);
												others = intersection;
												cub->d[1 - upperindex[i]][i] = cubevecc[i];
												intersection = Intersection(others, cubevecc[i]->s[1 - upperindex[i]][i], NULL);
												DeleteList(&others, NULL);
												others = intersection;
											}
											for (i = edgeindex; i < degree - 1; i++) {
												cub->d[upperindex[i]][i + 1] = faces[vc[i]];
												intersection = Intersection(others, faces[vc[i]]->s[upperindex[i]][i + 1], NULL);
												DeleteList(&others, NULL);
												others = intersection;
												cub->d[1 - upperindex[i]][i + 1] = cubevecc[i];
												intersection = Intersection(others, cubevecc[i]->s[1 - upperindex[i]][i + 1], NULL);
												DeleteList(&others, NULL);
												others = intersection;
											}
/*insert the cube if the boundary conditions are satisfied and there is no other cube with the same boundary*/
											if (BdIdsOK(cub) && others == NULL) {												
												for (i = 0; i < degree - 1; i++) 
													((cube **) cub->edges->coord)[i] = ((cube **) cub->d[0][degree - 1]->edges->coord)[i];
												((cube **) cub->edges->coord)[degree - 1] = ((cube **) cub->d[0][degree - 3]->edges->coord)[degree - 2];
												top[0]->s[0][edgeindex] = InsertElement(cub, top[0]->s[0][edgeindex]);
												top[1]->s[1][edgeindex] = InsertElement(cub, top[1]->s[1][edgeindex]);
												for (i = 0; i < edgeindex; i++) {
													faces[vc[i]]->s[upperindex[i]][i] = InsertElement(cub, faces[vc[i]]->s[upperindex[i]][i]);
													cubevecc[i]->s[1 - upperindex[i]][i] = InsertElement(cub, cubevecc[i]->s[1 - upperindex[i]][i]);
												}
												for (i = edgeindex; i < degree - 1; i++) {
													faces[vc[i]]->s[upperindex[i]][i + 1] = InsertElement(cub, faces[vc[i]]->s[upperindex[i]][i + 1]);
													cubevecc[i]->s[1 - upperindex[i]][i + 1] = InsertElement(cub, cubevecc[i]->s[1 - upperindex[i]][i + 1]);
												}																																
												cubelist[degree] = InsertElement(cub, cubelist[degree]);
												cub->cl = cubelist[degree]->prev;																				
												newcount++;
											}
											else {
												DeleteCube(cub);
												DeleteList(&others, NULL);												
											}
											cveclist = cveclist->next;
										} while (cveclist != cubeveclist);
										DeleteList(&cubeveclist, DeleteVector);
									}
									tlist[1] = tlist[1]->next;
								} while (tlist[1] != toplist[1]);
								tlist[0] = tlist[0]->next;
							} while (tlist[0] != toplist[0]);
						}
						DeleteList(&toplist[0], NULL);
						DeleteList(&toplist[1], NULL);
						ulist = ulist->next;
					} while (ulist != upperindices);
					DeleteList(&upperindices, DeleteVector);
				}						
				veclist = veclist->next;			
			} while (veclist != config);
			DeleteList(&config, DeleteVector);
		}		
	}
	DeleteVector(facevec);	
	return newcount;
} 


/*Completes HDA cubelist at given edge, returns the dimension of the highest cube added, returns -1 if no cube is added*/ 

int FillCubes(cube *edge, list *cubelist[]) {

	int  facecount, dim = -1;

	facecount = FillSquares(edge, cubelist);
	if (facecount > 0)
		dim = 1;
	while (facecount != 0) 
		facecount = FillHDCubes(edge, cubelist, ++dim  + 1, facecount);	
	return dim;	
}
