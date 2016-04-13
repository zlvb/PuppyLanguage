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


#ifdef _MSC_VER
#pragma warning(disable:4127) // �ж�����Ϊ���������磺while(1)
#endif

#include "state.h"
#include "error.h"
#include "util.h"
#include <vector>

int vm(Pu *L);
static void term(Pu *L, __pu_value *&temp);
static void factor(Pu *L, __pu_value *&temp);
static void cmp(Pu *L, __pu_value *&temp);
static void logc(Pu *L, __pu_value *&temp);
static void add(Pu *L, __pu_value *&temp);
static void callfunction(Pu *L, FuncPos &finfo);

#define _CHT PuType tp = TOKEN.type;int nv = TOKEN.optype;
#define _TERM(L,t)	{_CHT if(tp == OP && (nv==OPT_MUL||nv==OPT_DIV||nv==OPT_MOD))term(L,t);}
#define _ADD(L,t)	{_CHT if(tp == OP && (nv==OPT_ADD||nv==OPT_SUB))add(L,t);}
#define _CMP(L,t)	{_CHT if(tp == OP && (nv==OPT_GT||nv==OPT_LT||nv==OPT_EQ||nv==OPT_GTA||nv==OPT_LTA||nv==OPT_NEQ))cmp(L,t);}
#define _LOGC(L,t)	{_CHT if(tp == OP && (nv==OPT_OR||nv==OPT_AND))logc(L,t); }
#define LOGC(L,t)	factor(L,t); _TERM(L,t) _ADD(L,t) _CMP(L,t) _LOGC(L,t)
#define CMP(L,t)	factor(L,t); _TERM(L,t) _ADD(L,t) _CMP(L,t)
#define ADD(L,t)	factor(L,t); _TERM(L,t) _ADD(L,t)
#define TERM(L,t)	factor(L,t); _TERM(L,t)
#define FACTOR(L,t)	factor(L,t);
#define DO_GC if (L->tempvals.size() >= GC_LEVEL){ clear_temp(L);if(L->gclink != 0 && L->varstack.size() == 1){gc(L);}}

extern void gc(Pu *L);
extern void clear_temp(Pu *L);
/*
����չ������
factor(L,t);
{
TokenType tp = TOKEN.type();
int nv = TOKEN.optype;
if (tp==OP && nv==2 ...)
term(L,t);
}
{
TokenType tp = TOKEN.type();
int nv = TOKEN.optype;
if (tp==OP ...)
add(L,t);

}...

*/

#define CLEAR_RETURN							\
L->return_value.SetType(NIL);

static __pu_value *get_varref(Pu *L, PuString &name)
{
    VarMap *pvarmap = L->varstack.top();
    VarMap::Bucket_T *ik = pvarmap->find(name);
    if (ik != 0)
    {
        return &(ik->value);
    }

    pvarmap = L->varstack.bottom();
    ik = pvarmap->find(name);
    if (ik != 0)
    {
        return &(ik->value);
    }

    return NULL;
}

void get_var(Pu *L, const PuString &name, __pu_value *&v)
{
	VarMap *pvarmap = L->varstack.top();

	VarMap::Bucket_T *ik = pvarmap->find(name);
	if (ik != 0)
	{
		v = &ik->value;
		return;
	}

	pvarmap = L->varstack.bottom();
	ik = pvarmap->find(name);
	if (ik != 0)
	{
		v = &ik->value;
		return;
	}

	if (L->upvalue && L->upvalue != pvarmap)
	{
		pvarmap = L->upvalue;
		ik = pvarmap->find(name);
		if (ik != 0)
		{
			v = &ik->value;
			return;
		}
	}

    MAKE_TEMP_VALUE(v);
	error(L,7);
}

