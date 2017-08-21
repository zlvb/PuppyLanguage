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
#pragma warning(disable:4127) // while(1)
#endif

#include "state.h"
#include "error.h"
#include "util.h"
#include "travel.h"
#include <vector>

int vm(Pu *L);
static void term(Pu *L, __pu_var *&temp);
static void factor(Pu *L, __pu_var *&temp);
static void cmp(Pu *L, __pu_var *&temp);
static void logic(Pu *L, __pu_var *&temp);
static void add(Pu *L, __pu_var *&temp);
static void callfunction(Pu *L, FuncPos &finfo);

#define _CHT PuType tp = TOKEN.type;int nv = TOKEN.optype;
#define _TERM(L,t)    {_CHT if(tp == OP && (nv==OPT_MUL||nv==OPT_DIV||nv==OPT_MOD))term(L,t);}
#define _ADD(L,t)    {_CHT if(tp == OP && (nv==OPT_ADD||nv==OPT_SUB))add(L,t);}
#define _CMP(L,t)    {_CHT if(tp == OP && (nv==OPT_GT||nv==OPT_LT||nv==OPT_EQ||nv==OPT_GTA||nv==OPT_LTA||nv==OPT_NEQ))cmp(L,t);}
#define _LOGC(L,t)    {_CHT if(tp == OP && (nv==OPT_OR||nv==OPT_AND))logic(L,t); }
#define LOGC(L,t)    factor(L,t); _TERM(L,t) _ADD(L,t) _CMP(L,t) _LOGC(L,t)
#define CMP(L,t)    factor(L,t); _TERM(L,t) _ADD(L,t) _CMP(L,t)
#define ADD(L,t)    factor(L,t); _TERM(L,t) _ADD(L,t)
#define TERM(L,t)    factor(L,t); _TERM(L,t)
#define FACTOR(L,t)    factor(L,t);
#define DO_GC if (L->tempvals.size() >= GC_LEVEL){ clear_temp(L);if(L->gccontainer.size() != 0 && L->varstack.size() == 1){gc(L);}}

