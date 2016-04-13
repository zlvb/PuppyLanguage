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

#ifndef _pu_STATE_
#define _pu_STATE_

#include "PuMap.h"
#include "PuStack.h"
#include "PuVector.h"
#include "def.h"
#include "token.h"
typedef PuStack<int> CallStack;
typedef PuStack<VarMap*> VarStack;
typedef PuMap<PuString, int> LabelMap;
typedef PuVector<PuString, 4> FunArgs;
struct FuncPos
{
	int start;
	int end;
	PuVector<PuString, 4> argnames;
	ScriptFunc pfunc;
	VarMap *newvarmap;

	FuncPos()
        :start(-1)
        ,end(-1)
        ,pfunc(NULL)
	,newvarmap(NULL)
	{

	}
};
typedef PuVector<FuncPos> FuncList;
struct Pustrbuff
{
	Pustrbuff():pos(0),buff(0)
	{}
	int			pos;
	const char *buff;
};

struct Pusource
{
	Pusource():pf(0),type(-1)
	{}
	FILE		*pf;
	Pustrbuff	str;
	int			type;

	enum __SOURCETYPE
	{
		ST_BUFFER,
		ST_FILE
	};
};

struct coro
{
	VarMap		*varmap;
	int		begin;
	int		cur;
	int		end;
	int		funpos;
    int     id;
};

typedef PuVector<coro> CoroList;

struct Pu : public PuMemObj
{
	Pu()
	:token(NULL),
	cur_token(0),
	line(1),
	isquit(false),
    mode(0),
	err_handle(0),	
	output_handle(0),	
	lasterr(-1),
	isyield(false),
	upvalue(0),
	cur_nup(0),
	gclink(0),
	builtinreg(false),
    tail_optimize(false)
	{
		varstack.push(new VarMap);
		return_value.SetType(NIL);
	}

	~Pu()
	{
		for (int i = 0; i < funclist.size(); i++)
		{
			FuncPos &info = funclist[i];
			if (info.newvarmap)
			{
				delete info.newvarmap;
			}
		}
		VarMap *nd = varstack.bottom();
		delete nd;
	}

	Token				*token;
	int					cur_token;
	int					line;
    PuStack<bool>		isreturn;
	bool				isquit;
	int					mode;
    PuVector<int>       funstack;
	ErrHandle			err_handle;
	OutputHandle		output_handle;
	int					lasterr;
	bool				isyield;
	TokenList			tokens;
	Pusource			source;
	LabelMap			labelmap;
	FuncList			funclist;
	VarStack			varstack;
	CallStack			callstack;
	__pu_value			return_value;
	PuVector<PuString>	current_filename;
	CoroList			coros;
	PuStack<int>		uncomdef;
	PuStack<int>		jumpstack;
	VarMap				*upvalue;
	_up_value			*cur_nup;
	_up_value			*gclink;
    PuVector<__pu_value*> tempool;
    PuVector<__pu_value*> tempvals;
    bool                builtinreg;
    bool                tail_optimize;
};


#endif
