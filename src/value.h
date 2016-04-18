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
#include "config.h"
#include <unordered_map>
#include <stdio.h>
#include <assert.h>
#ifdef _MSC_VER
#pragma warning(disable:4127) // 判断条件为常量，比如：while(1)
#endif
struct Pu;


#define VALUE_IS_TRUE(v)                                        \
(                                                                \
    ((v).type() == BOOLEANT)?                                    \
        ((v).numVal() != 0)                                        \
                                                                \
    :(((v).type() == NUM)?                                        \
        ((v).numVal() != 0)                                        \
                                                                \
    :(((v).type() == CPTR)?                                     \
        (*(PU_INT*)&(v).numVal() != 0)                            \
                                                                \
    :(((v).type() == STR)?                                        \
        ((v).strVal().length() != 0)                            \
                                                                \
    :(((v).type() == ARRAY)?                                    \
        ((v).arr().size() != 0)                                    \
                                                                \
    :(((v).type() == MAP)?                                    \
        ((v).map().size() != 0)                                    \
                                                                \
    :(((v).type() == CORO)?                                        \
        ((v).numVal() != -1)                                    \
                                                                \
    :(((v).type() == FILEHANDLE)?                                \
        ((v).file() != 0 )                                  \
                                                                \
    :(((v).type() == FUNCTION)?                                    \
        (true):(false)                                            \
                                                                \
    ))))))))                                                       \
)

typedef enum PUVALUECREATEDBY{
    PU_SYSTEM,
    PU_USER
}PUVALUECREATEDBY;

struct __pu_var
{
	struct value_hash {
		size_t operator()(const __pu_var &__x) const
		{
			return __x.hash();
		}
	};

	struct value_eq {
		bool operator()(const __pu_var &__x, const __pu_var &__y) const
		{
			return (__x == __y) == 1;
		}
	};

	struct strptr_hash {
		size_t operator()(const std::string * const &__x) const
		{
			return std::hash<std::string>()(*__x);
		}
	};

	struct strptr_eq {
		bool operator()(const std::string * const &__x, const std::string * const &__y) const
		{
			return *__x == *__y;
		}
	};

    typedef std::unordered_map<const std::string*, __pu_var, strptr_hash, strptr_eq> StrKeyMap;
    typedef PuVector<__pu_var> ValueArr;
    typedef PuMap<__pu_var, __pu_var, value_hash, value_eq> ValueMap;

    struct _scope
    {
		struct _smap
		{
			_smap(Pu *L)
				: vmap_(new StrKeyMap)
				, refcount(1)
				, marked(false)
				, L_(L)
				, gc_(true)
			{
				add_to_gc();
			}
			_smap(Pu *L, StrKeyMap *__vmap)
				: vmap_(__vmap)
				, refcount(1)
				, marked(false)
				, L_(L)
				, gc_(false)
			{
			}
			~_smap()
			{
				if (gc_)
				{
					remove_from_gc();
					delete vmap_;
					vmap_ = nullptr;
				}
			}
			StrKeyMap *vmap_;
			int refcount;
			bool marked;
			Pu *L_;
			bool gc_;

		private:
			void add_to_gc();
			void remove_from_gc();
		};

		_scope(Pu *L)
			: base_(new _smap(L))			
		{ }
		_scope(Pu *L, StrKeyMap *__vmap)
			: base_(new _smap(L, __vmap))
		{ }
		_scope(const _scope &x)
			: base_(x.base_)
		{
			if (base_)
			{
				base_->refcount++;
			}
		}
		~_scope()
		{ 
			release();
		}
		_scope &operator=(const _scope &x)
		{
			if (base_ != x.base_)
			{
				release();
				base_ = x.base_;
				if (base_)
				{
					base_->refcount++;
				}
			}

			return *this;
		}
		_smap *base_;
	private:
		void release()
		{
			if (base_)
			{
				base_->refcount--;
				if (base_->refcount == 0)
				{
					delete base_;
				}
				base_ = nullptr;
			}
		}
    };