__pu_value *reg_var(Pu *L, const PuString &varname)
{
    __pu_value *got = NULL;
    VarMap *top_varmap = L->varstack.top();
    VarMap::Bucket_T *it = top_varmap->find(varname);
    if (it != 0)
    {
        got = &(it->value);
    }
    else 
    {
        VarMap *global_varmap = L->varstack.bottom();
        it = global_varmap->find(varname);
        if (it != 0)
        {
            got = &(it->value);
        }
        else
        {
            if (L->upvalue && L->upvalue != top_varmap)
            {
                VarMap *up_varmap = L->upvalue;
                it = up_varmap->find(varname);
                if (it != 0)
                {
                    got = &(it->value);
                }
            }
        }
    }

    if (got == NULL)
    {
        VarMap *varmap = L->varstack.top();
        VarMap::Bucket_T *ret = varmap->insert(varname, __pu_value(L));
        got = &(ret->value);
    }

    return got;
}

static const __pu_value *exp(Pu *L) 
{
	Token *ptoken = &TOKEN;
	__pu_value *temp = NULL;
	do{
		if (ptoken->type == VAR)
		{
			NEXT_TOKEN;
			PuType tp = TOKEN.type;
			OperatorType nv = TOKEN.optype;
			if (tp == OP 
				&& (nv == OPT_SET || nv == OPT_ADDS || nv == OPT_SUBS || nv == OPT_MULS || nv == OPT_DIVS))
			{
				break;
			}
			PREV_TOKEN;
		}

		LOGC(L,temp);
        CHECK_EXP_RETURN(temp, temp);

	}while(false);

	PuType tp = TOKEN.type;
	OperatorType nv = TOKEN.optype;
	if (!(tp == OP 
		&& (nv == OPT_SET || nv == OPT_ADDS || nv == OPT_SUBS || nv == OPT_MULS || nv == OPT_DIVS)))
		return temp;

	if (ptoken->type != VAR)
	{
		error(L,22);
		__pu_value *result = NULL;
        MAKE_TEMP_VALUE(result);
        return result;
	}

	NEXT_TOKEN;
	const __pu_value *t = exp(L);
    CHECK_EXP_RETURN(t, t);

    switch (nv)
	{
	case OPT_SET:{// =
	    if (!ptoken->regvar)
	    {
		    ptoken->regvar = reg_var(L, ptoken->value.strVal());
	    }
        __pu_value *var_value = ptoken->regvar;
        assert( !(var_value->type() == UNKNOWN && temp != NULL) );

        if (temp == NULL)
        {
            temp = var_value;
        }
        
        *temp = *t;
        return temp;}
	case OPT_ADDS:// +=
		{
			__pu_value *pv = get_varref(L, ptoken->value.strVal());
            ptoken->regvar = pv;
            CHECK_EXP_RETURNERR(pv);
			return &pv->operator+=(*t);

		}
	case OPT_SUBS:// -=
		{
			__pu_value *pv = get_varref(L, ptoken->value.strVal());
            ptoken->regvar = pv;
            CHECK_EXP_RETURNERR(pv);
			return &pv->operator-=(*t);
		}
	case OPT_MULS:// *=
		{
			__pu_value *pv = get_varref(L, ptoken->value.strVal());
            ptoken->regvar = pv;
            CHECK_EXP_RETURNERR(pv);
			return &pv->operator*=(*t);
		}
	case OPT_DIVS:// /=
		{
			__pu_value *pv = get_varref(L, ptoken->value.strVal());
            ptoken->regvar = pv;
            CHECK_EXP_RETURNERR(pv);
			return &pv->operator/=(*t);
		}
	default:
		;
	}

    return t;	
}

