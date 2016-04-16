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

template<class Key_T, class Value_T>
struct __bucket_t 
{
    Key_T    key;
    Value_T    value;
};


template<class Key_T, class Value_T, int MAPSIZE=65536>
class PuMap : public PuMemObj
{
public:
    typedef __bucket_t<Key_T,Value_T>   Bucket_T;
    typedef PuVector<Bucket_T,4>        Node_T;
    Node_T*                             _container[MAPSIZE];
    PuVector<int>                       _setted;
    int                                 size_;

    PuMap():size_(0)
    {
        memset(_container, 0, sizeof(Node_T*) * MAPSIZE);
    }

    ~PuMap()
    {
        clear();
    }

    int size() const
    {
        return size_;
    }

    Bucket_T *find(const Key_T &key) const
    {
        unsigned int hk = key.hash() % MAPSIZE;
        Node_T *p = _container[hk];
        if (p)
        {
            int bucket_number = p->size();
            
            if (bucket_number == 1 && key == (*p)[0].key) // 只有1个桶，直接返回第一个元素
            {
                return &(*p)[0];
            }
            // 有多个桶，线性查找
            for (int i=0; i < bucket_number; ++i)
            {
                if (key == (*p)[i].key)
                {
                    return &(*p)[i];
                }
            }
        }
        return 0;
    }

    Bucket_T *insert(const Key_T &key, const Value_T &value)
    {
        unsigned int hk = key.hash()  % MAPSIZE;
        Node_T *p = _container[hk];
        Bucket_T newb = {key,value};
        if (p)
        {
            for (int i = 0; i < p->size(); i++)
            {
                if ((*p)[i].key == key)
                {
                    (*p)[i].value = value;
                    return &(*p)[i];
                }
            }
            p->push_back(newb);
            size_++;
            return &(*p)[p->size()-1];
        }
        p = new Node_T;
        p->push_back(newb);
        _setted.push_back(hk);
        _container[hk] = p;
        size_++;
        return &(*p)[0];
    }

    void clear()
    {
        int s = _setted.size();

        for (int i=0; i < s; ++i)
        {
            int k = _setted[i];
            delete _container[k];
            _container[k] = 0;
        }
        _setted.clear();
        size_ = 0;
    }

};

#endif

