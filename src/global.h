/*
    Copyright (c) 2009 Zhang li

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.

    MIT License: http://www.opensource.org/licenses/mit-license.php
*/

/*
    Author zhang li
    Email zlvbvbzl@gmail.com
*/

#ifndef __GLOBAL_H_
#define __GLOBAL_H_

#include "config.h"
#include <stddef.h>

#define KEYWORDS_TYPES_LENGTH 17
#define OPERATOR_TYPES_LENGTH 28
#define SPACE_TYPES_LENGTH 4

extern const char SPACE_TYPES[];
extern const char *KEYWORDS[];
extern const char *OPERATORS[];
extern const char *OPERATOR_CHARS;

enum OperatorType
{    
    OPT_ADD,
    OPT_SUB,
    OPT_MUL,
    OPT_DIV,
    OPT_GT,
    OPT_LT,
    OPT_SET,
    OPT_EQ,
    OPT_GTA,
    OPT_LTA, // <=
    OPT_NEQ, // !=
    OPT_LB,  // (
    OPT_RB,  // )
    OPT_OR,
    OPT_AND,
    OPT_MOD,
    OPT_LSB, // [
    OPT_RSB, // ]
    OPT_COM, // ,
    OPT_LBR, // {
    OPT_RBR, // }
    OPT_ADDS,
    OPT_SUBS,
    OPT_NOT,
    OPT_MULS,
    OPT_DIVS,
    OPT_COL, // :
    OPT_SEM, // ;
//--------------
    OPT_COUNT,
    OPT_UNKNOWN = -1,
};


enum PuType
{
    KW_WHILE, // 0
    KW_END,  // 1
    KW_IF, // 2
    KW_ELSE, // 3
    KW_BREAK, // 4
    KW_GOTO, // 5
    KW_FUNCTION, // 6
    KW_RETURN, // 7
    KW_INCLUDE, // 8
    KW_CONTINUE, // 9
    KW_ELIF, // 10
    KW_NIL, // 11
    KW_FALSE, // 12
    KW_TRUE, // 13
    KW_VAR, // 14
    KW_FOR, // 15
    KW_IN, // 16
//-----------------------------------------------
    OP = 500, // 
    VAR, // 
    FINISH,
    LABEL,
//-----------------------------------------------
    UNKNOWN = 1000,
    NIL,
    NUM,
    STR,
    ARRAY, 
    MAP, 
    CORO, 
    FILEHANDLE, 
    BOOLEANT, 
    INTEGER, 
    FUN, 
    CFUN,        
    CPTR    
};

enum CODEFROM{
    FROM_BYTECODE,
    FROM_SOURCECODE
};

#endif