static void get_value(Pu *L, __pu_value *&temp)
{
	PuType tp = TOKEN.type;
	OperatorType nv = TOKEN.optype;
	PuString &sv = TOKEN.value.strVal();
	if (tp == VAR)
	{
        if (!TOKEN.regvar)
        {
            get_var(L, sv, TOKEN.regvar);            
        }
		temp = TOKEN.regvar;
		NEXT_TOKEN;
	}
	else if (tp == OP && ((nv == OPT_ADD) || (nv == OPT_SUB) || (nv == OPT_NOT)))
	{
		if (nv == OPT_SUB) // -
		{
			NEXT_TOKEN;
			get_value(L, temp);
            CHECK_EXP(temp);
			temp->numVal() *= -1;
		}
		else if (nv == OPT_NOT) // !
		{
			NEXT_TOKEN;
			get_value(L,temp);
            CHECK_EXP(temp);
			bool bbb = VALUE_IS_TRUE(*temp);
			temp->SetType(BOOLEANT);
			temp->numVal() = (PU_NUMBER)(bbb?0:1);
		}
		else
		{
			NEXT_TOKEN;
			get_value(L,temp);
		}
	}
	else
	{
		tp = TOKEN.type;
        MAKE_TEMP_VALUE(temp);
		switch (tp){
		case NIL: temp->SetType(NIL);
			break;
		case FALSEK: temp->SetType(BOOLEANT);
			temp->numVal() = 0;
			break;
		case TRUEK: temp->SetType(BOOLEANT);
			temp->numVal() = 1;
			break;
		case NUM: temp->SetType(NUM);
			temp->numVal() = TOKEN.value.numVal();
			break;
		case STR: temp->SetType(STR);
			temp->strVal() = TOKEN.value.strVal();
			break;
		default: 
            if (L->isreturn.top())
            {
                temp->SetType(NIL);
                break;
            }
            error(L,29);
			return;
		}
		NEXT_TOKEN;	
	}
}

static void get_arrvalue(Pu *L, __pu_value *&arrref, const int idx)
{
	if (arrref->type() == ARRAY)
	{// ����
		if (idx > int(arrref->arr().size()-1) || idx<0)
		{// �ж��±�Խ��
			error(L,5);
            MAKE_TEMP_VALUE(arrref);
			return;
		}

		arrref = &arrref->arr()[idx];
	}
	else if (idx >= 0)
	{// �ַ���
		if (idx > int(arrref->strVal().length()-1) || idx<0)
		{
			error(L,5);
            MAKE_TEMP_VALUE(arrref);
			return;
		}

		char buff[2] = {0};
		buff[0] = arrref->strVal()[idx];
        MAKE_TEMP_VALUE(arrref);
        arrref->SetType(STR);
		arrref->strVal() = buff;
	}
}

