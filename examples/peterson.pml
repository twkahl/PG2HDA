/*Peterson's mutual exclusion algorithm (with artificial control structure for illustration purposes)*/


/*The Promela file that describes a system must be organized in three sections (for the precise syntax see the Bison grammar at the end of this file).*/


/*Section 1: One or more variable declarations*/

int b[2] = 0, t = 0;  /*Variables must be global and of type int, and they must be initialized.*/
int c[3] = {0, 0, 1}; 


/*Section 2: Optional definitions*/

#define set_flag b[_pid%2] = 1		/*Transitions should be given names so that they will be accepted by SageMath as generators of an exterior algebra.*/
#define give_priority t = 1 -_pid	/*Transitions may be assignments or atomic sequences of assignments; an atomic sequence may have a guard condition. A skip action is seen as an assignment (of the form x = x).*/
#define enter_crit atomic {(b[1-_pid%2] == 0 || t == _pid) -> c[_pid]++}	/*Arrows may not be replaced by semicolons, but the increment and decrement operators and the _pid variable may be used in the customary way.*/ 
#define leave_crit atomic {c[_pid]--; b[_pid%2] = 0}	
#define p (c[0] >= 1 && c[1] >= 1) /*Definitions of names for boolean expressions may be included but are ignored.*/


/*Section 3: One or more proctype declarations*/

active [2] proctype peterson_with_artificial_control_structure() { /*All processes must be active.*/
end: 			/*The label 'end' may be used to mark the final location of a process.*/
	do			/*A process may contain repetition structures (do-od, without 'break'), selection structures (if-fi, without 'else'), and jumps (goto).*/
 	::	set_flag;
		if 	
		::	give_priority;
			goto next;				 
		next: 
			enter_crit	
		::	atomic {(0 == 1) -> skip};
			leave_crit;
			do
			:: enter_crit
			od	 	 	  	  	 
		fi; 
		assert(c[0] + c[1] <= 1); /*Assert statements may be included but are ignored.*/
		leave_crit 
	::	atomic {(0 != 0) -> skip}; 
		enter_crit;
		goto out     		      
	od;	
out: 
	leave_crit		 			
}

active proctype dummy() {
	do
	::	atomic {(0 != 0) -> skip}	
	od 
}


/*All text after the keywords 'never' and 'ltl' is ignored.*/

never  {    /* <>p */
T0_init:
	do
	:: atomic { ((p)) -> assert(!((p))) }
	:: (1) -> goto T0_init
	od;
accept_all:
	skip
}

ltl always_not_p { []!p };


/*************************************************************************************************************************	

    Bison grammar

    1 pml: vardecls optionaldefs procs

    2 vardecls: vardecl
    3         | vardecls vardecl

    4 vardecl: INTEGER vardefs ';'

    5 vardefs: vardef
    6        | vardefs ',' vardef

    7 vardef: STRING '=' NAT_NUMBER
    8       | STRING '=' '-' NAT_NUMBER
    9       | STRING '[' NAT_NUMBER ']' '=' NAT_NUMBER
   10       | STRING '[' NAT_NUMBER ']' '=' '-' NAT_NUMBER
   11       | STRING '[' NAT_NUMBER ']' '=' '{' values '}'

   12 values: NAT_NUMBER
   13       | '-' NAT_NUMBER
   14       | values ',' NAT_NUMBER
   15       | values ',' '-' NAT_NUMBER

   16 optionaldefs: %empty
   17             | defs

   18 defs: def
   19     | defs def

   20 def: DEF STRING assignment
   21    | DEF STRING ATOMIC '{' sequence '}'
   22    | DEF STRING ATOMIC '{' condition ARROW sequence '}'
   23    | DEF STRING boolexp

   24 assignment: varid '=' ariexp
   25           | varid INCREMENT
   26           | varid DECREMENT
   27           | SKIP

   28 varid: STRING
   29      | STRING '[' ariexp ']'

   30 ariexp: ariexp '+' ariexp
   31       | ariexp '-' ariexp
   32       | ariexp '*' ariexp
   33       | ariexp '/' ariexp
   34       | ariexp '%' ariexp
   35       | '(' ariexp ')'
   36       | '-' ariexp
   37       | varid
   38       | NAT_NUMBER
   39       | PID

   40 sequence: assignment
   41         | sequence ';' assignment

   42 condition: boolexp

   43 boolexp: ariexp IS_EQUAL ariexp
   44        | ariexp IS_L ariexp
   45        | ariexp IS_LEQ ariexp
   46        | ariexp IS_G ariexp
   47        | ariexp IS_GEQ ariexp
   48        | ariexp NOT_EQUAL ariexp
   49        | boolexp LOGICAL_AND boolexp
   50        | boolexp LOGICAL_OR boolexp
   51        | '(' boolexp ')'
   52        | LOGICAL_NEG boolexp

   53 procs: procdef '(' ')' '{' stmnts endproc
   54      | procs procdef '(' ')' '{' stmnts endproc

   55 procdef: ACTIVE PROCTYPE STRING
   56        | ACTIVE '[' NAT_NUMBER ']' PROCTYPE STRING

   57 stmnts: stmnt
   58       | stmnts ';' stmnt

   59 stmnt: transition
   60      | locmarker transition
   61      | doopen options doclose
   62      | locmarker doopen options doclose
   63      | ifopen options ifclose
   64      | locmarker ifopen options ifclose
   65      | GOTO loclabel
   66      | ASSERT '(' boolexp ')'
   67      | loclabel ':' ASSERT '(' boolexp ')'

   68 transition: assignment
   69           | ATOMIC '{' sequence '}'
   70           | ATOMIC '{' condition ARROW sequence '}'
   71           | STRING

   72 locmarker: loclabel ':'

   73 loclabel: STRING

   74 doopen: DO

   75 options: option
   76        | options option

   77 option: doublecolon transition
   78       | doublecolon transition ';' stmnts
   79       | doublecolon ifopen options ifclose
   80       | doublecolon ifopen options ifclose ';' stmnts

   81 doublecolon: DOUBLECOLON

   82 ifopen: IF

   83 ifclose: FI

   84 doclose: OD

   85 endproc: '}'
   
************************************************************************   

	Tokens

	ACTIVE			"active"
	ARROW			"->"
	ASSERT			"assert"
	ATOMIC			"atomic"
	DECREMENT		"--"
	DEF				"#define"
	DO				"do"
	DOUBLECOLON		"::"  
	FI				"fi"
	GOTO			"goto"
	IF				"if"
	INCREMENT		"++" 
	INTEGER			"int"                         	
	IS_EQUAL    	"=="                            	
	IS_G			">"       
	IS_GEQ			">=" 
	IS_L			"<"
	IS_LEQ			"<=" 
	LOGICAL_AND		"&&"
	LOGICAL_NEG 	"!"
	LOGICAL_OR		"||"
	NAT_NUMBER		natural number
	NOT_EQUAL		"!=" 
	OD				"od"                           	
	PID				"_pid"                           	
	PROCTYPE		"proctype"                      	
	SKIP			"skip"                           	
	STRING			string starting with a letter and consisting otherwise of letters, digits, and underscores
	
**************************************************************************************************************************/   
