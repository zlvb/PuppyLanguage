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
#include "error.h"
const char *PU_VERSION = "Puppy 1.5.1";

extern int do_string(Pu *L, const char *str);
extern void clear_state(Pu *L);
extern int vm(Pu *L);
extern const char *get_typestr(__pu_var &v);
extern void regbuiltin(Pu *L);
extern void set_var(Pu *L, const std::string &varname, __pu_var &new_value, __pu_var *&got);
extern __pu_var *reg_var(Pu *L, const std::string *varname);
#ifdef _MSC_VER
#pragma warning(disable:4127) // while(1)
#endif


PUAPI const char *pu_version()
{
    return PU_VERSION;
}

PUAPI void pu_set_return_value(Pu *L, pu_var v)
{
    L->return_value = *v;
}

PUAPI pu_var pu_get_return_value(Pu *L)
{
    return &L->return_value;
}

PUAPI void pu_reg_func(Pu *L, const char *funcname, 
                       ScriptFunc pfunc)
{
    FuncPos fps;
    fps.pfunc = pfunc;
    L->funclist.push_back(fps);

    __pu_var v(L);
    v.SetType(CFUN);
    v.numVal() = (PU_NUMBER)L->funclist.size()-1;
	const std::string *strname = InsertStrPool(L, funcname);
    __pu_var *got = reg_var(L, strname);
    *got = v;        
}

PUAPI const char *pu_str(pu_var v)
{
    return v->strVal().c_str();
}

PUAPI PU_NUMBER pu_num(pu_var v)
{
    return v->numVal();
}

PUAPI pu_var pu_arr(pu_var v, int idx)
{
    return &(v->arr()[idx]);
}

PUAPI void *pu_ptr(pu_var v)
{
    PU_NUMBER n = v->numVal();
    return (void*)*(PU_INT*)&n;
}

PUAPI int pu_type(pu_var v)
{
    return int(v->type());
}

PUAPI PURESULT pu_set_str(pu_var v, const char *str)
{
    if (v->createby_ == PU_SYSTEM)
    {
        return PU_FAILED;
    }

    v->SetType(STR);
    v->strVal() = str;
    return PU_SUCCESS;
}

PUAPI int pu_eval(Pu *L, const char *str)
{
    int ret = do_string(L, str);
    L->isquit = false;
    L->isreturn.clear();
    return ret;
}

PUAPI PURESULT pu_set_num(pu_var v, PU_NUMBER number)
{
    if (v->createby_ == PU_SYSTEM)
    {
        return PU_FAILED;
    }

    v->SetType(NUM);
    v->numVal() = number;
    return PU_SUCCESS;
}

PUAPI PURESULT pu_set_ptr(pu_var v, void *ptr)
{
    if (v->createby_ == PU_SYSTEM)
    {
        return PU_FAILED;
    }

    v->SetType(CPTR);
    PU_INT n = (PU_INT)ptr;
    v->numVal() = *(PU_NUMBER*)&n;
    return PU_SUCCESS;
}

PUAPI PURESULT pu_set_arr(pu_var v, 
                          int idx, pu_var u)
{
    if (v->createby_ == PU_SYSTEM)
    {
        return PU_FAILED;
    }

    if (u->type() == ARRAY)
    {
        return PU_FAILED;
    }

    if (v->type() != ARRAY)
    {
        return PU_FAILED;
    }

    if (v->arr().size() < idx+1)
    {
        return PU_FAILED;
    }

    v->arr()[idx] = *u;
    return PU_SUCCESS;
}


PUAPI pu_var pu_new_value(Pu *L)
{
    __pu_var *v = new __pu_var(L);
    v->createby_ = PU_USER;
    return v;
}

PUAPI PURESULT pu_del_value(pu_var v)
{
    if (v->createby_ == PU_SYSTEM)
    {
        return PU_FAILED;
    }
    delete v;
    return PU_SUCCESS;
}

PUAPI PURESULT pu_push_arr(pu_var varr, 
                           pu_var v)
{
    if (varr->createby_ == PU_SYSTEM)
    {
        return PU_FAILED;
    }

    varr->SetType(ARRAY);

    varr->arr().push_back(*v);
    return PU_SUCCESS;
}

PUAPI PURESULT pu_pop_arr(pu_var varr)
{
    if (varr->createby_ == PU_SYSTEM)
    {
        return PU_FAILED;
    }

    varr->arr().pop_back();
    return PU_SUCCESS;
}