static void get_arrref(Pu *L, __pu_value *&temp)
{
	NEXT_TOKEN;
	const __pu_value *vidx = exp(L);
    CHECK_EXP(vidx);
	if (vidx->type() != NUM)
	{// ֻ�����ֲſ�����Ϊ����������
		error(L,3);
        MAKE_TEMP_VALUE(temp);
		return;
	}

	PuType tp = TOKEN.type;
	OperatorType nv = TOKEN.optype;
	if (tp != OP || nv != OPT_RSB)
	{// ����������������[��ʼ����]����
		error(L,4);
        MAKE_TEMP_VALUE(temp);
		return;
	}

	int idx = int(vidx->numVal());
	get_arrvalue(L, temp, idx);// ��ȡ����ָ����ֵ
}
PUAPI void pu_val2str(Pu *, const pu_value *p, char *b, int buffsize);
static void get_map(Pu *L, __pu_value *&a)
{
    __pu_value v;
    PuType last_tp = UNKNOWN;
    OperatorType last_nv = OPT_UNKNOWN;
    MAKE_TEMP_VALUE(a);
    a->SetType(MAP);
    NEXT_TOKEN;
    for (;;)
    {
        PuType tp = TOKEN.type;
        OperatorType nv = TOKEN.optype;
        if (tp == OP && nv == OPT_COM) // ,
        {
            NEXT_TOKEN;
            goto CONTINUE_NEXT;
        }
        else if (tp == OP && nv == OPT_RBR) // }
        {
            NEXT_TOKEN;
            break;
        }
        else if (tp == OP && nv == OPT_COL) // :
        {
            NEXT_TOKEN;
            goto CONTINUE_NEXT;
        }
        else
        {
            --L->cur_token;
            NEXT_TOKEN;
            const __pu_value *v0 = exp(L);
            CHECK_EXP(v0);
            if (last_tp == OP && last_nv == OPT_COL)
            {
                char buff[256];
                pu_value pv = &v;
                pu_val2str(L, &pv, buff, sizeof(buff));
                a->map().insert(buff, *v0);
            }
            else
            {
                v = *v0;
            }
        }
CONTINUE_NEXT:
        last_tp = tp;
        last_nv = nv;
    }
}
#ifdef _DEBUG
static void get_array(Pu *L, __pu_value *&a)
{
    MAKE_TEMP_VALUE(a);
    a->SetType(ARRAY);
	NEXT_TOKEN;
	for (;;)
	{
		PuType tp = TOKEN.type;
		OperatorType nv = TOKEN.optype;
		if (tp == OP && nv == OPT_COM) // ,
		{
			NEXT_TOKEN;
			continue;
		}
		else if (tp == OP && nv == OPT_RSB) // ]
		{
			NEXT_TOKEN;
			break;
		}
		else
		{
			--L->cur_token;
			NEXT_TOKEN;
			const __pu_value *v = exp(L);
            CHECK_EXP(v);
			a->arr().push_back(*v);
		}
	}
}
#else
#define get_array(L,a)					\
{										\
    MAKE_TEMP_VALUE(a);                 \
	a->SetType(ARRAY);					\
	NEXT_TOKEN;							\
	for (;;)							\
	{									\
		PuType tp = TOKEN.type;		    \
		OperatorType nv = TOKEN.optype;	\
		if (tp == OP && nv == OPT_COM)	\
		{								\
			NEXT_TOKEN;					\
			continue;					\
		}								\
		else if (tp == OP && nv == OPT_RSB)	\
		{								\
			NEXT_TOKEN; break;			\
		}								\
		else							\
		{								\
			--L->cur_token;				\
			NEXT_TOKEN;					\
			const __pu_value *v = exp(L);\
            CHECK_EXP(v);               \
			a->arr().push_back(*v);	    \
		}								\
	}									\
}
#endif

static void ift(Pu *L, Token *ptoken) //if
{
	NEXT_TOKEN;
	const __pu_value *t = exp(L);
    CHECK_EXP(t);
	bool loop = VALUE_IS_TRUE(*t);
	if (loop)
	{
		vm(L);
		if (L->isreturn.top() || L->isquit || L->isyield)
			return;

		PuType putp = TOKEN.type;
		if (putp == BREAK || putp == CONTINUE)
			return;
			
		while (TOKEN.type != END){
			L->cur_token = ptoken->optype;
			NEXT_TOKEN;
			ptoken = &(TOKEN);
		}
		NEXT_TOKEN;
	}
	else
	{
		L->cur_token = ptoken->optype;
		NEXT_TOKEN;
		PuType tp = TOKEN.type;
		switch (tp)
		{
		case ELIF:
			ift(L, &(TOKEN));
			break;
		case ELSE:
			NEXT_TOKEN;
			vm(L);
			NEXT_TOKEN;
			break;
		default:
			NEXT_TOKEN;
		}
	}
}


static void logc(Pu *L, __pu_value *&temp)
{
	PuType token_type = TOKEN.type;
	OperatorType op_type = TOKEN.optype;

	while (token_type == OP) // || &&
	{
		switch (op_type)
		{
		case OPT_OR:
			{
				NEXT_TOKEN;	// ||				
                __pu_value *t = NULL;
                CMP(L, t);
                CHECK_EXP(t);
                PU_NUMBER r = (PU_NUMBER)(*temp || *t);
                MAKE_TEMP_VALUE(temp);
                temp->SetType(BOOLEANT);
                temp->numVal() = r;
			}
			break;
		case OPT_AND:
			{
				NEXT_TOKEN;	// &&
                __pu_value *t = NULL;
                CMP(L, t);
                CHECK_EXP(t);
                PU_NUMBER r = (PU_NUMBER)(*temp && *t);
                MAKE_TEMP_VALUE(temp);
                temp->SetType(BOOLEANT);
                temp->numVal() = r;
			}
			break;
		default:
			return;
		}
		token_type = TOKEN.type;
		op_type = TOKEN.optype;
	}
}

