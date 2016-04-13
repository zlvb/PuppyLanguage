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
extern const char *get_typestr(__pu_value &v);
extern void regbuiltin(Pu *L);
extern void set_var(Pu *L, const PuString &varname, __pu_value &new_value, __pu_value *&got);
extern __pu_value *reg_var(Pu *L, const PuString &varname);
#ifdef _MSC_VER
#pragma warning(disable:4127) // �ж�����Ϊ���������磺while(1)
#endif

pumalloc g_pumalloc = malloc;
pufree g_pufree = free;

PUAPI const char *pu_version()
{
	return PU_VERSION;
}

PUAPI void pu_set_return_value(Pu *L, pu_value v)
{
	L->return_value = *v;
}

PUAPI pu_value pu_get_return_value(Pu *L)
{
    return &L->return_value;
}

PUAPI pumalloc pu_set_malloc(pumalloc fun_malloc)
{
    pumalloc old = g_pumalloc;
    g_pumalloc = fun_malloc;
    return old;
}

PUAPI pufree pu_set_free(pufree fun_free)
{
    pufree old = g_pufree;
    g_pufree = fun_free;
    return old;
}

PUAPI void pu_reg_func(Pu *L, const char *funcname, 
					   ScriptFunc pfunc)
{
	FuncPos fps;
	fps.pfunc = pfunc;
	L->funclist.push_back(fps);

	__pu_value v(L);
    v.SetType(CFUN);
    v.numVal() = (PU_NUMBER)L->funclist.size()-1;
    __pu_value *got = reg_var(L, funcname);
    *got = v;        
}

PUAPI const char *pu_str(pu_value v)
{
	return v->strVal().c_str();
}

PUAPI PU_NUMBER pu_num(pu_value v)
{
	return v->numVal();
}

PUAPI pu_value pu_arr(pu_value v, int idx)
{
	return &(v->arr()[idx]);
}

PUAPI void *pu_ptr(pu_value v)
{
    PU_NUMBER n = v->numVal();
    return (void*)*(PU_INT*)&n;
}

PUAPI int pu_type(pu_value v)
{
	return int(v->type());
}

PUAPI PURESULT pu_set_str(pu_value v, const char *str)
{
	if (v->createby == PU_SYSTEM)
	{
		return PU_FAILED;
	}

// 	if (v->type() == ARRAY)
// 	{
// 		v->arr().decref();
// 	}


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

PUAPI PURESULT pu_set_num(pu_value v, PU_NUMBER number)
{
	if (v->createby == PU_SYSTEM)
	{
		return PU_FAILED;
	}

// 	if (v->type() == ARRAY)
// 	{
// 		v->arr().decref();
// 	}


	v->SetType(NUM);
	v->numVal() = number;
	return PU_SUCCESS;
}

PUAPI PURESULT pu_set_ptr(pu_value v, void *ptr)
{
    if (v->createby == PU_SYSTEM)
    {
        return PU_FAILED;
    }

    v->SetType(CPTR);
    PU_INT n = (PU_INT)ptr;
    v->numVal() = *(PU_NUMBER*)&n;
    return PU_SUCCESS;
}

PUAPI PURESULT pu_set_arr(pu_value v, 
						  int idx, pu_value u)
{
	if (v->createby == PU_SYSTEM)
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


PUAPI pu_value pu_new_value(Pu *L)
{
	__pu_value *v = new __pu_value(L);
	v->createby = PU_USER;
	return v;
}

PUAPI PURESULT pu_del_value(pu_value v)
{
	if (v->createby == PU_SYSTEM)
	{
		return PU_FAILED;
	}
	delete v;
	return PU_SUCCESS;
}

PUAPI PURESULT pu_push_arr(pu_value varr, 
						   pu_value v)
{
	if (varr->createby == PU_SYSTEM)
	{
		return PU_FAILED;
	}

	varr->SetType(ARRAY);

	varr->arr().push_back(*v);
	return PU_SUCCESS;
}

PUAPI PURESULT pu_pop_arr(pu_value varr)
{
	if (varr->createby == PU_SYSTEM)
	{
		return PU_FAILED;
	}

	varr->arr().pop_back();
	return PU_SUCCESS;
}

PUAPI int pu_len(pu_value v)
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


PUAPI PUVALUECREATEDBY pu_value_created_by(pu_value v)
{
	return v->createby;
}

extern void run_coro( Pu * L, int coro_id, __pu_value * corov );
PUAPI void pu_run(Pu *L)
{    
	clear_state(L);
	regbuiltin(L);
	NEXT_TOKEN;
	vm(L);
    while (L->coros.size() > 0)
    {
        run_coro(L, 0, NULL);
    }        
}

PUAPI pu_value pu_global(Pu *L, const char *name)
{
	VarMap *pvarmap = L->varstack.bottom();
	VarMap::Bucket_T *ik = pvarmap->find(name);
	if (ik != 0)
	{
		return &(ik->value);
	}

	return NULL;
}

PUAPI pu_value pu_call(Pu *L, const char *funcname, 
							 pu_value varr)
{
	L->return_value.SetType(UNKNOWN);
	pu_value fv = pu_global(L,funcname);
	FuncPos &fps = L->funclist[(int)fv->numVal()];

	const FunArgs &vArgs = fps.argnames;
    L->isreturn.push(0);
	if (fps.start == -1)
	{
		pu_value *args = NULL;
		ValueArr  vs;
		for (int j=0; j<varr->arr().size(); ++j)
		{
			vs.push_back(varr->arr()[j]);
		}

		int arg_num = vs.size();

		if (arg_num > 0)
		{
			args = (pu_value *)g_pumalloc(arg_num * sizeof(__pu_value*));
		}

		for (int j = 0;  j < arg_num; ++j)
		{
			args[j] = &(vs[j]);
		}

		fps.pfunc(L, int(arg_num), args);

		if (args)
			g_pufree(args);
	}
	else
	{
		VarMap *newvarmap = new VarMap;
		L->varstack.push(newvarmap);

		for (int j=0; j<vArgs.size(); ++j)
		{
			__pu_value v = varr->arr()[j];
			newvarmap->insert(vArgs[j], v);
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


static void s_write_arr(const __pu_value &arr, char *b)
{
	strcat(b,"[");
	ValueArr::iterator it = arr.arr().begin();
	ValueArr::iterator ite = arr.arr().end();
	while (it != ite)
	{
		__pu_value temp = *it;
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

PUAPI void pu_val2str(Pu *, pu_value *p, char *b, int buffsize)
{
	b[0]=0;
	pu_value &v = *p;
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



