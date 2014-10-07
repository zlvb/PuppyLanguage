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

#ifndef __TOKEN_H_
#define __TOKEN_H_

#include "value.h"

struct Token
{
	Token()
        :type(UNKNOWN)
        ,line(0)
        ,optype(OPT_UNKNOWN)
        ,regvar(NULL){}
	Token(const Token &x);
	void operator=(const Token &x);
	PuType type;
	int line;
	OperatorType optype;
	PuString filename;
	__pu_value value;
    __pu_value *regvar;
};

typedef PuVector<Token> TokenList;
#define check_source_end(c,s)							\
(														\
	((s).type == Pusource::ST_FILE && (c) == EOF)||	\
	((s).type == Pusource::ST_BUFFER   && (c) =='\0')		\
)

#define CHECKTOKENERROR if (L->lasterr >= 0) return
#define PU_CODE_FROM_FILE 0
#define PU_CODE_FROM_BUFF 1
#define PU_FUNCTION_UNCOMPLETED 2

#endif