static void cmp(Pu *L, __pu_value *&temp)
{
	PuType token_type = TOKEN.type;
	OperatorType op_type = TOKEN.optype;

	while (token_type == OP)
	{
		switch (op_type)
		{
		case OPT_GT:
			{
				NEXT_TOKEN;
				__pu_value *t = NULL;
				ADD(L, t);
                CHECK_EXP(t);
                PU_NUMBER r = (PU_NUMBER)(*temp > *t);
                MAKE_TEMP_VALUE(temp);
                temp->SetType(BOOLEANT);
				temp->numVal() = r;				
			}
			break;
		case OPT_LT:
			{
				NEXT_TOKEN;
				__pu_value *t = NULL;
				ADD(L, t);        
                CHECK_EXP(t);
                PU_NUMBER r = (PU_NUMBER)(*temp < *t);
                MAKE_TEMP_VALUE(temp);
                temp->SetType(BOOLEANT);
				temp->numVal() = r;	                
			}
			break;
		case OPT_EQ:
			{
				NEXT_TOKEN;
				__pu_value *t = NULL;
				ADD(L, t);          
                CHECK_EXP(t);
                PU_NUMBER r = (PU_NUMBER)(*temp == *t);
                MAKE_TEMP_VALUE(temp);
                temp->SetType(BOOLEANT);
				temp->numVal() = r;                
			}
			break;
		case OPT_GTA:
			{
				NEXT_TOKEN;
				__pu_value *t = NULL;
				ADD(L,t);     
                CHECK_EXP(t);
                PU_NUMBER r = (PU_NUMBER)(*temp >= *t);
                MAKE_TEMP_VALUE(temp);
                temp->SetType(BOOLEANT);
				temp->numVal() = r;                
			}
			break;
		case OPT_LTA:
			{
				NEXT_TOKEN;
				__pu_value *t = NULL;
				ADD(L, t);  
                CHECK_EXP(t);
                PU_NUMBER r = (PU_NUMBER)(*temp <= *t);
                MAKE_TEMP_VALUE(temp);
                temp->SetType(BOOLEANT);
				temp->numVal() = r;                
			}
			break;
		case OPT_NEQ:
			{
				NEXT_TOKEN;
				__pu_value *t = NULL;
				ADD(L, t);       
                CHECK_EXP(t);
                PU_NUMBER r = (PU_NUMBER)(*temp != *t);
                MAKE_TEMP_VALUE(temp);
                temp->SetType(BOOLEANT);
				temp->numVal() = r;                
			}
			break;
		default:
			return;
		}
		token_type = TOKEN.type;
		op_type = TOKEN.optype;
	}
}

static void add(Pu *L, __pu_value *&temp)
{
	PuType token_type = TOKEN.type;
	OperatorType op_type = TOKEN.optype;

	while (token_type == OP) // +-
	{
		switch (op_type)
		{
		case OPT_ADD:
			{
				NEXT_TOKEN;
				__pu_value *t = NULL;
				TERM(L, t);
                CHECK_EXP(t);
                __pu_value *r = NULL;
                MAKE_TEMP_VALUE(r);
                //r->SetType(NUM);
				*r = *temp + *t;
                temp = r;
			}
			break;
		case OPT_SUB:
			{
				NEXT_TOKEN;
				__pu_value *t = NULL;
				TERM(L,t);
                CHECK_EXP(t);
                __pu_value *r = NULL;
                MAKE_TEMP_VALUE(r);
				*r = *temp - *t;
                temp = r;
			}
			break;
		default:
			return;
		}

		token_type = TOKEN.type;
		op_type = TOKEN.optype;
	}
}

