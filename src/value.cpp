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

#include "value.h"
#include "error.h"
#include "def.h"
#include "util.h"

const __pu_var &__pu_var::operator +=(const __pu_var &x)
{
    *this = *this + x;
    return *this;
}

const __pu_var &__pu_var::operator -=(const __pu_var &x)
{
    *this = *this - x;
    return *this;
}

const __pu_var &__pu_var::operator *=(const __pu_var &x)
{
    *this = *this * x;
    return *this;
}
const __pu_var &__pu_var::operator /=(const __pu_var &x)
{
    *this = *this / x;
    return *this;
}

union d2ui
{
	struct u2
	{
		unsigned int x1;
		unsigned int x2;
	};

	u2 ui;
	int64_t li;
	double f;
};

unsigned int __pu_var::hash() const
{
    switch (type())
    {
    case STR:
        return strVal_->hash();
    case INTEGER:
        return (unsigned int)intVal();
    case NUM:
        return ((d2ui*)&numVal_)->ui.x1 + ((d2ui*)&numVal_)->ui.x2;
    default:
        break;
    }
    error(L, 31);
    return 0;
}

void __pu_var::destroy()
{
	switch (type())
	{
	case STR:
		delete strVal_;
		strVal_ = nullptr;
		break;
	case ARRAY:
		delete arr_;
		arr_ = nullptr;
		break;
	case FUN:
		delete upval_;
		upval_ = nullptr;
		break;
	case MAP:
		delete map_;
		map_ = nullptr;
		break;
	default:
		break;
	}
}

void __pu_var::build()
{
	switch (type())
	{
	case STR:
		strVal_ = new PuString;
		break;
	case MAP:
		map_ = new ValueMap;
		break;
	case ARRAY:
		arr_ = new ValueArr;
		break;
	case FUN:
		upval_ = new _scope(L, nullptr);
		break;
	default:
		break;
	}
}


__pu_var __pu_var::operator +(const __pu_var &x)
{
    __pu_var n(L);
    if (type() == INTEGER || x.type() == INTEGER)
    {
        n.SetType(INTEGER);
        n.intVal() = intVal() + x.intVal();
    }
    else if (type() == INTEGER || x.type() == NUM)
    {
        n.SetType(NUM);
        n.numVal() = intVal() + x.numVal();
    }
    else if (type() == NUM || x.type() == INTEGER)
    {
        n.SetType(NUM);
        n.numVal() = numVal() + x.intVal();
    }
    else if (type() == NUM || x.type() == NUM)
    {
        n.SetType(NUM);
        n.numVal() = numVal() + x.numVal();
    }
    else if (type() == STR && x.type() == STR)
    {
        n.SetType(STR);
        n.strVal() = strVal() + x.strVal();
    }
    else if (type() == STR && x.type() == INTEGER)
    {
        n.SetType(STR);
        char temp[64];
        PU_SNPRINTF(temp, sizeof(temp), "%lld", x.intVal());
        n.strVal() = strVal() + temp;
    }
    else if (type() == INTEGER && x.type() == STR)
    {
        n.SetType(STR);
        char temp[64];
        PU_SNPRINTF(temp, sizeof(temp), "%lld", intVal());
        n.strVal() = PuString(temp) + x.strVal();
    }
    else if (type() == STR && x.type() == NUM)
    {
        n.SetType(STR);
        char temp[64];

        if (PU_INT(x.numVal()) == x.numVal())
            PU_SNPRINTF(temp, sizeof(temp), "%.lf", x.numVal());
        else
            PU_SNPRINTF(temp, sizeof(temp), "%lf", x.numVal());

        n.strVal() = strVal() + temp;
    }
    else if (type() == NUM && x.type() == STR)
    {
        n.SetType(STR);
        char temp[64];
        if (PU_INT(numVal()) == numVal())
            PU_SNPRINTF(temp, sizeof(temp), "%.lf", numVal());
        else
            PU_SNPRINTF(temp, sizeof(temp), "%lf", numVal());
        n.strVal() = PuString(temp) + x.strVal();
    }
    else if (type() == STR && x.type() == BOOLEANT)
    {
        n.SetType(STR);
        if (x.intVal() == 1)
            n.strVal() += "true";
        else
            n.strVal() += "false";
    }
    else if (type() == BOOLEANT && x.type() == STR)
    {
        n.SetType(STR);
        PuString res = n.strVal();
        if (intVal() == 1)
            n.strVal() = "true";
        else
            n.strVal() = "false";

        n.strVal() += res;
    }
    else if (type() == ARRAY && x.type() != ARRAY)
    {
        n.SetType(ARRAY);
        ValueArr::iterator it = arr().begin();
        ValueArr::iterator ite = arr().end();
        while (it != ite)
        {
            n.arr().push_back(*it);
            ++it;
        }

        n.arr().push_back(x);
    }
    else
    {
        error(L, 14);
    }

    return n;
}

