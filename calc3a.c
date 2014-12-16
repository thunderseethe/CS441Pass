#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "symbol_table.h"
#include "calc3.h"
#include "y.tab.h"

extern int yylineno; /* added JWJ */

dataType ex(nodeType *p) {
	dataType rvTrue; //rgc: 'true' return variable
	rvTrue._int=1;
	rvTrue.type = TYPE_INT;
	
	dataType rvFalse; //rgc: 'false' return variable
	rvFalse._int=0;
	rvFalse.type = TYPE_INT;
	
    if (!p) return rvFalse;
	
    switch(p->type) {
		case typeCon:	
			return p->con.value;
		case typeId: {
			/* changed JWJ */
			struct symbol_entry *entry = getSymbolEntry(p->id.s);
			if (!entry) {
				fprintf(stderr, "Undeclared variable %s at line %d\n", p->id.s,yylineno);
				exit(1);
			}
			return entry->value;
		}
		case typeIdInit: {
			struct symbol_entry *entry = getSymbolEntry(p->id.s);
			if (!entry || entry->blk_level != getCurrentLevel()) {
				entry = calloc(1, sizeof(struct symbol_entry));
				entry->name = p->id.s;
				entry->value.type = p->id.type; // rgc: new LHS var inherits type from idnode type
				entry->value.flag = p->id.flag;
				addSymbol(entry, yylineno);
				free(entry);

				/* need to look it up again since addSymbol makes a copy */
				entry = getSymbolEntry(p->id.s);
			} else {
				fprintf(stderr, "Warning: redeclaring variable %s in the same scope, re-using instance\n", p->id.s);
			}
			return rvTrue; //rgc: just initializing here, nothing to see, move along
		}
		case typeOpr: {
			switch(p->opr.oper) {
				case B_BEGIN: {
					pushSymbolTable();
					ex(p->opr.op[0]);
					popSymbolTable();
					return rvTrue;
				}
				case WHILE:	
					while(ex(p->opr.op[0])._int) ex(p->opr.op[1]); return rvTrue;
				case DO:
					do { ex(p->opr.op[1]); } while (ex(p->opr.op[0])._int); return rvTrue;
				case UNTIL:
					do { ex(p->opr.op[1]); } while (!ex(p->opr.op[0])._int); return rvTrue;
				case IF:	
					if (ex(p->opr.op[0])._int)
						ex(p->opr.op[1]);
					else if (p->opr.nops > 2)
						ex(p->opr.op[2]);
					return rvTrue;
				case FOR:
					for( ex(p->opr.op[0]); ex(p->opr.op[1])._int; ex(p->opr.op[2])) { ex(p->opr.op[3]); } return rvTrue;
				case LIST_INIT: {
					const char *name = p->opr.op[0]->id.s;
					int length = p->opr.op[1]->con.value._int;
					struct symbol_entry *entry = getSymbolEntry(name);
					if(entry){
						fprintf(stderr, "Warning: redeclaring variable %s in the same scope\n", name);
					}
					entry = calloc(1, sizeof(struct symbol_entry));
					entry->name = name;
					entry->value.type = TYPE_LIST; // rgc: new LHS var inherits type from idnode type
					entry->value.flag = FLAG_VAR;
					entry->value._int = length;
					addSymbol(entry, 0);
					free(entry);
					for(int i = 0; i < length; i++){
						int l = strlen(name) + 21;
						entry = calloc(1, sizeof(struct symbol_entry));
						entry->name = malloc(l);
						sprintf(entry->name, "%d%s", i, name);
						entry->value.type = TYPE_UNDEF;
						entry->value.flag = FLAG_VAR;
						addSymbol(entry, 0);
						free(entry);
					}
					return rvTrue;
				}
				case LIST_SET: {
					dataType rhs = ex(p->opr.op[2]);
					char* name = p->opr.op[0]->id.s;
					int index = p->opr.op[1]->con.value._int;
					char* id = malloc(strlen(name)+21);
					sprintf(id, "%d%s", index, name);
					struct symbol_entry *entry = getSymbolEntry(id);
					entry->value = rhs;
					return entry->value;
				}
				case LIST_ACCESS: {
					char*name = p->opr.op[0]->id.s;
					int index = p->opr.op[1]->con.value._int;
					char* id = malloc(strlen(name)+21);
					sprintf(id, "%d%s", index, name);
					struct symbol_entry *entry = getSymbolEntry(id);
					return entry->value;
				}
				case PRINT:	{
					dataType pv = ex(p->opr.op[0]);
					if (pv.type == TYPE_INT) {
						printf("%d\n", pv._int); 
						return rvTrue;
					} else if (pv.type == TYPE_DOUBLE) {
						printf("%f\n", pv._double);
						return rvTrue;
					} else if (pv.type == TYPE_LONG) {
						printf("%ld\n", pv._long);
						return rvTrue;
					} else {
						fprintf(stderr, "Error at line %d: printing unrecognized type\n", yylineno);
						exit(1);
					}
				}
				case ';':	
					ex(p->opr.op[0]); 
					return ex(p->opr.op[1]);
				case '=':
					/* changed JWJ */
					{
						/* rgc: check to make sure LHS is an identifier and not a constant */
						if ((p->opr.op[0]->type != typeId) && (p->opr.op[0]->type != typeIdInit)) {
							fprintf(stderr, "Syntax error at line %d: cannot perform assignment\n", yylineno);
							exit(1);
						}
						
						// rgc: create new var if inline declaration (int num=5)
						if (p->opr.op[0]->type == typeIdInit) ex(p->opr.op[0]);
						
						const char *name = p->opr.op[0]->id.s;
						/* look up an existing variable */
						struct symbol_entry *entry = getSymbolEntry(name);

						/* doesn't exist: add a new variable to the symbol table */
						/* rgc: actually, we should throw an error. now we need to define vars before we use them */
						if (!entry) {
							fprintf(stderr, "Undeclared variable %s at line %d\n", name, yylineno);
							exit(1);
						}
						
						if(entry->value.flag == FLAG_CONST_DEF){
							fprintf(stderr, "Variable '%s' is declared constant, cannot be modified at line %d\n", name, yylineno);
							exit(1);
						}
						
						// calculate RHS
						dataType rhs = ex(p->opr.op[1]);
						
						// rgc: determine need for type casting
						rhs.flag = entry->value.flag;
						if (entry->value.type == rhs.type) {
							// same type
							entry->value = rhs;
						} else if (entry->value.type == TYPE_INT) {
							// LHS int, RHS double
							entry->value._int = (int) (rhs.type == TYPE_LONG ? rhs._long : rhs._double);
						} else if (entry->value.type == TYPE_DOUBLE) {
							// LHS double, RHS int
							entry->value._double = (double) (rhs.type == TYPE_INT ? rhs._long : rhs._int);
						} else if (entry->value.type == TYPE_LONG){
							entry->value._long = (long) (rhs.type == TYPE_INT ? rhs._int : rhs._double);
						}
						if(entry->value.flag == FLAG_CONST_UNDEF){
							entry->value.flag = FLAG_CONST_DEF;
						}
						return entry->value;
					}
				case UMINUS: {
					dataType value = ex(p->opr.op[0]);
					if (value.type == TYPE_INT) {
						value._int *= -1;
					} else if (value.type == TYPE_DOUBLE) {
						value._double *= -1;
					} else if (value.type == TYPE_LONG) {
						value._long *= -1;
					}
					return value;
				}
				
				case T_INT: {
					dataType value = ex(p->opr.op[0]);
					if (value.type == TYPE_DOUBLE) {
						int tmp = (int) value._double;
						value.type = TYPE_INT;
						value._int = tmp;
					} else if (value.type == TYPE_LONG){
						int tmp = (int) value._long;
						value.type = TYPE_INT;
						value._int = tmp;
					}
					return value;
				}
				
				case T_DOUBLE: {
					dataType value = ex(p->opr.op[0]);
					if (value.type == TYPE_INT) {
						double tmp = (double) value._int;
						value.type = TYPE_DOUBLE;
						value._double = tmp;
					} else if (value.type == TYPE_LONG){
						double tmp = (double) value._long;
						value.type = TYPE_DOUBLE;
						value._double = tmp;
					}
					return value;
				}

				case T_LONG: {
					dataType value = ex(p->opr.op[0]);
					if(value.type == TYPE_INT) {
						long tmp = (long) value._int;
						value.type = TYPE_LONG;
						value._long = tmp;
					}
					if(value.type == TYPE_DOUBLE) {
						long tmp = (long) value._double;
						value.type = TYPE_LONG;
						value._long = tmp;
					}
					return value;
				}
				
				// rgc: in the case of all binary arithmetic operations, if both operands are int, result will be int. 
				// otherwise, intermediate calculations should be carried out with the full precision of a double.
				case '+':	{
					dataType value0 = ex(p->opr.op[0]);
					dataType value1 = ex(p->opr.op[1]);
					dataType rv;
					if ((value0.type == TYPE_INT) && (value1.type == TYPE_INT)) {
						rv.type = TYPE_INT;
						rv._int = value0._int + value1._int;
					} else if ((value0.type == TYPE_DOUBLE) || (value1.type == TYPE_DOUBLE)) {
						rv.type = TYPE_DOUBLE;
						rv._double = (value0.type==TYPE_INT ? (double)value0._int : (value0.type==TYPE_LONG ? (double)value0._long : value0._double)) + (value1.type==TYPE_INT ? (double)value1._int : (value1.type==TYPE_LONG? (double)value1._long : value1._double));
					} else {
						rv.type = TYPE_LONG;
						rv._long = (value0.type == TYPE_INT ? (long) value0._int : value0._long) + (value1.type==TYPE_INT ? (long) value1._int : value1._long);
					}
					return rv;
				}
				case '-':	{
					dataType value0 = ex(p->opr.op[0]);
					dataType value1 = ex(p->opr.op[1]);
					dataType rv;
					if ((value0.type == TYPE_INT) && (value1.type == TYPE_INT)) {
						rv.type = TYPE_INT;
						rv._int = value0._int - value1._int;
					} else if ((value0.type == TYPE_DOUBLE) || (value1.type == TYPE_DOUBLE)) {
						rv.type = TYPE_DOUBLE;
						rv._double = (value0.type==TYPE_INT ? (double)value0._int : (value0.type==TYPE_LONG ? (double)value0._long : value0._double)) - (value1.type==TYPE_INT ? (double)value1._int : (value1.type==TYPE_LONG? (double)value1._long : value1._double));
					} else {
						rv.type = TYPE_LONG;
						rv._long = (value0.type == TYPE_INT ? (long) value0._int : value0._long) - (value1.type==TYPE_INT ? (long) value1._int : value1._long);
					}
					return rv;
				}
				case '*':	{
					dataType value0 = ex(p->opr.op[0]);
					dataType value1 = ex(p->opr.op[1]);
					/*if(value0.type != value1.type){
						fprintf(stderr, "Cannot multiply incompatible types\n");
						exit(1);
					}*/
					dataType rv;
					if ((value0.type == TYPE_INT) && (value1.type == TYPE_INT)) {
						rv.type = TYPE_INT;
						rv._int = value0._int * value1._int;
					} else if ((value0.type == TYPE_DOUBLE) || (value1.type == TYPE_DOUBLE)) {
						rv.type = TYPE_DOUBLE;
						rv._double = (value0.type==TYPE_INT ? (double)value0._int : (value0.type==TYPE_LONG ? (double)value0._long : value0._double)) * (value1.type==TYPE_INT ? (double)value1._int : (value1.type==TYPE_LONG? (double)value1._long : value1._double));
					} else {
						rv.type = TYPE_LONG;
						rv._long = (value0.type == TYPE_INT ? (long) value0._int : value0._long) * (value1.type==TYPE_INT ? (long) value1._int : value1._long);
					}
					return rv;
				}
				case '/':	{
					dataType value0 = ex(p->opr.op[0]);
					dataType value1 = ex(p->opr.op[1]);
					/*if(value0.type != value1.type){
						fprintf(stderr, "Cannot divide incompatible types\n");
						exit(1);
					}*/
					dataType rv;
					if ((value0.type == TYPE_INT) && (value1.type == TYPE_INT)) {
						rv.type = TYPE_INT;
						rv._int = value0._int / value1._int;
					} else if ((value0.type == TYPE_DOUBLE) || (value1.type == TYPE_DOUBLE)) {
						rv.type = TYPE_DOUBLE;
						rv._double = (value0.type==TYPE_INT ? (double)value0._int : (value0.type==TYPE_LONG ? (double)value0._long : value0._double)) / (value1.type==TYPE_INT ? (double)value1._int : (value1.type==TYPE_LONG? (double)value1._long : value1._double));
					} else {
						rv.type = TYPE_LONG;
						rv._long = (value0.type == TYPE_INT ? (long) value0._int : value0._long) / (value1.type==TYPE_INT ? (long) value1._int : value1._long);
					}
					return rv;
				}
				case '%': {
					//Computes modulo, any doubles are coerced to ints to perform the operation. Int's are coerced to longs if a long is present
					dataType value0 = ex(p->opr.op[0]);
					dataType value1 = ex(p->opr.op[1]);
					dataType rv;
					if((value0.type == TYPE_INT) && (value1.type == TYPE_INT)) {
						rv.type = TYPE_INT;
						rv._int = value0._int % value1._int;
					} else if ((value0.type == TYPE_LONG) || (value1.type == TYPE_LONG)) {
						rv.type = TYPE_LONG;
						rv._long = (value0.type==TYPE_INT ? (long)value0._int : (value0.type==TYPE_LONG ? value0._long : (long)value0._double)) % (value1.type==TYPE_INT ? (long)value1._int : (value1.type==TYPE_LONG? value1._long : (long)value1._double));
					} else {
						rv.type = TYPE_LONG;
						rv._long = (value0.type==TYPE_INT ? (long)value0._int : value0._long) % (value1.type==TYPE_INT ? (long)value1._int : value1._long);
					}
					return rv;
				}
				
				case '^': {
					dataType value0 = ex(p->opr.op[0]);
					dataType value1 = ex(p->opr.op[1]);
					dataType rv;
					if((value0.type == TYPE_INT) && (value1.type == TYPE_INT)){
						long check = (long) pow(value0._int, value1._int);
						if(check >= INT_MAX){
							rv.type = TYPE_LONG;
							rv._long = check;
						} else {
							rv.type = TYPE_INT;
							rv._int = (int) check;
						}
					} else if ((value0.type == TYPE_DOUBLE) || (value1.type == TYPE_DOUBLE)) {
						rv.type = TYPE_DOUBLE;
						rv._double = pow((value0.type==TYPE_INT ? (double)value0._int : (value0.type==TYPE_LONG ? (double)value0._long : value0._double)), (value1.type==TYPE_INT ? (double)value1._int : (value1.type==TYPE_LONG? (double)value1._long : value1._double)));
					} else {
						rv.type = TYPE_LONG;
						rv._long = pow((value0.type==TYPE_INT ? (long)value0._int : value0._long), (value1.type==TYPE_INT ? (long)value0._int : value0._long));
					}
					return rv;
				}
				// rgc: for logic operations, we return an int 1 if true and int 0 if false. (rvTrue and rvFalse)
				case '<':	{
					dataType value0 = ex(p->opr.op[0]);
					dataType value1 = ex(p->opr.op[1]);
					if ((value0.type==TYPE_INT ? value0._int : (value0.type==TYPE_LONG ? (int)value0._long : value0._double)) < (value1.type==TYPE_INT ? value1._int : (value1.type==TYPE_LONG? (int)value1._long : value1._double))) return rvTrue;
					// if we get here it's not true.
					return rvFalse;
				}
				case '>':	{
					dataType value0 = ex(p->opr.op[0]);
					dataType value1 = ex(p->opr.op[1]);
					if ((value0.type==TYPE_INT ? value0._int : (value0.type==TYPE_LONG ? (int)value0._long : value0._double)) > (value1.type==TYPE_INT ? value1._int : (value1.type==TYPE_LONG? (int)value1._long : value1._double))) return rvTrue;
					// if we get here it's not true.
					return rvFalse;
				}
				case GE:	{
					dataType value0 = ex(p->opr.op[0]);
					dataType value1 = ex(p->opr.op[1]);
					if ((value0.type==TYPE_INT ? value0._int : (value0.type==TYPE_LONG ? (int)value0._long : value0._double)) >= (value1.type==TYPE_INT ? value1._int : (value1.type==TYPE_LONG? (int)value1._long : value1._double))) return rvTrue;
					// if we get here it's not true.
					return rvFalse;
				}
				case LE:	{
					dataType value0 = ex(p->opr.op[0]);
					dataType value1 = ex(p->opr.op[1]);
					if ((value0.type==TYPE_INT ? value0._int : (value0.type==TYPE_LONG ? (int)value0._long : value0._double)) <= (value1.type==TYPE_INT ? value1._int : (value1.type==TYPE_LONG? (int)value1._long : value1._double))) return rvTrue;
					// if we get here it's not true.
					return rvFalse;
				}
				case NE:	{
					dataType value0 = ex(p->opr.op[0]);
					dataType value1 = ex(p->opr.op[1]);
					if ((value0.type==TYPE_INT ? value0._int : (value0.type==TYPE_LONG ? (int)value0._long : value0._double)) != (value1.type==TYPE_INT ? value1._int : (value1.type==TYPE_LONG? (int)value1._long : value1._double))) return rvTrue;
					// if we get here it's not true.
					return rvFalse;
				}
				case EQ:	{
					dataType value0 = ex(p->opr.op[0]);
					dataType value1 = ex(p->opr.op[1]);
					if ((value0.type==TYPE_INT ? value0._int : (value0.type==TYPE_LONG ? (int)value0._long : value0._double)) == (value1.type==TYPE_INT ? value1._int : (value1.type==TYPE_LONG? (int)value1._long : value1._double))) return rvTrue;
					// if we get here it's not true.
					return rvFalse;
				}
			}
		}
	}
    return rvFalse;
}
