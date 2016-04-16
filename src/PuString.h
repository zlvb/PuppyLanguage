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

#ifndef _ZLSTR_H_
#define _ZLSTR_H_

#include "global.h"
#include <string>
#include <stdlib.h>

struct PuBuffer : public std::string
{
    PuBuffer(const char *s)
        :std::string(s)
        ,refc(1)
    {

    }

    PuBuffer(const char *s, int l)
        :std::string(s, l)
        , refc(1)
    {

    }

    inline PuBuffer()
        :refc(1)
    {}

    int refc;
};


struct PuString
{
    PuString():pbuff(NULL),hash_Key(0)
    {
    
    }

    PuString(const char *x):pbuff(NULL),hash_Key(0)
    {
        *this = x;
    }

    PuString(const PuString &x):pbuff(NULL),hash_Key(0)
    {
        *this = x;
    }

    ~PuString()
    {
        release_buff();
    }

    bool operator==( const char *x ) const
    {
        if (!pbuff)
        {
            return false;
        }
        return *pbuff == x;
    }

    bool operator!=( const char *x ) const
    {
        return !(*this == x);
    }

    int length() const
    {
        return (pbuff) ? pbuff->length() : 0;
    }

    char operator[](int idx) const 
    {
        return (*pbuff)[idx];
    }

    void set_char(int idx, char c);

    bool operator<(const PuString &x) const
    {
        return strcmp(c_str(),x.c_str()) < 0;
    }

    PuString &operator=(const char *x);
    PuString &operator=(const PuString &x);
    PuString operator+(const PuString &x) const;
    PuString operator+(const char *x) const;

    PuString operator+(char x) const
    {
        char temp[2]={0};
        temp[0] = x;
        return operator+(temp);
    }

    bool operator==(const PuString &x) const;
    bool operator!=(const PuString &x) const;
    PuString &operator+=(const PuString &x);
    PuString &operator+=(const char *x);
    void release_buff();
    PuString &operator+=(char x);

    const char *c_str() const
    {
        return (pbuff)? pbuff->c_str(): "";
    }

    unsigned int hash() const
    {
        if (hash_Key != 0)
            return hash_Key;

        const unsigned int seed = 131;
        const char *str = c_str();
        unsigned int h = 0;

        while (*str)
            h = h * seed + ( *str++ );
        
        hash_Key = h & 0x7FFFFFFF;
        return hash_Key;
    }

    PuBuffer *pbuff;
    mutable unsigned int hash_Key;
};


#endif