	__pu_var(Pu *_L)
		:L(_L)
		, createby_(PU_SYSTEM)
		, type_(UNKNOWN)
		, arr_(nullptr)
        , upval_(nullptr)
    {
    }

	__pu_var();

    __pu_var(const __pu_var &x)
        : L(x.L)
        , createby_(PU_SYSTEM)
        , type_(UNKNOWN)
        , arr_(nullptr)
        , upval_(nullptr)
    {
        *this = x;
    }

    ~__pu_var();

    void operator =(const __pu_var &x);
    __pu_var operator +(const __pu_var &x);
    __pu_var operator -(const __pu_var &x);
    __pu_var operator /(const __pu_var &x);
    __pu_var operator %(const __pu_var &x);
    __pu_var operator *(const __pu_var &x);
    int operator >(const __pu_var &x) const;
    int operator <(const __pu_var &x) const;
    int operator >=(const __pu_var &x) const;
    int operator <=(const __pu_var &x) const;
    int operator !=(const __pu_var &x) const;
    int operator ==(const __pu_var &x) const;
    int operator ||(const __pu_var &x) const;
    int operator &&(const __pu_var &x) const;
    const __pu_var &operator +=(const __pu_var &x);
    const __pu_var &operator -=(const __pu_var &x);
    const __pu_var &operator *=(const __pu_var &x);
    const __pu_var &operator /=(const __pu_var &x);
    
    Pu *L;
    PUVALUECREATEDBY createby_;

    PuType type() const 
    { 
        return type_; 
    }

    void SetType(PuType val) 
    { 
        if (val != type_)
        {
            destroy();
            type_ = val; 
            build();
        }
    }

    unsigned int hash() const;

    ValueArr& arr()
    {
#ifdef _DEBUG
        if (type() != ARRAY || !arr_)
        {
            assert(!"arr get value failed");
        }
#endif
        return *arr_;
    }

    const ValueArr& arr() const
    {
#ifdef _DEBUG
        if (type() != ARRAY || !arr_)
        {
            assert(!"arr get value failed");
        }
#endif
        return *arr_;
    }

    ValueMap& map()
    {
#ifdef _DEBUG
        if (type() != MAP || !map_)
        {
            assert(!"map get value failed");
        }
#endif
        return *map_;
    }

    const ValueMap& map() const
    {
#ifdef _DEBUG
        if (type() != MAP || !map_)
        {
            assert(!"map get value failed");
        }
#endif
        return *map_;
    }

    PuString& strVal()
    {
        return *strVal_;
    }

    const PuString& strVal() const
    {
#ifdef _DEBUG
        if (type() != STR || !strVal_)
        {
            assert(!"string get value failed");
        }
#endif
        return *strVal_;
    }

    PU_NUMBER& numVal()
    {
        return numVal_;
    }

    const PU_NUMBER& numVal() const
    {
#ifdef _DEBUG
        if (type() != FUN && type() != BOOLEANT && type() != NUM && type() != CORO && type() != CFUN && type() != CPTR)
        {
            assert(!"number get value failed");
        }
#endif
        return numVal_;
    }

	_scope &up_value()
    {    
        return *upval_;
    }

	const _scope &up_value() const
    {   
        return *upval_;
    }

	FILE *&file()
	{
		return pfile_;
	}

	FILE *const file() const
	{
		return pfile_;
	}

	void destroy();

private:

	void build();
    PuType type_;    
    union {
		PU_NUMBER numVal_;
		PU_INT    intVal_;
    };
	union {
		ValueArr  *arr_;
		ValueMap  *map_;
		PuString  *strVal_;
		FILE      *pfile_;
		_scope	  *upval_;
	};
};

typedef __pu_var::StrKeyMap StrKeyMap;
typedef __pu_var::ValueArr ValueArr;
typedef __pu_var::ValueMap ValueMap;
typedef __pu_var::_scope _scope;


#endif
