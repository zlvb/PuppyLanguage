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

template<class ValueType>
struct VectorBuff : public PuMemObj
{
	VectorBuff():_vb(0),size_(0),capacity(0),refcount(1)
	{}

	~VectorBuff()
	{
		delete[] _vb;
	}

	void expand(int size)
	{
		ValueType *newbuff = new ValueType[capacity + size];
		capacity += size;
		if (_vb)
		{
			for (int i=0; i<size; ++i)
			{
				newbuff[i] = _vb[i];
			}
			delete[] _vb;
		}
		_vb = newbuff;
	}

	ValueType *_vb;
	int size_;
	int capacity;
	int refcount;
};

template<class ValueType, int INIT_SIZE=32>
struct PuVector : public PuMemObj
{
	typedef ValueType* iterator;

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
		return (buff)? buff->size_ : 0;
	}

	void pop_back()
	{
		--buff->size_;
	}

	const ValueType &operator[](int idx) const
	{
		return buff->_vb[idx];
	}

    ValueType &operator[](int idx)
    {
        return buff->_vb[idx];
    }

	ValueType *begin() const
	{
		return (buff)? buff->_vb : 0;
	}

	ValueType *end() const
	{
		return (buff)? &(buff->_vb[buff->size_]) : 0;
	}

    ValueType &back()
    {
        return buff->_vb[buff->size_ - 1];
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

	void operator=(const PuVector<ValueType, INIT_SIZE> &x)
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
		buff->size_ = 0;
	}

	void push_back(const ValueType &v)
	{
		if (buff)
		{
			if (size() >= buff->capacity)
			{
				buff->expand(buff->capacity);
			}
		}
		else
		{
			buff = new VectorBuff<ValueType>;
			buff->expand(INIT_SIZE);
		}
		buff->_vb[buff->size_++] = v;
	}

    void erase(int idx)
    {
        (*this)[idx] = back();
        pop_back();
    }

	VectorBuff<ValueType> *buff;
};



#endif