PUAPI int pu_len(pu_var v)
{
    if (v->type() == STR)
    {
        return v->strVal().length();
    }
    else if (v->type() == ARRAY)
    {
        return v->arr().size();
    }
    return -1;
}


PUAPI PUVALUECREATEDBY pu_value_created_by(pu_var v)
{
    return v->createby_;
}

extern void run_coro( Pu * L, int coro_id, __pu_var * corov );
PUAPI void pu_run(Pu *L)
{    
    clear_state(L);
    regbuiltin(L);
    NEXT_TOKEN;
    vm(L);
    while (L->coros.size() > 0)
    {
        run_coro(L, 0, nullptr);
    }        
}

PUAPI pu_var pu_global(Pu *L, const char *name)
{
    StrKeyMap *pvarmap = L->varstack.bottom();
	std::string strkey(name);
    auto it = pvarmap->find(&strkey);
    if (it != pvarmap->end())
    {
        return &(it->second);
    }

    return nullptr;
}

PUAPI pu_var pu_call(Pu *L, const char *funcname, 
                             pu_var varr)
{
    L->return_value.SetType(UNKNOWN);
    pu_var fv = pu_global(L,funcname);
    FuncPos &fps = L->funclist[(int)fv->numVal()];

    const FunArgs &vArgs = fps.argnames;
    L->isreturn.push(0);
    if (fps.start == -1)
    {
        pu_var *args = nullptr;
        ValueArr  vs;
        for (auto &v : varr->arr())
        {
            vs.push_back(v);
        }

        int arg_num = vs.size();

        if (arg_num > 0)
        {
            args = new pu_var[arg_num];
        }

        for (int j = 0; j < arg_num; ++j)
        {
            args[j] = &(vs[j]);
        }

        fps.pfunc(L, int(arg_num), args);

        if (args)
        {
            delete[] args;
        }
    }
    else
    {
        StrKeyMap *newvarmap = new StrKeyMap;
        L->varstack.push(newvarmap);

        for (int j = 0; j < (int)vArgs.size(); ++j)
        {
            __pu_var v = varr->arr()[j];
            newvarmap->insert(std::make_pair(vArgs[j], v));
        }

        L->callstack.push(L->cur_token); 
        L->cur_token = fps.start;     
        NEXT_TOKEN;
        vm(L);
        L->varstack.pop();
        delete newvarmap;

        L->cur_token = L->callstack.top(); 
        L->callstack.pop(); 
    }
    L->isreturn.pop();
    
    return &(L->return_value);
}


PUAPI void pu_set_error_handle(Pu *L, ErrHandle func)
{
    L->err_handle = func;
}

PUAPI void pu_set_output_handle(Pu *L, OutputHandle func)
{
    L->output_handle = func;
}

#define s_WRITE_STR(s,b) \
    strcat(b, s->strVal().c_str());


#define s_WRITE_NUM(n,b) \
    char num[64]={0};\
    if (PU_INT(n->numVal()) == n->numVal())\
        PU_SNPRINTF(num, sizeof(num), "%.lf", n->numVal());\
    else\
        PU_SNPRINTF(num, sizeof(num),"%lf", n->numVal());\
    strcat(b,num);


static void s_write_arr(const __pu_var &arr, char *b)
{
    strcat(b,"[");
    ValueArr::iterator it = arr.arr().begin();
    ValueArr::iterator ite = arr.arr().end();
    while (it != ite)
    {
        __pu_var temp = *it;
        if (temp.type() == STR)
        {
            strcat(b,"\'");
            s_WRITE_STR((&temp),b);
            strcat(b, "\'");
        }
        else if (temp.type() == NUM)
        {
            s_WRITE_NUM((&temp),b);
        }
        else if (temp.type() == ARRAY)
        {
            s_write_arr(temp,b);
        }
        ++it;

        if (it != ite)
            strcat(b,",");
        else
            break;
    }
    strcat(b,"]");
}

PUAPI void pu_val2str(Pu *, pu_var *p, char *b, int buffsize)
{
    b[0]=0;
    pu_var &v = *p;
    if (v->type() == NUM)
    {
        s_WRITE_NUM(v,b);
    }
    else if (v->type() == STR)
    {
        s_WRITE_STR(v,b);
    }
    else if (v->type() == ARRAY)
    {
        s_write_arr(*v,b);
    }
    else if (v->type() == MAP)
    {

    }      
    else if (v->type() == BOOLEANT)
    {
        PU_SNPRINTF(b, buffsize, "%s", (v->numVal() != 0)?"true":"false");
    }
    else
    {
        strncpy(b, get_typestr(*v), buffsize);
    }
}



