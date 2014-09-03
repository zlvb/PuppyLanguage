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

template<class ValueType, int INIT_SIZE>
struct VectorBuff : public PuMemObj
{
	VectorBuff():_vb(0),cur(-1),count(0),refc(1)
	{}

	~VectorBuff()
	{
		delete[] _vb;
	}

	void expand_size(int size)
	{
		ValueType *newbuff = new ValueType[count + size];
		count += INIT_SIZE;
		if (_vb)
		{
			for (int i=0; i<=cur; ++i)
			{
				newbuff[i] = _vb[i];
			}
			delete[] _vb;
		}
		_vb = newbuff;
	}

	ValueType *_vb;
	int cur;
	int count;
	int refc;
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
		return (buff)? buff->cur+1 : 0;
	}

	void pop_back()
	{
		--buff->cur;
	}

	ValueType &operator[](int idx) const
	{
		return buff->_vb[idx];
	}

	ValueType *begin() const
	{
		return (buff)? buff->_vb : 0;
	}

	ValueType *end() const
	{
		return (buff)? &(buff->_vb[buff->cur+1]) : 0;
	}

    ValueType &back()
    {
        return buff->_vb[buff->cur];
    }

	void incref()
	{
		if (buff) ++buff->refc;
	}

	void decref()
	{
		if (buff && (--buff->refc) == 0)
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
		buff->cur = -1;
	}

	void push_back(const ValueType &v)
	{
		if (buff)
		{
			if (size() >= buff->count)
			{
				buff->expand_size(buff->count);
			}
		}
		else
		{
			buff = new VectorBuff<ValueType,INIT_SIZE>;
			buff->expand_size(INIT_SIZE);
		}
		buff->_vb[++buff->cur] = v;
	}

    void erase(int idx)
    {
        (*this)[idx] = back();
        pop_back();
    }

	VectorBuff<ValueType,INIT_SIZE> *buff;
};



#endif

