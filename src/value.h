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

#ifndef __VM_VALUE__
#define __VM_VALUE__

#include "PuMap.h"
#include "PuString.h"
#include "PuVector.h"
#include <stdio.h>
#include <assert.h>
#ifdef _MSC_VER
#pragma warning(disable:4127) // 判断条件为常量，比如：while(1)
#endif
struct Pu;

enum PuType
{
	WHILE,
	END,
	IF,
	ELSE,
	BREAK,
	GOTO,
	FUNCTION,
	RETURN,
	INCLUDE,
	CONTINUE,
	ELIF,
	NIL,
	FALSEK,
	TRUEK,
	OP,
	VAR,
	//---------------
	NUM,
	STR,
	ARRAY,
    MAP,
	CORO,
	FILEHANDLE,
	BOOLEANT,
	FUN,
    CFUN,
	//---------------
	LABEL,
	FINISH,
	UNKNOWN
};

#define GET_VARREF(L,name,t)									\
(																\
	(__pu_value*)( ((t) && (t)->var && L->varstack.size()==1)?	\
		(t)->var												\
	:															\
		get_varref(L,(name),t) )								\
)


#define GET_VAR(L,name,v,t)					\
{											\
	if (t&&t->var && L->varstack.size()==1)	\
		v=*(__pu_value*)(t->var);			\
	else									\
		get_var(L,(name),(v),(t));			\
}	

#ifndef _DEBUG
#define DECVMAP_REF(userdata)						\
if (userdata)										\
{													\
	((_up_value*)(userdata))->refcount--;			\
	if (((_up_value*)(userdata))->refcount == 0)	\
	{												\
		delete ((_up_value*)(userdata))->vmap;		\
		((_up_value*)(userdata))->vmap = 0;			\
		userdata = 0;								\
	}												\
}
#else
void DECVMAP_REF(void *&userdata);
#endif


#define SET_VAR(L,varname,new_value,vart)			\
{													\
	if (vart && vart->var && L->varstack.size()==1)	\
		(*(__pu_value*)(vart->var) = (new_value));	\
	else											\
		set_var(L,(varname),(new_value),(vart));	\
}

#define VALUE_IS_TRUE(v)										\
(																\
	((v).type() == BOOLEANT)?									\
		((v).numVal() != 0)										\
																\
	:(((v).type() == NUM)?										\
		((v).numVal() != 0)										\
																\
	:(((v).type() == STR)?										\
		((v).strVal().length() != 0)							\
																\
	:(((v).type() == ARRAY)?									\
		((v).arr().size() != 0)									\
																\
	:(((v).type() == CORO)?										\
		((v).numVal() != -1)									\
																\
	:(((v).type() == FILEHANDLE)?								\
		((v).userdata() != 0 )                                  \
																\
	:(((v).type() == FUNCTION)?									\
		(true):(false)											\
																\
	))))))														\
)

typedef PuVector<__pu_value> ValueArr;
typedef PuMap<PuString, __pu_value, 10> Var;
typedef PuVector<Token> TokenList;
typedef enum PUVALUECREATEDBY{
	PU_SYSTEM,
	PU_USER
}PUVALUECREATEDBY;

struct _up_value
{
	Var *vmap;
	int refcount;
	bool marked;
	_up_value *next;
};

struct __pu_value
{
    typedef Var PuValMap;
	inline __pu_value(Pu *_L)
        :L(_L)
        ,createby(PU_SYSTEM)
        ,_type(UNKNOWN)
        ,_arr(0)
        ,_userdata(0)
	{
	}

	inline __pu_value()
        :L()
        ,createby(PU_SYSTEM)
        ,_type(UNKNOWN)
        ,_arr(0)
        ,_userdata(0)
	{
	}

	__pu_value(const __pu_value &x)
        :L(x.L)
        ,createby(PU_SYSTEM)
        ,_type(UNKNOWN)
        ,_arr(0)
        ,_userdata(0)
	{
		*this = x;
	}

	~__pu_value();

	void operator =(const __pu_value &x);
	__pu_value operator +(const __pu_value &x);
	__pu_value operator -(const __pu_value &x);
	__pu_value operator /(const __pu_value &x);
	__pu_value operator %(const __pu_value &x);
	__pu_value operator *(const __pu_value &x);
	int operator >(const __pu_value &x) const;
	int operator <(const __pu_value &x) const;
	int operator >=(const __pu_value &x) const;
	int operator <=(const __pu_value &x) const;
	int operator !=(const __pu_value &x) const;
	int operator ==(const __pu_value &x) const;
	int operator ||(const __pu_value &x) const;
	int operator &&(const __pu_value &x) const;
	const __pu_value & operator +=(const __pu_value &x);
	const __pu_value & operator -=(const __pu_value &x);
	const __pu_value & operator *=(const __pu_value &x);
	const __pu_value & operator /=(const __pu_value &x);
    
    Pu *L;
    PUVALUECREATEDBY createby;

    PuType type() const 
    { 
        return _type; 
    }

    void SetType(PuType val) 
    { 
        if (val != _type)
        {
            destroy();
            _type = val; 
            build();
        }
    }

    ValueArr& arr()
    {
#ifdef _DEBUG
        if (type() != ARRAY || !_arr)
        {
            assert(!"arr get value failed");
        }
#endif
        return *_arr;
    }

    const ValueArr& arr() const
    {
#ifdef _DEBUG
        if (type() != ARRAY || !_arr)
        {
            assert(!"arr get value failed");
        }
#endif
        return *_arr;
    }

    PuValMap& map()
    {
#ifdef _DEBUG
        if (type() != MAP || !_map)
        {
            assert(!"map get value failed");
        }
#endif
        return *_map;
    }

    const PuValMap& map() const
    {
#ifdef _DEBUG
        if (type() != MAP || !_map)
        {
            assert(!"map get value failed");
        }
#endif
        return *_map;
    }

    PuString& strVal()
    {
        return *_strVal;
    }

    const PuString& strVal() const
    {
#ifdef _DEBUG
        if (type() != STR || !_strVal)
        {
            assert(!"string get value failed");
        }
#endif
        return *_strVal;
    }

    PU_NUMBER& numVal()
    {
        return _numVal;
    }

    const PU_NUMBER& numVal() const
    {
#ifdef _DEBUG
        if (type() != FUN && type() != BOOLEANT && type() != NUM && type() != CORO && type() != CFUN)
        {
            assert(!"number get value failed");
        }
#endif
        return _numVal;
    }

    void *&userdata()
    {    
        return _userdata;
    }

    void *const userdata() const
    {   
        return _userdata;
    }

private:
    void destroy()
    {
        switch (type())
        {
        case STR:
            RELEASE_BUFF(strVal());
            break;
        case ARRAY:
            arr().decref();
            break;
        case FUN:
            DECVMAP_REF(userdata());
            break;
        default:
            break;
        }
    }

    void build()
    {
        switch (type())
        {
        case STR:
            _strVal = new PuString;
            break;
        case MAP:
            _map = new PuValMap;
            break;
        case ARRAY:
            _arr = new ValueArr;
            break;
        default:
            break;
        }
    }
    PuType _type;    
    union {
	PU_NUMBER _numVal;
	ValueArr *_arr;
    PuValMap *_map;			
	PuString *_strVal;	
    };
    void *_userdata;
};


#endif
