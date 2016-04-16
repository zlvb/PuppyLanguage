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

#include "travel.h"
#include "error.h"
#include "state.h"

static void term_trvel(Pu *L);
static void get_value_trvel(Pu *L);
static void factor_trvel(Pu *L);
static void cmp_trvel(Pu *L);
static void logic_trvel(Pu *L);
static void add_trvel(Pu *L);
static void callfunction_trvel(Pu *L);
static void get_arrref_trvel(Pu *L);
static void get_array_trvel(Pu *L);
static void get_map_trvel(Pu *L);

#define _CHT PuType tp = TOKEN.type;int nv = TOKEN.optype;
#define _TERM(L) {_CHT if(tp == OP && (nv==OPT_MUL||nv==OPT_DIV||nv==OPT_MOD))term_trvel(L);}
#define _ADD(L)     {_CHT if(tp == OP && (nv==OPT_ADD||nv==OPT_SUB))add_trvel(L);}
#define _CMP(L)     {_CHT if(tp == OP && (nv==OPT_GT||nv==OPT_LT||nv==OPT_EQ||nv==OPT_GTA||nv==OPT_LTA||nv==OPT_NEQ))cmp_trvel(L);}
#define _LOGC(L) {_CHT if(tp == OP && (nv==OPT_OR||nv==OPT_AND))logic_trvel(L); }
#define LOGC(L)     factor_trvel(L); _TERM(L) _ADD(L) _CMP(L) _LOGC(L)
#define CMP(L)     factor_trvel(L); _TERM(L) _ADD(L) _CMP(L)
#define ADD(L)     factor_trvel(L); _TERM(L) _ADD(L)
#define TERM(L)     factor_trvel(L); _TERM(L)
#define FACTOR(L)    factor_trvel(L);

struct AutoAddPath
{
    AutoAddPath(Pu *L, CONTROL_PATH eTp) :L_(L), eTp_(eTp){}
    ~AutoAddPath()
    {
        L_->control_flow->back().push_back(eTp_);
    }
    Pu *L_;
    CONTROL_PATH eTp_;
};

#define AddPath(eTp) AutoAddPath __aap(L, eTp);

static void _exp_trvel(Pu *L);
static void _doexp_trvel(Pu * L);

void _doexp_trvel(Pu * L)
{
    AddPath(CP_EXP);
    Token *ptoken = &TOKEN;
    do {
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

        LOGC(L);

    } while (false);

    PuType tp = TOKEN.type;
    OperatorType nv = TOKEN.optype;
    if (!(tp == OP
        && (nv == OPT_SET || nv == OPT_ADDS || nv == OPT_SUBS || nv == OPT_MULS || nv == OPT_DIVS)))
        return;

    NEXT_TOKEN;
    _exp_trvel(L);
}

void _exp_trvel(Pu *L)
{
    L->control_flow->push_back(PuVector<CONTROL_PATH>());
    _doexp_trvel(L);
    L->control_flow->pop_back();
}

void exp_control_flow_analyze(Pu *L)
{
    L->control_flow = new PuVector<PuVector<CONTROL_PATH> >();
    L->control_flow->push_back(PuVector<CONTROL_PATH>());
    _doexp_trvel(L);    
}

void get_map_trvel(Pu *L)
{

}

void term_trvel(Pu * L)
{
    AddPath(CP_TERM);
    PuType tp = TOKEN.type;
    OperatorType nv = TOKEN.optype;

    while (tp == OP)  // */%
    {
        switch (nv)
        {
        case OPT_MUL:
        case OPT_DIV:
        case OPT_MOD:
        {
            NEXT_TOKEN;
            FACTOR(L);
        }
        break;
        default:
            return;
        }

        tp = TOKEN.type;
        nv = TOKEN.optype;
    }
}