static void term(Pu *L, __pu_value *&temp)
{
	PuType tp = TOKEN.type;
	OperatorType nv = TOKEN.optype;

	while (tp == OP)  // */%
	{
		switch (nv)
		{
		case OPT_MUL:
			{
				NEXT_TOKEN;
				__pu_value *t = NULL;
				FACTOR(L,t);
                CHECK_EXP(t);
                __pu_value *r = NULL;
                MAKE_TEMP_VALUE(r);
				*r = *temp * *t;
                temp = r;
			}
			break;
		case OPT_DIV:
			{
				NEXT_TOKEN;
				__pu_value *t = NULL;
				FACTOR(L,t);
                CHECK_EXP(t);
                __pu_value *r = NULL;
                MAKE_TEMP_VALUE(r);
				*r = *temp / *t;
                temp = r;
			}
			break;
		case OPT_MOD:
			{
				NEXT_TOKEN;
				__pu_value *t = NULL;
				FACTOR(L, t);
                CHECK_EXP(t);
                __pu_value *r = NULL;
                MAKE_TEMP_VALUE(r);
				*r = *temp % *t;
                temp = r;
			}
			break;
		default:
			return;
		}

		tp = TOKEN.type;
		nv = TOKEN.optype;
	}
}


static void factor(Pu *L, __pu_value *&temp)
{
	PuType tp = TOKEN.type;
	OperatorType nv = TOKEN.optype;

	if (tp == OP)// ( [
	{
		switch (nv)
		{
		case OPT_LB:
			{
				NEXT_TOKEN;
				const __pu_value *exp_result = exp(L);
                CHECK_EXP(exp_result);
                MAKE_TEMP_VALUE(temp);
                *temp = *exp_result;
				tp = TOKEN.type;
				nv = TOKEN.optype;
				if (tp != OP || nv != OPT_RB)
				{
					error(L,1);
					return;
				}
				NEXT_TOKEN;
			}
			break;
		case OPT_LSB:
			get_array(L, temp);
			break;
        case OPT_LBR:
            get_map(L, temp);
            break;
        case OPT_NOT:
            get_value(L, temp);
		default:
			break;
		}
	}
	else
	{
		get_value(L, temp);
        CHECK_EXP(temp);
		tp = TOKEN.type;
		nv = TOKEN.optype;

		while (tp == OP)
		{
			switch (nv)
			{
			case OPT_LB:
				if (temp->type() == FUN || temp->type() == CFUN)
				{
					VarMap *old = L->upvalue;
					L->upvalue = 0;
					if (temp->userdata())
					{
						L->upvalue = ((_up_value*)temp->userdata())->vmap;
					}
                    
					callfunction(L, L->funclist[(int)temp->numVal()]);
					L->upvalue = old;
					MAKE_TEMP_VALUE(temp);
                    *temp = L->return_value;                    
					tp = TOKEN.type;
					nv = TOKEN.optype;
					CLEAR_RETURN;
					break;
				}
				else
				{
					// TODO: error
				}
				return;
			case OPT_LSB:
				if (temp->type() == ARRAY || temp->type() == STR)
				{
					get_arrref(L,temp);
                    CHECK_EXP(temp);
					NEXT_TOKEN;
					break;
				}
				else
				{
					// TODO: error
				}
				return;
			default:
				return;
			}
			tp = TOKEN.type;
			nv = TOKEN.optype;
		}
	}
}

static void callexternfunc(Pu *L, FuncPos &ps)
{
	pu_value argsstore[PU_MAXCFUNCARGNUM]={0};
	pu_value *args=NULL;
	ValueArr  vs;
	int i = 0;
	for (;;)// )
	{
		PuType tp = TOKEN.type;
		int nv = TOKEN.optype;

		if (tp == OP && nv == OPT_RB)break; //)
		if (tp == FINISH) {error(L,1);return;}
		if (tp == OP && nv == OPT_COM)
		{
			NEXT_TOKEN;
			continue;
		}
        const __pu_value *expresult = exp(L);
        CHECK_EXP(expresult);
		vs.push_back(*expresult);
		++i;
	}

	int arg_num = vs.size();

	if (arg_num > 0)
	{
		args = argsstore;
	}

	for (int j = 0; arg_num > 0 && j < i; ++j)
	{
		args[j] = &(vs[j]);
	}

	ps.pfunc(L, arg_num, args);
}

