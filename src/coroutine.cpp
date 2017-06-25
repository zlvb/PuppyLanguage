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

#include "def.h"
#include "value.h"
#include "state.h"
#include "error.h"

PUAPI void pu_set_return_value(Pu *L, pu_var v);
extern int vm(Pu *L);
extern void bi_return_int( Pu * L, int v );
void bi_coro_create(Pu *L, int argnum, pu_var *v)
{
    if (argnum == 0)
    {
        return;
    }

    if (v[0]->type() != STR && v[0]->type() != FUN)
    {
        return;
    }

    FuncPos &fps = L->funclist[(int)v[0]->intVal()];

    const FunArgs &args = fps.argnames;

    if (argnum - 1 != (int)args.size())
    {
        error(L,27);
        return;
    }

    if (fps.start == -1)
    {
        return;
    }
    else
    {
        StrKeyMap *newvarmap = new StrKeyMap;
        
        int i=0;

        for (; i < argnum - 1; ++i)// )
        {
            newvarmap->insert(std::make_pair(args[i], *(v[i+1])));
        }
        coro c;
        c.begin = fps.start; 
        c.end = fps.end;
        c.cur = c.begin;
        c.funpos = (int)v[0]->intVal();
        c.varmap = newvarmap;
        c.id = L->coros.size();
        L->coros.push_back(c);        
        __pu_var retv(L);
        retv.SetType(CORO);
        retv.intVal() = (PU_INT)L->coros.size()-1;
        pu_set_return_value(L, &retv);
    }
}

void run_coro( Pu * L, int coro_id, __pu_var * corov );

void bi_coro_resume(Pu *L, int argnum, pu_var *v)
{
    int coro_id = 0;
    __pu_var *corov = nullptr;
    if (argnum == 0)
    {
        coro_id = rand() % L->coros.size();        
    }
    else
    {
        corov = v[0];
        if (corov->type() != CORO)
        {
            bi_return_int(L, -1);
            return;
        }
        coro_id = int(corov->intVal());        
    }

    if (coro_id < 0 || coro_id >= (int)L->coros.size())
    {
        return;
    }
    
    run_coro(L, coro_id, corov);
    return;
}

void bi_coro_yield(Pu *L, int, pu_var *)
{
    L->isyield = true;
}

void run_coro( Pu *L, int coro_id, __pu_var *corov )
{
    coro &c = L->coros[coro_id];

    if (c.cur == c.end)
    {
        bi_return_int(L, 0);
        return;
    }

    L->varstack.push(c.varmap);
    L->callstack.push(L->cur_token); 
    L->isreturn.push(0);
    L->cur_token = c.cur;
    NEXT_TOKEN;

    if (c.cur != c.begin)
    {
        PREV_TOKEN;
    }

    vm(L);
    L->varstack.pop();
    c.cur = L->cur_token;
    L->cur_token = L->callstack.top(); 
    L->callstack.pop(); 
    L->isreturn.pop();

    __pu_var r = __pu_var(L);
    r.SetType(INTEGER);
    if (L->isyield)
    {
        r.intVal() = 1;
        coro cbak = L->coros[coro_id];
        L->coros[coro_id] = L->coros.back();
        L->coros.back() = cbak;
    }
    else
    {
        r.intVal() = 0;
        if (corov)
        {
            corov->intVal() = -1;
        }
        delete c.varmap;
        c.varmap = nullptr;
        L->coros.erase(L->coros.begin() + coro_id);
    }    
    pu_set_return_value(L, &r);
    L->isyield = false;
}