void get_value_trvel(Pu *L)
{
    AddPath(CP_VAL);
    PuType tp = TOKEN.type;
    OperatorType nv = TOKEN.optype;
    if (tp == OP && ((nv == OPT_ADD) || (nv == OPT_SUB) || (nv == OPT_NOT)))
    {
        NEXT_TOKEN;
        get_value_trvel(L);
    }
    else
    {
        NEXT_TOKEN;
    }    
}

void factor_trvel(Pu * L)
{
    AddPath(CP_FACTOR);
    PuType tp = TOKEN.type;
    OperatorType nv = TOKEN.optype;

    if (tp == OP)// ( [
    {
        switch (nv)
        {
        case OPT_LB:
        {
            NEXT_TOKEN;
            _exp_trvel(L);
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
            get_array_trvel(L);
            break;
        case OPT_LBR:
            get_map_trvel(L);
            break;
        case OPT_NOT:
            get_value_trvel(L);
        default:
            break;
        }
    }
    else
    {
        get_value_trvel(L);
        tp = TOKEN.type;
        nv = TOKEN.optype;

        while (tp == OP)
        {
            switch (nv)
            {
            case OPT_LB: // (
                callfunction_trvel(L);
                break;
            case OPT_LSB: // [
                get_arrref_trvel(L);
                NEXT_TOKEN;
                break;
            default:
                return;
            }
            tp = TOKEN.type;
            nv = TOKEN.optype;
        }
    }
}

void cmp_trvel(Pu * L)
{
    AddPath(CP_CMP);
    PuType token_type = TOKEN.type;
    OperatorType op_type = TOKEN.optype;

    while (token_type == OP)
    {
        switch (op_type)
        {
        case OPT_GT:
        case OPT_LT:
        case OPT_EQ:
        case OPT_GTA:
        case OPT_LTA:
        case OPT_NEQ:
        {
            NEXT_TOKEN;
            ADD(L);
        }
        break;        
        default:
            return;
        }
        token_type = TOKEN.type;
        op_type = TOKEN.optype;
    }
}

void logic_trvel(Pu * L)
{
    AddPath(CP_LOGIC);
    PuType token_type = TOKEN.type;
    OperatorType op_type = TOKEN.optype;

    while (token_type == OP) // || &&
    {
        switch (op_type)
        {
        case OPT_OR:
        case OPT_AND:
        {
            NEXT_TOKEN;            
            CMP(L);
        }
        break;
        default:
            return;
        }
        token_type = TOKEN.type;
        op_type = TOKEN.optype;
    }
}

void add_trvel(Pu *L)
{    
    AddPath(CP_ADD);
    PuType token_type = TOKEN.type;
    OperatorType op_type = TOKEN.optype;

    while (token_type == OP) // +-
    {
        switch (op_type)
        {
        case OPT_ADD:
        case OPT_SUB:
        {
            NEXT_TOKEN;
            TERM(L);            
        }
        break;
        default:
            return;
        }

        token_type = TOKEN.type;
        op_type = TOKEN.optype;
    }
}

void callfunction_trvel(Pu *L)
{
    AddPath(CP_CALL);
    NEXT_TOKEN;
    int i = 0;
    for (;;)// )
    {
        PuType tp = TOKEN.type;
        int nv = TOKEN.optype;

        if (tp == OP && nv == OPT_RB)
        {
            break; //)
        }
        if (tp == FINISH) 
        {
            error(L, 1); 
            return; 
        }
        if (tp == OP && nv == OPT_COM)
        {
            NEXT_TOKEN;
            continue;
        }
        _exp_trvel(L);        
        ++i;
    }
    NEXT_TOKEN;
}

void get_arrref_trvel(Pu *L)
{
    AddPath(CP_ARRREF);
    NEXT_TOKEN;
    _exp_trvel(L);

    PuType tp = TOKEN.type;
    OperatorType nv = TOKEN.optype;
    if (tp != OP || nv != OPT_RSB)
    {
        error(L, 4);
        return;
    }
}

void get_array_trvel(Pu *L)
{
    AddPath(CP_ARRAY);
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
            _exp_trvel(L);
        }
    }
}