extern void gc(Pu *L);
extern void clear_temp(Pu *L);
/*
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

#define CLEAR_RETURN                            \
L->return_value.SetType(NIL);

static __pu_var *get_varref(Pu *L, const std::string *name)
{
    StrKeyMap *pvarmap = L->varstack.top();
    auto ik = pvarmap->find(name);
    if (ik != pvarmap->end())
    {
        return &(ik->second);
    }

    pvarmap = L->varstack.bottom();
    ik = pvarmap->find(name);
    if (ik != pvarmap->end())
    {
        return &(ik->second);
    }

    return nullptr;
}

void get_var(Pu *L, const std::string *name, __pu_var *&v)
{
    StrKeyMap *pvarmap = L->varstack.top();
    auto ik = pvarmap->find(name);
    if (ik != pvarmap->end())
    {
        v = &ik->second;
        return;
    }

    pvarmap = L->varstack.bottom();
    ik = pvarmap->find(name);
    if (ik != pvarmap->end())
    {
        v = &ik->second;
        return;
    }

    if (L->up_scope.base_->vmap_ && L->up_scope.base_->vmap_ != pvarmap 
		&& L->up_scope.base_->vmap_ != L->varstack.top())
    {
        pvarmap = L->up_scope.base_->vmap_;
        ik = pvarmap->find(name);
        if (ik != pvarmap->end())
        {
            v = &ik->second;
            return;
        }
    }

    MAKE_TEMP_VALUE(v);
    error(L,7);
}

__pu_var *regvar(Pu *L, const std::string *varname)
{
    __pu_var *got = nullptr;
    StrKeyMap *top_varmap = L->varstack.top();
    auto it = top_varmap->find(varname);
    if (it != top_varmap->end())
    {
        got = &(it->second);
    }
    else 
    {
        StrKeyMap *global_varmap = L->varstack.bottom();
        it = global_varmap->find(varname);
        if (it != global_varmap->end())
        {
            got = &(it->second);
        }
        else
        {
            if (L->up_scope.base_->vmap_ && L->up_scope.base_->vmap_ != top_varmap
				&& L->up_scope.base_->vmap_ != global_varmap)
            {
                StrKeyMap *up_varmap = L->up_scope.base_->vmap_;
                it = up_varmap->find(varname);
                if (it != up_varmap->end())
                {
                    got = &(it->second);
                }
            }
        }
    }

    if (got == nullptr)
    {
        StrKeyMap *varmap = L->varstack.top();
        auto ret = varmap->insert(std::make_pair(varname, __pu_var(L)));
        got = &(ret.first->second);
        debug(L, "reg %s in %p", varname->c_str(), varmap);
    }

    return got;
}

static __pu_var *regfunc(Pu *L)
{
    PU_INT fpos = (PU_INT)TOKEN.optype;
    NEXT_TOKEN;
    if (TOKEN.type == VAR)
    {    
        __pu_var fv(L);
        fv.SetType(FUN);
        fv.intVal() = fpos;
        fv.up_value() = L->cur_scope;
        __pu_var *got = regvar(L, TOKEN.name);
        *got = fv;
        L->cur_token = L->funclist[(int)fv.intVal()].end;
        NEXT_TOKEN;
        return got;
    }
    error(L, 29);
    __pu_var *result = nullptr;
    MAKE_TEMP_VALUE(result);
    return result;
}

static const __pu_var *exp(Pu *L) 
{
    Token *ptoken = &TOKEN;

    if (ptoken->type == KW_FUNCTION) 
    {
        PU_INT fpos = (PU_INT)TOKEN.optype;
        NEXT_TOKEN;
        if (TOKEN.type == OP && TOKEN.optype == OPT_LB)
        {
            PREV_TOKEN;
            __pu_var *fv = nullptr;
            MAKE_TEMP_VALUE(fv);
            fv->SetType(FUN);
            fv->intVal() = fpos;
            fv->up_value() = L->cur_scope;
            L->cur_token = L->funclist[(int)fv->intVal()].end;
            NEXT_TOKEN;
            return fv;
        }
        else
        {
            error(L, 29);
            __pu_var *result = nullptr;
            MAKE_TEMP_VALUE(result);
            return result;
        }
    }

    __pu_var *temp = nullptr;
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
        __pu_var *result = nullptr;
        MAKE_TEMP_VALUE(result);
        return result;
    }

    NEXT_TOKEN;
    const __pu_var *t = exp(L);
    CHECK_EXP_RETURN(t, t);

    switch (nv)
    {
    case OPT_SET:{// =
		__pu_var *var_value = regvar(L, ptoken->name);
        assert( !(var_value->type() == UNKNOWN && temp != nullptr) );

        if (temp == nullptr)
        {
            temp = var_value;
        }
        
        *temp = *t;
        return temp;}
    case OPT_ADDS:// +=
        {
            __pu_var *pv = get_varref(L, ptoken->name);
            CHECK_EXP_RETURNERR(pv);
            return &pv->operator+=(*t);

        }
    case OPT_SUBS:// -=
        {
            __pu_var *pv = get_varref(L, ptoken->name);
            CHECK_EXP_RETURNERR(pv);
            return &pv->operator-=(*t);
        }
    case OPT_MULS:// *=
        {
            __pu_var *pv = get_varref(L, ptoken->name);
            CHECK_EXP_RETURNERR(pv);
            return &pv->operator*=(*t);
        }
    case OPT_DIVS:// /=
        {
            __pu_var *pv = get_varref(L, ptoken->name);
            CHECK_EXP_RETURNERR(pv);
            return &pv->operator/=(*t);
        }
    default:
        ;
    }

    return t;    
}

static void get_value(Pu *L, __pu_var *&temp)
{
    PuType tp = TOKEN.type;
    OperatorType nv = TOKEN.optype;
    if (tp == VAR)
    {
        const std::string *name = TOKEN.name;
        get_var(L, name, temp);
        NEXT_TOKEN;
    }
    else if (tp == OP && ((nv == OPT_ADD) || (nv == OPT_SUB) || (nv == OPT_NOT)))
    {
        if (nv == OPT_SUB) // -
        {
            NEXT_TOKEN;
            get_value(L, temp);
            CHECK_EXP(temp);
            if (temp->type() == NUM)
                temp->numVal() *= -1;
            else if (temp->type() == INTEGER)
                temp->intVal() *= -1;
            else
                error(L, 29);
        }
        else if (nv == OPT_NOT) // !
        {
            NEXT_TOKEN;
            get_value(L,temp);
            CHECK_EXP(temp);
            bool bbb = VALUE_IS_TRUE(*temp);
            temp->SetType(BOOLEANT);
            temp->intVal() = bbb ? 0 : 1;
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
        case KW_FALSE: temp->SetType(BOOLEANT);
            temp->intVal() = 0;
            break;
        case KW_TRUE: temp->SetType(BOOLEANT);
            temp->intVal() = 1;
            break;
        case INTEGER: temp->SetType(INTEGER);
            temp->intVal() = TOKEN.literal_value->intVal();
            break;
        case NUM: temp->SetType(NUM);
            temp->numVal() = TOKEN.literal_value->numVal();
            break;
        case STR: temp->SetType(STR);
            temp->strVal() = TOKEN.literal_value->strVal();
            break;
        default: 
            if (!L->isreturn.empty() && L->isreturn.top())
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

static void get_arrvalue(Pu *L, __pu_var *&arrref, const int idx)
{
    if (arrref->type() == ARRAY)
    {// arr
        if (idx > int(arrref->arr().size()-1) || idx<0)
        {// idx over bound
            error(L,5);
            MAKE_TEMP_VALUE(arrref);
            return;
        }

        arrref = &arrref->arr()[idx];
    }
    else if (idx >= 0)
    {// str
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

static void get_mapvalue(Pu *L, __pu_var *&mapref_out, const __pu_var &key)
{
    auto it = mapref_out->map().find(key);
    if (it == mapref_out->map().end())
    {
        error(L, 32);
        MAKE_TEMP_VALUE(mapref_out);
        return;
    }
    mapref_out = &it->second;
}

static void get_arrref(Pu *L, __pu_var *&temp)
{
    NEXT_TOKEN;
    const __pu_var *vidx = exp(L);
    CHECK_EXP(vidx);
    if (vidx->type() != INTEGER)
    {// only num can be idx
        error(L, 3);
        MAKE_TEMP_VALUE(temp);
        return;
    }

    PuType tp = TOKEN.type;
    OperatorType nv = TOKEN.optype;
    if (tp != OP || nv != OPT_RSB)
    {// array op must start with [ end with ]
        error(L,4);
        MAKE_TEMP_VALUE(temp);
        return;
    }

    int idx = (int)vidx->intVal();
    get_arrvalue(L, temp, idx);// get value by idx
}


static void get_mapref(Pu *L, __pu_var *&temp)
{
    NEXT_TOKEN;
    const __pu_var *key = exp(L);
    CHECK_EXP(key);
    if (key->type() != INTEGER && key->type() != NUM && key->type() != STR)
    {// only num or str can be key
        error(L, 31);
        MAKE_TEMP_VALUE(temp);
        return;
    }

    PuType tp = TOKEN.type;
    OperatorType nv = TOKEN.optype;
    if (tp != OP || nv != OPT_RSB)
    {// array op must start with [ end with ]
        error(L, 4);
        MAKE_TEMP_VALUE(temp);
        return;
    }

    get_mapvalue(L, temp, *key);// get value by key
}
PUAPI void pu_val2str(Pu *, const pu_var *p, char *b, int buffsize);
/*
k = exp(L);
match(':');
v = exp(L);
if (!match(','))
{
    if (!match('}'))
    {
        error(L);
    }
    else
    {
        break;
    }
}
else
{
    continue;
}
*/
static void get_map(Pu *L, __pu_var *&a)
{
    __pu_var v(L);
    MAKE_TEMP_VALUE(a);
    a->SetType(MAP);
    NEXT_TOKEN;
    for (;;)
    {
        PuType tp = TOKEN.type;
        OperatorType nv = TOKEN.optype;

        if (tp == OP)
        {
            if (nv == OPT_RBR) // }
            {
                NEXT_TOKEN;
                break;
            }
            else
            {
                error(L, 29);
				return;
            }
        }
        else
        {
            const __pu_var *key = exp(L);
            CHECK_EXP(key);
            tp = TOKEN.type;
            nv = TOKEN.optype;
            if (tp == OP && nv == OPT_COL) // :
            {
                NEXT_TOKEN;
            }
            else
            {
                error(L, 29);
				return;
            }
            const __pu_var *const_value = exp(L);
            CHECK_EXP(const_value);
            a->map().insert(*key, *const_value);

            if (TOKEN.type != OP)
            {
                error(L, 29);
				return;
            }

            if (TOKEN.optype != OPT_COM) // ,
            {
                if (TOKEN.optype != OPT_RBR) // }
                {
                    error(L, 29);
					return;
                }
                else
                {
                    NEXT_TOKEN;
                    break;
                }
            }
            else
            {
                NEXT_TOKEN;
            }
        }
    }
}

