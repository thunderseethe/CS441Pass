#ifndef __CALC3_H
#define __CALC3_H

#define INT_MAX 2147483647

// rgc: node types
typedef enum { 
	typeCon, // constant (int or FP)
	typeId, // identifier
	typeOpr, // operator
	typeIdInit //rgc: special case: identifier being initialized
} nodeEnum; 

// rgc: id types.
typedef enum { 
	TYPE_INT,
	TYPE_LONG,
	TYPE_DOUBLE,
	TYPE_CONST,
	TYPE_FUNCTION, //rgc: for function support later on
	TYPE_UNDEF	 	//rgc: used by the parser for an identifier not being initialized
} vartypeEnum; 

typedef enum {
	FLAG_CONST_UNDEF,
	FLAG_CONST_DEF,
	FLAG_VAR
} flagsEnum;

// rgc: struct for numerical data, with built-in type tracking
typedef struct {
	vartypeEnum type;			// rgc: id type (double or int)
	flagsEnum flag;
	union {
		int _int;					// data for int
		double _double;				// data for double
		long _long;					// data for long
	};
} dataType;

/* constants */
typedef struct {
    dataType value;                  /* value of constant, FP or int */
} conNodeType;

/* identifiers */
typedef struct {
    char *s;                    /* variable name - changed JWJ */
	vartypeEnum type;			//rgc: id type (double or int)
	flagsEnum flag;
} idNodeType;

/* operators */
typedef struct {
    int oper;                   /* operator */
    int nops;                   /* number of operands */
    struct nodeTypeTag *op[1];  /* operands (expandable) */
} oprNodeType;

typedef struct nodeTypeTag {
    nodeEnum type;              /* type of node */

    /* union must be last entry in nodeType */
    /* because operNodeType may dynamically increase */
    union {
        conNodeType con;        /* constants */
        idNodeType id;          /* identifiers */
        oprNodeType opr;        /* operators */
    };
} nodeType;

/* removed extern int sym[26]; JWJ */
#endif