__pu_var __pu_var::operator -(const __pu_var &x)
{
    __pu_var n(L);
    if(type() == INTEGER || x.type() == INTEGER)
    {
        n.SetType(INTEGER);
        n.intVal() = intVal() - x.intVal();
    }
    else if(type() == NUM || x.type() == NUM)
    {
        n.SetType(NUM);
        n.numVal() = numVal() - x.numVal();
    }
    else if(type() == INTEGER || x.type() == NUM)
    {
        n.SetType(NUM);
        n.numVal() = intVal() - x.numVal();
    }
    else if(type() == NUM || x.type() == INTEGER)
    {
        n.SetType(NUM);
        n.numVal() = numVal() - x.intVal();
    }
    else
    {
        error(L, 9);
    }
    
    return n;
}

__pu_var __pu_var::operator /(const __pu_var &x)
{
    __pu_var n(L);
    if (x.type() == INTEGER)
    {
        if (x.intVal() == 0)
        {
            error(L, 35);
            return n;
        }
    }
    else if (x.type() == NUM)
    {
        if (x.numVal() == 0)
        {
            error(L, 35);
            return n;
        }
    }

    if (type() == INTEGER || x.type() == INTEGER)
    {
        n.SetType(INTEGER);
        n.intVal() = intVal() / x.intVal();
    }
    else if (type() == NUM || x.type() == NUM)
    {
        n.SetType(NUM);
        n.numVal() = numVal() / x.numVal();
    }
    else if (type() == INTEGER || x.type() == NUM)
    {
        n.SetType(NUM);
        n.numVal() = (PU_NUMBER)intVal() / x.numVal();
    }
    else if (type() == NUM || x.type() == INTEGER)
    {
        n.SetType(NUM);
        n.numVal() = numVal() / (PU_NUMBER)x.intVal();
    }
    else
    {
        error(L, 10);
    }
    
    return n;
}

__pu_var __pu_var::operator %(const __pu_var &x)
{
    __pu_var n(L);
    if (type() == INTEGER && x.type() == INTEGER)
    {
        n.SetType(INTEGER);
        n.intVal() = intVal() % x.intVal();
    }
    else
    {
        error(L, 11);
    }
    
    return n;
}

__pu_var __pu_var::operator *(const __pu_var &x)
{
    __pu_var n(L);
    if (type() == INTEGER || x.type() == INTEGER)
    {
        n.SetType(INTEGER);
        n.intVal() = intVal() * x.intVal();
    }
    else if (type() == NUM || x.type() == NUM)
    {
        n.SetType(NUM);
        n.numVal() = numVal() * x.numVal();
    }
    else if (type() == INTEGER || x.type() == NUM)
    {
        n.SetType(NUM);
        n.numVal() = intVal() * x.numVal();
    }
    else if (type() == NUM || x.type() == INTEGER)
    {
        n.SetType(NUM);
        n.numVal() = numVal() * x.intVal();
    }
    else if (type() == STR && x.type() == INTEGER)
    {
        n.SetType(STR);
        n.strVal() = "";
        int strvn = int(x.intVal());
        for (int k=0; k<strvn; ++k)
        {
            n.strVal() += strVal();
        }
    }
    else
    {
        error(L, 12);
    }
    
    return n;
}