static void get_array(Pu *L, __pu_var *&a)
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
            const __pu_var *v = exp(L);
            CHECK_EXP(v);
            a->arr().push_back(*v);
        }
    }
}

static void if_stage(Pu *L, Token *ptoken) //if
{
    NEXT_TOKEN;
    const __pu_var *t = exp(L);
    CHECK_EXP(t);
    bool loop = VALUE_IS_TRUE(*t);
    if (loop)
    {
        vm(L);
        if ((!L->isreturn.empty() && L->isreturn.top()) || L->isquit || L->isyield)
        {
            return;
        }            

        PuType putp = TOKEN.type;
        if (putp == KW_BREAK || putp == KW_CONTINUE)
            return;
            
        while (TOKEN.type != KW_END){
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
        case KW_ELIF:
            if_stage(L, &(TOKEN));
            break;
        case KW_ELSE:
            NEXT_TOKEN;
            vm(L);
            NEXT_TOKEN;
            break;
        default:
            NEXT_TOKEN;
        }
    }
}


static void logic(Pu *L, __pu_var *&temp)
{
    PuType token_type = TOKEN.type;
    OperatorType op_type = TOKEN.optype;

    while (token_type == OP) // || &&
    {
        switch (op_type)
        {
        case OPT_OR:
            {
                NEXT_TOKEN;    // ||                
                __pu_var *t = nullptr;
                CMP(L, t);
                CHECK_EXP(t);
                PU_INT r = (PU_INT)(*temp || *t);
                MAKE_TEMP_VALUE(temp);
                temp->SetType(BOOLEANT);
                temp->intVal() = r;
            }
            break;
        case OPT_AND:
            {
                NEXT_TOKEN;    // &&
                __pu_var *t = nullptr;
                CMP(L, t);
                CHECK_EXP(t);
                PU_INT r = (PU_INT)(*temp && *t);
                MAKE_TEMP_VALUE(temp);
                temp->SetType(BOOLEANT);
                temp->intVal() = r;
            }
            break;
        default:
            return;
        }
        token_type = TOKEN.type;
        op_type = TOKEN.optype;
    }
}