static void link_to_gc(Pu *L, _up_value *node)
{
	node->marked = false;
	if (L->gclink == 0)
	{
		L->gclink = node;
		return;
	}

	_up_value *p = L->gclink;
	while (p->next != 0)
	{
		p = p->next;
	}
	p->next = node;
}

static void callfunction(Pu *L, FuncPos &funcinfo)
{
	const FunArgs &args = funcinfo.argnames;
	
	if (funcinfo.start == -1)
	{
        NEXT_TOKEN;
		callexternfunc(L, funcinfo);
	}
	else
	{
        L->tail_optimize = false;
        PREV_TOKEN_N(2);
        if (L->token->type == RETURN)
        {
            L->tail_optimize = true;
        }
        NEXT_TOKEN_N(3);
		int i=0;
        PuVector<std::pair<const PuString*, const __pu_value*>, 8> arg_tmp_cnt;
		for (;;) // find )
		{
			PuType tp = TOKEN.type;
			int nv = TOKEN.optype;

            if (tp == OP && nv == OPT_RB) // )
            {
                break;
            }

			if (tp == OP && nv == OPT_COM) // ,
			{
				NEXT_TOKEN;
				continue;
			}
			const __pu_value *v = exp(L);
			CHECK_EXP(v);
            arg_tmp_cnt.push_back(std::make_pair(&args[i++], v));
		}

        NEXT_TOKEN;
        if (TOKEN.type != END)
        {
            L->tail_optimize = false;
        }
        PREV_TOKEN;

        if (!funcinfo.newvarmap && !L->tail_optimize)
        {
            funcinfo.newvarmap = new VarMap;
            L->varstack.push(funcinfo.newvarmap);
        }
        else
        {
            funcinfo.newvarmap = L->varstack.top();
        }

        for (auto &p : arg_tmp_cnt)
        {
            funcinfo.newvarmap->insert(*p.first, *p.second);
        }
		
        if (L->tail_optimize)
        {
            L->cur_token = funcinfo.start;
            NEXT_TOKEN;
            CLEAR_RETURN;
            funcinfo.newvarmap = 0;
            return;
        }

        _up_value *newupnode = new _up_value;
        newupnode->refcount = 1;
        link_to_gc(L, newupnode);
        newupnode->next = 0;
        newupnode->vmap = funcinfo.newvarmap;
        _up_value *oldcur = L->cur_nup;
        L->cur_nup = newupnode;
        L->varstack.push(funcinfo.newvarmap);
        L->callstack.push(L->cur_token);
        L->isreturn.push(false);
        L->cur_token = funcinfo.start;
        NEXT_TOKEN;        
        vm(L);
        L->isreturn.pop();
		L->cur_token = L->callstack.top();
		L->callstack.pop(); 
		L->varstack.pop();
		
		L->cur_nup = oldcur;
		newupnode->refcount -= 1;
		if (newupnode->refcount == 0)	
		{												
			newupnode->vmap = 0;											
		}
	}

	NEXT_TOKEN;
}

static void gotot(Pu *L)
{
	NEXT_TOKEN;

	PuType tp = TOKEN.type;
	OperatorType optype = TOKEN.optype;
	PuString &sv = TOKEN.value.strVal();

	if (tp == VAR)
	{
		if (optype >= 0)
		{
			L->cur_token = optype;
		}
		else
		{
			LabelMap::Bucket_T *it = L->labelmap.find(sv);
			if (it == 0)
			{
				error(L,16);
				return;
			}

			TOKEN.optype = (OperatorType)(it->value);
			L->cur_token = it->value;
		}

		NEXT_TOKEN;
		NEXT_TOKEN;
	}
}