int __pu_var::operator <(const __pu_var &x) const
{
    if (type() == INTEGER || x.type() == INTEGER)
    {
        return (intVal() < x.intVal())? 1 : 0;
    }
    else if (type() == INTEGER || x.type() == NUM)
    {
        return (intVal() < x.numVal())? 1 : 0;
    }
    else if (type() == NUM || x.type() == INTEGER)
    {
        return (numVal() < x.intVal())? 1 : 0;
    }
    else if (type() == NUM && x.type() == NUM)
    {
        return (numVal() < x.numVal())? 1 : 0;
    }
    else if(type() == STR && x.type() == STR)
    {
        return (strVal() < x.strVal())? 1 : 0;
    }
    else 
    {
        return ((int)VALUE_IS_TRUE(*this) < (int)VALUE_IS_TRUE(x))?1 : 0;
    }
}

int __pu_var::operator ==(const __pu_var &x) const
{
    if (type() == BOOLEANT && x.type() != BOOLEANT)
    {
        return (intVal() && VALUE_IS_TRUE(x))? 1 : 0;
    }
    else if (type() != BOOLEANT && x.type() == BOOLEANT)
    {
        return (x.intVal() && VALUE_IS_TRUE(*this))? 1 : 0;
    }
    if (type() == NUM && x.type() == INTEGER)
    {
        return (numVal() == x.intVal())? 1 : 0;
    }
    else if (type() == INTEGER && x.type() == NUM)
    {
        return (intVal() == x.numVal())? 1 : 0;
    }
    else if (type() != x.type())
    {
        return 0;
    }
    else if (type() == INTEGER || type() == BOOLEANT || type() == CPTR)
    {
        return (intVal() == x.intVal())?1:0;
    }
    else if (type() == NUM)
	{
		return (numVal() == x.numVal())?1:0;
	}
    else if (type() == STR)
    {
        return (strVal() == x.strVal())?1:0;
    }
    else if (type() == ARRAY)
    {
        return (arr() == x.arr())?1:0;
    }
    else if (type() == MAP)
    {
        return (map() == x.map()) ? 0 : 1;
    }
	else if (upval_ == x.upval_)
	{
		return 1;
	}
    
    return 0;
}

int __pu_var::operator >(const __pu_var &x) const
{
    return (*this < x || *this == x)? 0 : 1;
}

int __pu_var::operator >=(const __pu_var &x) const
{
    return (*this < x)? 0 : 1;
}

int __pu_var::operator <=(const __pu_var &x) const
{
    return ((*this < x) || (*this == x))? 1 : 0;
}

int __pu_var::operator !=(const __pu_var &x) const
{
    return (*this == x) ? 0 : 1;
}

int __pu_var::operator ||(const __pu_var &x) const
{
    return (VALUE_IS_TRUE(*this)) || (VALUE_IS_TRUE(x));
}

int __pu_var::operator &&(const __pu_var &x) const
{
    return (VALUE_IS_TRUE(*this)) && (VALUE_IS_TRUE(x));
}

void __pu_var::operator =(const __pu_var &x)
{
    SetType(x.type());
    switch (x.type())
    {
    case CORO: case INTEGER: case BOOLEANT: case CFUN: case CPTR:
        {            
            intVal() = x.intVal();
        }break;
    case NUM:
        {            
			numVal() = x.numVal();
        }break;
    case FUN:
        {            
			intVal() = x.intVal();
			up_value() = x.up_value();
        }break;
    case STR:
        {            
            strVal() = x.strVal();
        }break;
    case ARRAY:
        {            
            arr() = x.arr();
        }break;
    case MAP:
        {
            map() = x.map();
        }break;
	case FILEHANDLE:
        {
            pfile_ = x.pfile_;
        } break;
    default:
        break;
    }
	
    L = x.L;    
}

__pu_var::__pu_var()
	:L(nullptr)
	, createby_(PU_SYSTEM)
	, type_(UNKNOWN)
	, arr_(nullptr)
{
	debug(L, "waring call default __pi_var::construct");
}

__pu_var::~__pu_var()
{
    destroy();
}


__pu_var::_scope::_smap::_smap(Pu *L) : vmap_(new StrKeyMap)
, refcount(1)
, marked(false)
, L_(L)
, gc_(true)
{

}


__pu_var::_scope::_smap::_smap(Pu *L, StrKeyMap *__vmap) : vmap_(__vmap)
, refcount(1)
, marked(false)
, L_(L)
, gc_(false)
{

}

__pu_var::_scope::_smap::~_smap()
{
	if (gc_)
	{
		delete vmap_;
		vmap_ = nullptr;
	}
}