static void cmp(Pu *L, __pu_var *&temp)
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
                __pu_var *t = nullptr;
                ADD(L, t);
                CHECK_EXP(t);
                PU_INT r = (PU_INT)(*temp > *t);
                MAKE_TEMP_VALUE(temp);
                temp->SetType(BOOLEANT);
                temp->intVal() = r;
            }
            break;
        case OPT_LT:
            {
                NEXT_TOKEN;
                __pu_var *t = nullptr;
                ADD(L, t);       
                CHECK_EXP(t);
                PU_INT r = (PU_INT)(*temp < *t);
                MAKE_TEMP_VALUE(temp);
                temp->SetType(BOOLEANT);
                temp->intVal() = r;
            }
            break;
        case OPT_EQ:
            {
                NEXT_TOKEN;
                __pu_var *t = nullptr;
                ADD(L, t);      
                CHECK_EXP(t);
                PU_INT r = (PU_INT)(*temp == *t);
                MAKE_TEMP_VALUE(temp);
                temp->SetType(BOOLEANT);
                temp->intVal() = r;
            }
            break;
        case OPT_GTA:
            {
                NEXT_TOKEN;
                __pu_var *t = nullptr;
                ADD(L,t);     
                CHECK_EXP(t);
                PU_INT r = (PU_INT)(*temp >= *t);
                MAKE_TEMP_VALUE(temp);
                temp->SetType(BOOLEANT);
                temp->intVal() = r;
            }
            break;
        case OPT_LTA:
            {
                NEXT_TOKEN;
                __pu_var *t = nullptr;
                ADD(L, t);  
                CHECK_EXP(t);
                PU_INT r = (PU_INT)(*temp <= *t);
                MAKE_TEMP_VALUE(temp);
                temp->SetType(BOOLEANT);
                temp->intVal() = r;
            }
            break;
        case OPT_NEQ:
            {
                NEXT_TOKEN;
                __pu_var *t = nullptr;
                ADD(L, t);      
                CHECK_EXP(t);
                PU_INT r = (PU_INT)(*temp != *t);
                MAKE_TEMP_VALUE(temp);
                temp->SetType(BOOLEANT);
                temp->intVal() = r;
            }
            break;
        default:
            return;
        }
        token_type = TOKEN.type;
        op_type = TOKEN.optype;
    }
}

