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
typedef PuStack<StrKeyMap*> VarStack;
typedef PuMap<PuString, int> LabelMap;
typedef std::vector<PuString> FunArgs;
struct FuncPos
{
    int start;
    int end;
    std::vector<PuString> argnames;
    ScriptFunc pfunc;
    StrKeyMap *newvarmap;

    FuncPos()
        :start(-1)
        ,end(-1)
        ,pfunc(NULL)
    ,newvarmap(NULL)
    {

    }
};
typedef std::vector<FuncPos> FuncList;
struct Pustrbuff
{
    Pustrbuff():pos(0),buff(0)
    {}
    int            pos;
    const char *buff;
};

struct Pusource
{
    Pusource():pf(0),type(-1)
    {}
    FILE        *pf;
    Pustrbuff    str;
    int            type;

    enum __SOURCETYPE
    {
        ST_BUFFER,
        ST_FILE
    };
};

struct coro
{
    StrKeyMap    *varmap;
    int           begin;
    int           cur;
    int           end;
    int           funpos;
    int       id;
};

typedef std::vector<coro> CoroList;

struct Pu
{
    Pu();
    ~Pu();

    Token                *token;
    int                    cur_token;
    int                    line;
    PuStack<int>        isreturn;
    bool                isquit;
    int                    mode;
    std::vector<int>       funstack;
    ErrHandle            err_handle;
    OutputHandle        output_handle;
    int                    lasterr;
    bool                isyield;
    TokenList            tokens;
    Pusource            source;
    LabelMap            labelmap;
    FuncList            funclist;
    VarStack            varstack;
    CallStack            callstack;
    __pu_value            return_value;
    std::vector<PuString>    current_filename;
    CoroList            coros;
    PuStack<int>        uncomdef;
    PuStack<int>        jumpstack;
    StrKeyMap                *upvalue;
    _up_value            *cur_nup;
    _up_value            *gclink;
    std::vector<__pu_value*> tempool;
    std::vector<__pu_value*> tempvals;
    bool                builtinreg;
    bool                tail_optimize;
    std::vector<std::vector<CONTROL_PATH> > *control_flow;
};


#endif
