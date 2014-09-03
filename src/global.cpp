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

#include "global.h"

const char *OPERATOR_CHARS = "+-*/<>!=()|&%[],{}:";
const char *KEYWORDS[] = {"while", "end", "if", "else", 
								"break", "goto", "def", "return", 
								"include", "continue", "elif", "null", "false", "true"};


const char SPACE_TYPES[] = {' ', '\t', '\n', '\r'};

const char *OPERATORS[OPT_COUNT] = {};

static int init_operator()
{
    OPERATORS[OPT_ADD] =   "+";//0
    OPERATORS[OPT_SUB] =   "-";//1 
    OPERATORS[OPT_MUL] =   "*";//2 
    OPERATORS[OPT_DIV] =   "/";//3 
    OPERATORS[OPT_GT] =    ">";//4 
    OPERATORS[OPT_LT] =    "<";//5 
    OPERATORS[OPT_SET] =   "=";//6 
    OPERATORS[OPT_EQ] =    "==";//7 
    OPERATORS[OPT_GTA] =   ">=";//8 
    OPERATORS[OPT_LTA] =   "<=";//9 
    OPERATORS[OPT_NEQ] =   "!=";//10 
    OPERATORS[OPT_LB] =    "(";//11 
    OPERATORS[OPT_RB] =    ")";//12 
    OPERATORS[OPT_OR] =    "||";//13
    OPERATORS[OPT_AND] =   "&&";//14
    OPERATORS[OPT_MOD] =   "%";//15 
    OPERATORS[OPT_LSB] =   "[";//16
    OPERATORS[OPT_RSB] =   "]";//17 
    OPERATORS[OPT_COM] =   ",";//18 
    OPERATORS[OPT_LBR] =   "{";//19 
    OPERATORS[OPT_RBR] =   "}";//20 
    OPERATORS[OPT_ADDS] =  "+=";//21 
    OPERATORS[OPT_SUBS] =  "-=";//22
    OPERATORS[OPT_NOT] =   "!";//23
    OPERATORS[OPT_MULS] =  "*=";//24
    OPERATORS[OPT_DIVS] =  "/=";//25
    OPERATORS[OPT_COL] =   ":";//26
    return 0;
};

static int init_op = init_operator();