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

#ifndef __pu_STACK_H__ 
#define __pu_STACK_H__ 
#include <string.h>
#include <stdlib.h>

// 注意，这个栈只能用于存储简单类型
template<class T>
struct  PuStack
{
    inline PuStack()
        :count(0),buff(0),_size(0)
    {
    }

    void push( T i )
    {
        if (count <= _size)
            resize();

        buff[_size++] = i;
    }

    ~PuStack()
    {
        if (buff)
            free(buff);
    }

    void pop()
    {
        --_size; 
    }

    void clear()
    {
        _size = 0;
    }

    bool empty() const
    {
        return size() == 0;
    }

    int size() const
    {
        return _size;
    }

    T top() const
    {
        return buff[_size-1];
    }

    T &top()
    {
        return buff[_size - 1];
    }

    T bottom() const
    {
        return buff[0];
    }

    void resize()
    {
        T *newbuff = (T*)malloc(sizeof(T)*(count+64));
        count += 64;
        if (buff)
        {
            memcpy(newbuff, buff, count * sizeof(T));
            free(buff);
        }
        buff = newbuff;    
    }

    void release()
    {
        if (buff)
            free(buff);
        buff = 0;
        count = 0;
        _size = 0;
    }

    int count;
    T *buff;
    int _size;
};

#endif

