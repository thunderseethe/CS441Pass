/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     NUMBER = 258,
     FLOAT = 259,
     VARIABLE = 260,
     WHILE = 261,
     IF = 262,
     PRINT = 263,
     B_BEGIN = 264,
     B_END = 265,
     T_INT = 266,
     T_DOUBLE = 267,
     T_LONG = 268,
     T_LIST = 269,
     CONST = 270,
     DO = 271,
     FOR = 272,
     UNTIL = 273,
     LIST_INIT = 274,
     LIST_SET = 275,
     LIST_ACCESS = 276,
     IFX = 277,
     ELSE = 278,
     NE = 279,
     EQ = 280,
     LE = 281,
     GE = 282,
     UMINUS = 283
   };
#endif
/* Tokens.  */
#define NUMBER 258
#define FLOAT 259
#define VARIABLE 260
#define WHILE 261
#define IF 262
#define PRINT 263
#define B_BEGIN 264
#define B_END 265
#define T_INT 266
#define T_DOUBLE 267
#define T_LONG 268
#define T_LIST 269
#define CONST 270
#define DO 271
#define FOR 272
#define UNTIL 273
#define LIST_INIT 274
#define LIST_SET 275
#define LIST_ACCESS 276
#define IFX 277
#define ELSE 278
#define NE 279
#define EQ 280
#define LE 281
#define GE 282
#define UMINUS 283




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 28 "calc3.y"
{
    long lValue;                 /* integer value */
	double fValue;               /* floating point value */
    char *sName;                /* name of a variable - changed JWJ */
    nodeType *nPtr;             /* node pointer */
}
/* Line 1529 of yacc.c.  */
#line 112 "y.tab.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif

extern YYSTYPE yylval;

