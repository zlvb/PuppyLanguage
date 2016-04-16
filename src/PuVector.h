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

#ifndef _PuVector_H_
#define _PuVector_H_

#include "global.h"
#include <vector>

template<class ValueType>
struct VectorBuff : public std::vector<ValueType>
{
    VectorBuff():refcount(1)
    {}

    int refcount;
};

template<class ValueType>
struct PuVector
{
    typedef ValueType *iterator;

    PuVector():buff(0)
    {}

    PuVector(const PuVector<ValueType> &x):buff(0)
    {
        buff = x.buff;
        incref();
    }

    ~PuVector()
    {
        decref();
    }

    void clear()
    {
        decref();
    }

    int size() const
    {
        return (buff)? (int)buff->size() : 0;
    }

    void pop_back()
    {
        buff->pop_back();
    }

    const ValueType &operator[](int idx) const
    {
        return (*buff)[idx];
    }

    ValueType &operator[](int idx)
    {
        return (*buff)[idx];
    }

    iterator begin() const
    {
        return (buff)? &*buff->begin() : 0;
    }

    iterator end() const
    {
        iterator it = &buff->back();
        return (buff)? (++it) : 0;
    }

    ValueType &back()
    {
        return buff->back();
    }

    void incref()
    {
        if (buff) ++buff->refcount;
    }

    void decref()
    {
        if (buff && (--buff->refcount) == 0)
        {
            delete buff;
        }
        buff = 0;
    }

    void operator=(const PuVector<ValueType> &x)
    {
        if (buff != x.buff)
        {
            decref();
            buff = x.buff;
            incref();
        }
    }

    void init(const ValueType &v, int count)
    {
        for (int i=0; i < count; ++i)
        {
            push_back(v);
        }
    }

    void push_back(const ValueType &v)
    {
        if (!buff)
        {
            buff = new VectorBuff<ValueType>();
        }
        buff->push_back(v);
    }

    void erase(int idx)
    {
        buff->erase(buff->begin() + idx);
    }

    VectorBuff<ValueType> *buff;
};



#endif