static void add(Pu *L, __pu_var *&temp)
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
                __pu_var *t = nullptr;
                TERM(L, t);
                CHECK_EXP(t);
                __pu_var *r = nullptr;
                MAKE_TEMP_VALUE(r);
                *r = *temp + *t;
                temp = r;
            }
            break;
        case OPT_SUB:
            {
                NEXT_TOKEN;
                __pu_var *t = nullptr;
                TERM(L,t);
                CHECK_EXP(t);
                __pu_var *r = nullptr;
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

static void term(Pu *L, __pu_var *&temp)
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
                __pu_var *t = nullptr;
                FACTOR(L,t);
                CHECK_EXP(t);
                __pu_var *r = nullptr;
                MAKE_TEMP_VALUE(r);
                *r = *temp * *t;
                temp = r;
            }
            break;
        case OPT_DIV:
            {
                NEXT_TOKEN;
                __pu_var *t = nullptr;
                FACTOR(L,t);
                CHECK_EXP(t);
                __pu_var *r = nullptr;
                MAKE_TEMP_VALUE(r);
                *r = *temp / *t;
                temp = r;
            }
            break;
        case OPT_MOD:
            {
                NEXT_TOKEN;
                __pu_var *t = nullptr;
                FACTOR(L, t);
                CHECK_EXP(t);
                __pu_var *r = nullptr;
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

static void factor(Pu *L, __pu_var *&temp)
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
                const __pu_var *exp_result = exp(L);
                CHECK_EXP(exp_result);
                MAKE_TEMP_VALUE(temp);
                *temp = *exp_result;
                tp = TOKEN.type;
                nv = TOKEN.optype;
                if (tp != OP || nv != OPT_RB)
                {
                    error(L, 1);
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
    }

    tp = TOKEN.type;
    nv = TOKEN.optype;
    while (tp == OP)
    {
        switch (nv)
        {
        case OPT_LB:
            if (temp->type() == FUN || temp->type() == CFUN)
            {
                _scope old = L->up_scope;
                if (temp->type() == FUN)
                {
                    L->up_scope = temp->up_value();
                }
                callfunction(L, L->funclist[(int)temp->intVal()]);
                if (temp->type() == FUN)
                {
                    L->up_scope = old;
                }
                MAKE_TEMP_VALUE(temp);
                *temp = L->return_value;                    
                tp = TOKEN.type;
                nv = TOKEN.optype;
                CLEAR_RETURN;
                break;
            }
            else
            {
                error(L, 29);
                return;
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
            else if (temp->type() == MAP)
            {
                get_mapref(L, temp);
                CHECK_EXP(temp);
                NEXT_TOKEN;
                break;
            }
            else
            {
                error(L, 29);
                return;
            }
            return;
        default:
            return;
        }
        tp = TOKEN.type;
        nv = TOKEN.optype;
    }
}

static void callexternfunc(Pu *L, FuncPos &ps)
{
    debug(L, "call function at %p", ps.pfunc); 
    pu_var argsstore[PU_MAXCFUNCARGNUM]={0};
    pu_var *args=nullptr;
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
        const __pu_var *expresult = exp(L);
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

extern void add_to_gc(Pu *L, _scope::_smap *vmap);
extern void remove_from_gc(Pu *L, _scope::_smap *vmap);
static void callfunction(Pu *L, FuncPos &funcinfo)
{
    if (funcinfo.start >= 0)
    {
        debug(L, "call function at %d", funcinfo.start);
    }
    
    const FunArgs &args = funcinfo.argnames;
    
    if (funcinfo.start == -1)
    {
        NEXT_TOKEN;
        callexternfunc(L, funcinfo);
    }
    else
    {        
        int return_end = -1;
        if (!L->isreturn.empty())
        {
            int return_pos = L->isreturn.top();            
            if (return_pos > 0)
            {
                const Token &return_token = L->tokens[return_pos];
                std::vector<CONTROL_PATH> &trvel_path = (*return_token.control_flow)[0];
                CONTROL_PATH eLastStep = trvel_path[trvel_path.size() - 3];
                if (return_token.exp_end != -1 && eLastStep == CP_CALL)
                {
                    return_end = return_token.exp_end;
                }
            }

        }
        NEXT_TOKEN;
		
        int i = 0;
        std::vector<std::pair<const std::string*, const __pu_var*> > arg_tmp_cnt;
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
            const __pu_var *v = exp(L);
            CHECK_EXP(v);
            arg_tmp_cnt.push_back(std::make_pair(args[i++], v));
        }

		if (arg_tmp_cnt.size() != args.size())
		{
			error(L, 34);
			return;
		}

		_scope *new_scope = nullptr;
        L->tail_optimize = return_end == L->cur_token + 1;
        if (L->tail_optimize)
        {
            debug(L, "%s", "tail optimize");
        }

        if (!L->tail_optimize)
        {
			new_scope = new _scope(L);
			add_to_gc(L, new_scope->base_);
        }
        else
        {
			new_scope = &L->cur_scope;
        }

        for (auto &p : arg_tmp_cnt)
        {
			new_scope->base_->vmap_->operator[](p.first) = *p.second;
        }
        
        if (L->tail_optimize)
        {
            L->cur_token = funcinfo.start;
            NEXT_TOKEN;
            CLEAR_RETURN;
            debug(L, "start tail call");
            return;
        }

        _scope old_scope = L->cur_scope;
        L->cur_scope = *new_scope;
        L->varstack.push(new_scope->base_->vmap_);
        debug(L, "cur_scop = %p", L->varstack.top());
        L->callstack.push(L->cur_token);
        L->isreturn.push(0);
        L->cur_token = funcinfo.start;
        NEXT_TOKEN;        
        vm(L);
        L->isreturn.pop();
        L->cur_token = L->callstack.top();
        L->callstack.pop(); 
        L->varstack.pop();
        debug(L, "cur_scop = %p", L->varstack.top());
        L->cur_scope = old_scope;
		if (new_scope->base_->refcount == 1)
		{
			remove_from_gc(L, new_scope->base_);
		}
		delete new_scope;
		new_scope = nullptr;
    }

    NEXT_TOKEN;
}

static void gotot(Pu *L)
{
    NEXT_TOKEN;

    PuType tp = TOKEN.type;
    int endpos = TOKEN.endpos;
   
    if (tp == VAR)
    {
        if (endpos >= 0)
        {
            L->cur_token = endpos;
        }
        else
        {
            const std::string &sv = *TOKEN.name;
            auto it = L->labelmap.find(sv);
            if (it == L->labelmap.end())
            {
                error(L,16);
                return;
            }

            TOKEN.endpos = it->second;
            L->cur_token = it->second;
        }

        NEXT_TOKEN;
        NEXT_TOKEN;
    }
}

static void procop(Pu *L)
{
    __pu_var *temp = nullptr;
    PuType tp = TOKEN.type;
    OperatorType nv = TOKEN.optype;

    if (tp == OP && nv == OPT_LB) //(
    {
        FACTOR(L, temp);
    }
    else if (tp == OP && (nv == OPT_SUB || nv == OPT_ADD))// - +
    {
        MAKE_TEMP_VALUE(temp);
        temp->SetType(INTEGER);
        temp->intVal() = 0;
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
        return;
    }
}
static void for_in_integer(Pu *L, const __pu_var *t, __pu_var *v, __pu_var *k, int for_begin, int for_end)
{
    bool breaked = false;
    for (int i = 0; !breaked && i < t->intVal(); i++)
    {
        k->intVal() = i;
        *v = *k;

        vm(L);
        if ((!L->isreturn.empty() && L->isreturn.top()) || L->isquit || L->isyield)
        {
            return;
        }

        PuType putp = TOKEN.type;
        switch (putp)
        {
        case KW_END:
        case KW_CONTINUE:
            SET_TOKEN(for_begin);
            continue;
        case KW_BREAK:
            breaked = true;
            break;
        default:
            break;
        }
    }

    L->cur_token = for_end;
    NEXT_TOKEN_N(2);
}

static void for_in_arr_str(Pu *L, const __pu_var *t, __pu_var *v, __pu_var *k, int for_begin, int for_end)
{
    if (t->type() == STR)
    {
        v->SetType(STR);
    }

    bool breaked = false;
    for (int i = 0; !breaked && i < t->arr().size(); i++)
    {
        k->intVal() = i;
        if (t->type() == ARRAY)
        {
            *v = t->arr()[i];
        }
        else
        {
            char buff[2] = { 0 };
            buff[0] = t->strVal()[i];
            v->strVal() = buff;
        }

        vm(L);
        if ((!L->isreturn.empty() && L->isreturn.top()) || L->isquit || L->isyield)
        {
            return;
        }

        PuType putp = TOKEN.type;
        switch (putp)
        {
        case KW_END:
        case KW_CONTINUE:
            SET_TOKEN(for_begin);
            continue;
        case KW_BREAK:
            breaked = true;
            break;
        default:
            break;
        }
    }

    L->cur_token = for_end;
    NEXT_TOKEN_N(2);
}

static void for_in_map(Pu *L, const __pu_var *t, __pu_var *v, __pu_var *k, int for_begin, int for_end)
{
    bool breaked = false;
    for (auto it = t->map().begin(); !breaked && it != t->map().end(); ++it)
    {
        *k = it->first;
        *v = it->second;

        vm(L);
        if ((!L->isreturn.empty() && L->isreturn.top()) || L->isquit || L->isyield)
        {
            return;
        }

        PuType putp = TOKEN.type;
        switch (putp)
        {
        case KW_END:
        case KW_CONTINUE:
            SET_TOKEN(for_begin);
            continue;
        case KW_BREAK:
            breaked = true;
            break;
        default:
            break;
        }
    }

    L->cur_token = for_end;
    NEXT_TOKEN_N(2);
}

// for
/*
for k,v in exp
*/
static void for_stage(Pu *L, Token *ptoken)
{
    int for_end = ptoken->endpos;
    NEXT_TOKEN_N(4);
    if (TOKEN.type != KW_IN)
    {
        error(L, 29);
        return;
    }
    
    PREV_TOKEN_N(3);
    __pu_var *k = regvar(L, TOKEN.name);
    k->SetType(INTEGER);
    NEXT_TOKEN_N(2);
    __pu_var *v = regvar(L, TOKEN.name);
    NEXT_TOKEN_N(2);
    const __pu_var *t = exp(L);
    CHECK_EXP(t);

    int for_begin = L->cur_token;
    if (t->type() == ARRAY || t->type() == STR)
    {
        for_in_arr_str(L, t, v, k, for_begin, for_end);
    }
    else if (t->type() == MAP)
    {
        for_in_map(L, t, v, k, for_begin, for_end);
    }
    else if (t->type() == INTEGER)
    {
        for_in_integer(L, t, v, k, for_begin, for_end);
    }
    else
    {
        error(L, 36);
    }
}

// while
static void while_stage(Pu *L, Token *ptoken)
{
    int while_begin = L->cur_token;
    int while_end = ptoken->endpos;
    for(;;)
    {
        NEXT_TOKEN;
        const __pu_var *t = exp(L);
        CHECK_EXP(t);
        bool loop = VALUE_IS_TRUE(*t);

        if (loop)
        {
            vm(L);

            if ((!L->isreturn.empty() && L->isreturn.top()) || L->isquit || L->isyield)
            {
                return;
            }

            PuType putp = TOKEN.type;
            switch (putp)
            {
            case KW_END:
            case KW_CONTINUE:
                L->cur_token = while_begin;
                continue;
            case KW_BREAK:
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
        case KW_WHILE:
            while_stage(L, &TOKEN);
            break;
        case KW_IF:
            if_stage(L, &TOKEN);
            break;
        case OP:
            procop(L);
            break;
        case KW_VAR: {
            NEXT_TOKEN;
            regvar(L, TOKEN.name);
            }
            break;
        case KW_GOTO:
            gotot(L);
            break;
        case KW_FUNCTION:
            regfunc(L);
            break;
        case KW_RETURN:{
			if (L->isreturn.empty())
			{
				error(L, 33);
				break;
			}
            L->isreturn.top() = L->cur_token - 1;
            Token *return_token = &L->tokens[L->cur_token - 1];
            NEXT_TOKEN;                  
            if (return_token->exp_end == -1)
            {
                Token *old_token = L->token;
                int old = L->cur_token;                        
                exp_control_flow_analyze(L);    
                return_token->control_flow = L->control_flow;
                L->control_flow = nullptr;
                int exp_end = L->cur_token;
                L->cur_token = old;
                return_token->exp_end = exp_end;
                L->token = old_token;
            }
            const __pu_var *expresult = exp(L);
            CHECK_EXP_CONTINUE(expresult);
            if (L->tail_optimize)
            {
                L->tail_optimize = false;
                L->isreturn.top() = 0;
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
        case KW_FOR:
            for_stage(L, &TOKEN);
            break;
        case KW_INCLUDE:
            NEXT_TOKEN;
            NEXT_TOKEN;
            break;
        case KW_ELSE:
        case KW_ELIF:
        case KW_BREAK:
        case KW_CONTINUE:
        case KW_END:
            ret = 1;
            goto END_VM;
        default:
            const __pu_var *er = exp(L);
            CHECK_EXP_CONTINUE(er);
            break;
        }
    }
END_VM:
    DO_GC;
    return ret;
}
