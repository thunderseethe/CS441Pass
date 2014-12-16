%error-verbose
%{
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "calc3.h"
#include "symbol_table.h"

/* prototypes */
nodeType *opr(int oper, int nops, ...);
nodeType *id(char *name); /* changed JWJ */
nodeType *cid(char *name, vartypeEnum type, flagsEnum flags); /* rgc: creating id with a type */
nodeType *con(long value);
nodeType *fpcon(double value);
void freeNode(nodeType *p);
dataType ex(nodeType *p);
int yylex(void);
vartypeEnum type; //Global to handle list of variable declarations, couldn't get inherited attributes to work
flagsEnum flags;

void yyerror(char *s);
/* int sym[26]; JWJ:  removed to replace with real Symbol Table                  */
%}

%union {
    long lValue;                 /* integer value */
	double fValue;               /* floating point value */
    char *sName;                /* name of a variable - changed JWJ */
    nodeType *nPtr;             /* node pointer */
};

%token <lValue> NUMBER
%token <fValue> FLOAT
%token <sName> VARIABLE /* changed JWJ */
%token WHILE IF PRINT B_BEGIN B_END T_INT T_DOUBLE T_LONG CONST
%token DO FOR UNTIL
%nonassoc IFX
%nonassoc ELSE

%left GE LE EQ NE '>' '<'
%left '+' '-'
%left '*' '/'
%nonassoc UMINUS

%type <nPtr> stmt stmt_list expr term factor typedecl_expr assgn_expr for_loop variable_list var_opr

%%

program:
        function                { exit(0); }
        ;

function:
          function stmt         { ex($2); freeNode($2); }
        | /* NULL */
        ;

stmt:
          ';'                            { $$ = opr(';', 2, NULL, NULL); }
        | expr ';'                       { $$ = $1; }
        | assgn_expr ';'                 { $$ = $1; }
        | PRINT expr ';'                 { $$ = opr(PRINT, 1, $2); }
        | WHILE '(' expr ')' stmt        { $$ = opr(WHILE, 2, $3, $5); }
        | DO stmt WHILE '(' expr ')'     { $$ = opr(DO, 2, $5, $2); } /*JWJ: TODO implement*/
        | DO stmt UNTIL '(' expr ')'     { $$ = opr(UNTIL, 2, $5, $2); }
        | for_loop                       { $$ = $1; } /* JWJ: TODO: Implement for loops */
        | IF '(' expr ')' stmt %prec IFX { $$ = opr(IF, 2, $3, $5); }
        | IF '(' expr ')' stmt ELSE stmt { $$ = opr(IF, 3, $3, $5, $7); }
        | '{' stmt_list '}'              { $$ = opr(B_BEGIN, 1, $2); }
        | B_BEGIN stmt_list B_END        { $$ = opr(B_BEGIN, 1, $2); }
        ;

stmt_list:
          stmt                  { $$ = $1; }
        | stmt_list stmt        { $$ = opr(';', 2, $1, $2); }
        ;

for_loop:
          FOR '(' assgn_expr ';' expr ';' assgn_expr ')' stmt       { $$ = opr(FOR, 4, $3, $5, $7, $9); }
        | FOR '(' assgn_expr ';' expr ';' assgn_expr ';' ')' stmt   { $$ = opr(FOR, 4, $3, $5, $7, $10); }
        ;

assgn_expr: 
          typedecl_expr                 { $$ = $1; }
        | VARIABLE '=' expr             { $$ = opr('=', 2, id($1), $3); }
        ;

expr:
          expr '+' term { $$ = opr('+', 2, $1, $3); }
        | expr '-' term { $$ = opr('-', 2, $1, $3); }
        | term          { $$ = $1; }
        ;


term:
          term '*' factor { $$ = opr('*', 2, $1, $3); }
        | term '/' factor { $$ = opr('/', 2, $1, $3); }
        | term '%' factor { $$ = opr('%', 2, $1, $3); }
        | factor          { $$ = $1; }
        ;

factor:
          NUMBER                { $$ = con($1); }
        | FLOAT                 { $$ = fpcon($1); }
        | VARIABLE              { $$ = id($1); }
        | '(' T_INT ')' expr    { $$ = opr(T_INT, 1, $4); }
        | '(' T_DOUBLE ')' expr { $$ = opr(T_DOUBLE, 1, $4); }
        | '-' expr %prec UMINUS { $$ = opr(UMINUS, 1, $2); }
        | expr '^' expr         { $$ = opr('^', 2, $1, $3); }
        | expr '<' expr         { $$ = opr('<', 2, $1, $3); }
        | expr '>' expr         { $$ = opr('>', 2, $1, $3); }
        | expr GE expr          { $$ = opr(GE, 2, $1, $3); }
        | expr LE expr          { $$ = opr(LE, 2, $1, $3); }
        | expr NE expr          { $$ = opr(NE, 2, $1, $3); }
        | expr EQ expr          { $$ = opr(EQ, 2, $1, $3); }
        | '(' expr ')'          { $$ = $2; }
        ;