static void procop(Pu *L)
{
	__pu_value *temp = NULL;

	PuType tp = TOKEN.type;
	OperatorType nv = TOKEN.optype;

	if (tp == OP && nv == OPT_LB) //(
	{
		FACTOR(L, temp);
	}
	else if (tp == OP && (nv == OPT_SUB || nv == OPT_ADD))// - +
	{
        MAKE_TEMP_VALUE(temp);
		temp->SetType(NUM);
		temp->numVal() = 0;
		add(L,temp);
        CHECK_EXP(temp);
	}
	else if (tp == OP && nv == OPT_NOT)// !
	{
        MAKE_TEMP_VALUE(temp);
        get_value(L,temp);
        CHECK_EXP(temp);
	}
    else
    {
        error(L, 29);
    }
}

// while
static void whilet(Pu *L, Token *ptoken)
{
	int while_begin = L->cur_token;
	OperatorType while_end = ptoken->optype;
	for(;;)
	{
		NEXT_TOKEN;
		const __pu_value *t = exp(L);
        CHECK_EXP(t);
		bool loop = VALUE_IS_TRUE(*t); // �ж�while�����Ƿ�����

		if (loop)
		{
			vm(L);

            if (L->isreturn.top() || L->isquit || L->isyield)
            {
                return;
            }

			PuType putp = TOKEN.type;
			switch (putp)
			{
			case END:
			case CONTINUE:
				L->cur_token = while_begin;
				continue;
			case BREAK:
				L->cur_token = while_end;
				NEXT_TOKEN;
				NEXT_TOKEN;
            default:
                break;
			}
			return;
		}
		L->cur_token = while_end;
		NEXT_TOKEN;
		NEXT_TOKEN;
		break;
	}
}

static void regfunc(Pu *L)
{
	PU_NUMBER fpos = (PU_NUMBER)TOKEN.optype;
	NEXT_TOKEN;
	if (TOKEN.type == VAR)
	{	
		__pu_value fv(L);
		fv.SetType(FUN);
		fv.numVal() = fpos;
		fv.userdata() = L->cur_nup;
 		if (L->cur_nup)
 		{
 			L->cur_nup->refcount++;
 		}
        __pu_value *got = reg_var(L, TOKEN.value.strVal());
        TOKEN.regvar = got;
        *got = fv;
		L->cur_token = L->funclist[(int)fv.numVal()].end;
		NEXT_TOKEN;
		return;
	}
	error(L, 29);
}

int vm(Pu *L)
{
	int ret = 0;
	for (;;)
	{
		PuType tp = TOKEN.type;
		if (tp == FINISH)
		{
			ret = 0;
			break;
		}

		if (L->isyield)
		{
			ret = -2;
			break;
		}
		
		if (L->isquit)
		{
			ret = -1;
			break;
		}

        if (!L->isreturn.empty() && L->isreturn.top())
        {
            ret = -3;
            break;
        }

		switch (tp)
		{
		case WHILE:
			whilet(L, &TOKEN);
			break;
		case IF:
			ift(L, &TOKEN);
			break;
		case OP:
			procop(L);
			break;
		case GOTO:
			gotot(L);
			break;
		case FUNCTION:
			regfunc(L);
			break;
		case RETURN:{
			NEXT_TOKEN;
			L->isreturn.top() = true;            
            const __pu_value *expresult = exp(L);
            if (L->tail_optimize)
            {
                L->tail_optimize = false;
                L->isreturn.top() = false;
                continue;
            }
            if (!expresult || expresult->type() == UNKNOWN)            
            {
                QUIT_SCRIPT;
                break;
            }
			L->return_value = *expresult;
			ret = 1;
            goto END_VM;}
		case INCLUDE:
			NEXT_TOKEN;
			NEXT_TOKEN;
			break;
		case ELSE:
		case ELIF:
		case BREAK:
		case CONTINUE:
		case END:
			ret = 1;
			goto END_VM;
		default:
			exp(L);
			break;
		}
	}
END_VM:
	DO_GC;
	return ret;
}



