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

#include "state.h"
#include "util.h"

#if defined(_MSC_VER) && defined(_DEBUG)
#if _DEBUG_MEM == 1
#include <crtdbg.h>
#endif
#endif
extern void bi_get_value_len(Pu *L, int, pu_var *v);
extern void bi_date(Pu *L, int ,pu_var*);
extern void bi_time(Pu *L, int, pu_var*);
extern void bi_get_value_len(Pu *L, int, pu_var *v);
extern void bi_write(Pu *L, int ,pu_var *v);
extern void bi_read(Pu *L, int ,pu_var *v);
extern void bi_rand(Pu *L, int ,pu_var *v);
extern void bi_sleep(Pu *L, int, pu_var *v);
extern void bi_type(Pu *L, int, pu_var *v);
extern void bi_eval(Pu *L, int, pu_var *v);
extern void bi_quit(Pu *L, int, pu_var *);
extern void bi_read(Pu *L, int argc, pu_var *v);
extern void bi_open(Pu *L, int argc, pu_var *v);
extern void bi_close(Pu *L, int argc, pu_var *v);
extern void bi_str(Pu *L, int argc, pu_var *v);
extern void bi_num(Pu *L, int argc, pu_var *v);
extern void bi_coro_create(Pu *L, int argnum, pu_var *v);
extern void bi_coro_resume(Pu *L, int argnum, pu_var *v);
extern void bi_coro_yield(Pu *L, int argnum, pu_var *v);
extern void bi_get_var(Pu *L, int argnum, pu_var *v);
PUAPI void pu_reg_func(Pu *L, const char *funcname, ScriptFunc pfunc);

Pu::Pu()
    :token(nullptr),
    cur_token(0),
    line(1),
    isquit(false),
    mode(0),
    err_handle(0),    
    output_handle(0),    
    lasterr(-1),
    isyield(false),
	return_value(this),
	up_scope(this, nullptr),
	cur_scope(this, new StrKeyMap),
    builtinreg(false),
    tail_optimize(false),
	start_gc(false)
{
    varstack.push(cur_scope.base_->vmap_);
    return_value.SetType(NIL);
    debug(this, "Global scop = %p", cur_scope.base_->vmap_);
}

extern void gc(Pu *L);
extern void sweep(Pu *L);
Pu::~Pu()
{
	gc(this);
	sweep(this);
	StrKeyMap *global_vmap = varstack.bottom();
	delete global_vmap;

	for (auto p : tempool)
	{
		delete p;
	}

	for (auto &pair : strpool)
	{
		delete pair.second;
	}

	for (auto &pair : const_str_vals)
	{
		delete pair.second;
	}

	for (auto &pair : const_num_vals)
	{
		delete pair.second;
	}
}

PUAPI Pu *pu_open()
{
#if defined(_MSC_VER) && defined(_DEBUG)
#if _DEBUG_MEM == 1
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
#endif
#endif

    Pu *L = new Pu;
    return L;
}

void regbuiltin(Pu *L)
{
    if (L->builtinreg)
    {
        return;
    }
    L->builtinreg = true;
    pu_reg_func(L, "len", bi_get_value_len);
    pu_reg_func(L, "write", bi_write);
    pu_reg_func(L, "read", bi_read);
    pu_reg_func(L, "rand", bi_rand);
    pu_reg_func(L, "date", bi_date);
    pu_reg_func(L, "time", bi_time);
    pu_reg_func(L, "sleep", bi_sleep);
    pu_reg_func(L, "type", bi_type);
    pu_reg_func(L, "eval", bi_eval);
    pu_reg_func(L, "quit", bi_quit);
    pu_reg_func(L, "coro_create", bi_coro_create);
    pu_reg_func(L, "coro_resume", bi_coro_resume);
    pu_reg_func(L, "coro_yield", bi_coro_yield);
    pu_reg_func(L, "open", bi_open);
    pu_reg_func(L, "read", bi_read);
    pu_reg_func(L, "close", bi_close);
    pu_reg_func(L, "str", bi_str);
    pu_reg_func(L, "num", bi_num);
    pu_reg_func(L, "get_var", bi_get_var);    
}

void clear_state(Pu *L)
{
    if (L->source.pf)
    {
        fclose(L->source.pf);
        L->source.pf = nullptr;
    }

    L->source.str.buff = 0;
    L->source.str.pos = 0;
    L->source.type = -1;
    L->callstack.clear();
    L->cur_token = 0;
    L->isquit = false;
    L->isreturn.clear();
    L->return_value.SetType(NIL);
}

PUAPI void pu_close(Pu *L)
{
    clear_state(L);
    delete L;
}