typedecl_expr:
		  type { flags = FLAG_VAR; } variable_list               { $$ = $3; }
        | CONST type { flags = FLAG_CONST_UNDEF; } variable_list { $$ = $4; }
        ;
type:
      T_INT { type = TYPE_INT; }
    | T_DOUBLE { type = TYPE_DOUBLE; }
    | T_LONG { type = TYPE_LONG; }

variable_list:
          var_opr ',' variable_list { $$ = $3; ex($1); }
        | var_opr                   { $$ = $1; }
        ;

var_opr:
          VARIABLE          { $$ = cid($1, type, flags); }
        | VARIABLE '=' expr { $$ = opr('=', 2, cid($1, type, flags), $3); }
        ;

%%

#define SIZEOF_NODETYPE ((char *)&p->con - (char *)p)

nodeType *con(long value) {
    nodeType *p;
    size_t nodeSize;

    /* allocate node */
    nodeSize = SIZEOF_NODETYPE + sizeof(conNodeType);
    if ((p = malloc(nodeSize)) == NULL)
        yyerror("out of memory");

    /* copy information */
    if(value >= INT_MAX){
        p->type = typeCon;
        p->con.value.type = TYPE_LONG;
        p->con.value._long = value;
    }else {
        p->type = typeCon;
	    p->con.value.type = TYPE_INT;
        p->con.value._int = (int)value;
    }

    return p;
}

/* rgc: handle floating point constants */
nodeType *fpcon(double value) {
    nodeType *p;
    size_t nodeSize;

    /* allocate node */
    nodeSize = SIZEOF_NODETYPE + sizeof(conNodeType);
    if ((p = malloc(nodeSize)) == NULL)
        yyerror("out of memory");

    /* copy information */
    p->type = typeCon;
	p->con.value.type = TYPE_DOUBLE;
    p->con.value._double = value;

    return p;
}

nodeType *id(char *name) {  /* changed JWJ */
    nodeType *p;
    size_t nodeSize;

    /* allocate node */
    nodeSize = SIZEOF_NODETYPE + sizeof(idNodeType);
    if ((p = malloc(nodeSize)) == NULL)
        yyerror("out of memory");

    /* copy information */
    p->type = typeId;
	p->id.type = TYPE_UNDEF; /* rgc: we only give the node a type to help with initialization */
    p->id.s = name; /* changed JWJ */
	
    return p;
}

nodeType *cid(char *name, vartypeEnum type, flagsEnum flags) {  /* changed JWJ */
    nodeType *p;
    size_t nodeSize;
    
    /* allocate node */
    nodeSize = SIZEOF_NODETYPE + sizeof(idNodeType);
    if ((p = malloc(nodeSize)) == NULL)
        yyerror("out of memory");

    /* copy information */
    p->type = typeIdInit;
	p->id.type = type;
    p->id.flag = flags;
    p->id.s = name; /* changed JWJ */

    return p;
}

nodeType *opr(int oper, int nops, ...) {
    va_list ap;
    nodeType *p;
    size_t nodeSize;
    int i;

    /* allocate node */
    nodeSize = SIZEOF_NODETYPE + sizeof(oprNodeType) +
        (nops - 1) * sizeof(nodeType*);
    if ((p = malloc(nodeSize)) == NULL)
        yyerror("out of memory");

    /* copy information */
    p->type = typeOpr;
    p->opr.oper = oper;
    p->opr.nops = nops;
    va_start(ap, nops);
    for (i = 0; i < nops; i++)
        p->opr.op[i] = va_arg(ap, nodeType*);
    va_end(ap);
    return p;
}

void freeNode(nodeType *p) {
    int i;

    if (!p) return;
    if (p->type == typeOpr) {
        for (i = 0; i < p->opr.nops; i++)
            freeNode(p->opr.op[i]);
    }
    free (p);
}

void yyerror(char *s) {
    fprintf(stdout, "%s\n", s);
}

int main(void) {
    pushSymbolTable();
    yyparse();
    return 0;
}
