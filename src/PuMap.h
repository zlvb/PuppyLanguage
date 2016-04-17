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


#ifndef __PU_MAP__
#define __PU_MAP__

#include "PuVector.h"
#include <unordered_map>

template<class Key_T, class Value_T, class HashFunc, class EqFunc>
struct MapRef : public std::unordered_map<Key_T, Value_T, HashFunc, EqFunc>
{
    MapRef() :refcount(1) {};
    int refcount;
};

template<class Key_T, class Value_T, class HashFunc, class EqFunc>
class PuMap
{
public:
	typedef ::MapRef<Key_T, Value_T, HashFunc, EqFunc> MapRef;
    typedef typename MapRef::iterator iterator;
    PuMap():mapptr_(0)
    { }

	PuMap(const PuMap &r) :mapptr_(r.mapptr_)
	{
		if (mapptr_)
		{
			mapptr_->refcount++;
		}
	}

    ~PuMap()
    {
        release();
    }

	void operator=(const PuMap &x)
	{
		if (mapptr_ != x.mapptr_)
		{
			if (mapptr_)
			{
				mapptr_->refcount--;
			}
			mapptr_ = x.mapptr_;
			if (mapptr_)
			{
				mapptr_->refcount++;
			}
		}
	}

	bool operator==(const PuMap &x) const
	{
		if (mapptr_ == x.mapptr_)
		{
			return true;
		}
		
		if (!mapptr_ || !x.mapptr_)
		{
			return false;
		}

		return *mapptr_ == *x.mapptr_;
	}

	int size() const
	{
		return mapptr_ ? mapptr_->size() : 0;
	}

    iterator begin()
    {
		if (!mapptr_)
		{
			return end();
		}
		return mapptr_->begin();
    }

    iterator end()
    {
		if (!mapptr_)
		{
			static MapRef dummy;
			return dummy.end();
		}
		return mapptr_->end();
    }

    iterator find(const Key_T &key)
    {
		if (!mapptr_)
		{
			return end();
		}
		return mapptr_->find(key);
    }

    iterator insert(const Key_T &key, const Value_T &value)
    {
		if (!mapptr_)
		{
			mapptr_ = new MapRef;
		}
		return mapptr_->insert(std::make_pair(key, value)).first;
    }

    void release()
    {
        if (mapptr_)
        {
            --mapptr_->refcount;
            if (mapptr_->refcount <= 0)
            {
                delete mapptr_;
            }
            mapptr_ = NULL;
        }
    }

    void clear()
    {
        if (mapptr_)
        {
            mapptr_->clear();
        }
    }

    MapRef *mapptr_;
};

#endif

