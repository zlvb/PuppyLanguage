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

#include "PuString.h"

void PuString::set_char( int idx, char c )
{
    if ((*pbuff)[idx] != c)
    {
        if (pbuff->refc > 1)
        {
            PuBuffer *new_pbuff = new PuBuffer(c_str(), length());
            release_buff();
            pbuff = new_pbuff;
        }
        (*pbuff)[idx]=c;
        hash_Key = 0;
    }
}

PuString & PuString::operator=( const char *x )
{
    if (pbuff)
    {
        if (pbuff->c_str() == x)
        {
            return *this;
        }

        release_buff();
    }
    hash_Key = 0;
    pbuff = new PuBuffer(x);
    return *this;
}

PuString & PuString::operator=( const PuString &x )
{
    if (pbuff == x.pbuff)
    {
        return *this;
    }

    if (pbuff)
    {
        release_buff();
    }
    pbuff = x.pbuff;
    hash_Key = x.hash_Key;

    if (pbuff)
    {
        ++pbuff->refc;
    }

    return *this;
}

PuString PuString::operator+( const PuString &x ) const
{
    PuBuffer *temp = new PuBuffer(c_str(), length());
    temp->append(x.c_str(), x.length());
    PuString ts;
    ts.pbuff = temp;
    return ts;
}

PuString &PuString::operator+=(char x)
{
    char temp[2]={0};
    temp[0] = x;
    return operator+=(temp);
}

PuString PuString::operator+( const char *x ) const
{
    PuBuffer *temp = new PuBuffer(c_str(), length());
    temp->append(x);
    PuString ts;
    ts.pbuff = temp;
    return ts;
}

bool PuString::operator==( const PuString &x ) const
{
    return (pbuff == x.pbuff)? true : strcmp(c_str(),x.c_str()) == 0;
}

bool PuString::operator!=(const PuString &x) const
{
    return !(*this == x);
}

PuString &PuString::operator+=( const PuString &x )
{
    if (pbuff)
    {
        PuBuffer *new_pbuff = new PuBuffer(c_str(), length());
        new_pbuff->append(x.c_str(), x.length());
        release_buff();
        pbuff = new_pbuff;
        hash_Key = 0;
    }
    else
    {
        pbuff = new PuBuffer(x.c_str(), x.length());
    }
    
    return *this;
}

PuString & PuString::operator+=( const char *x )
{
    if (pbuff)
    {
        PuBuffer *new_pbuff = new PuBuffer(c_str(), length());
        new_pbuff->append(x);
        release_buff();
        pbuff = new_pbuff;
        hash_Key = 0;
    }
    else
    {
        pbuff = new PuBuffer(x);
    }

    return *this;
}

void PuString::release_buff()
{
    if (pbuff)
    {
        --pbuff->refc;
        if (pbuff->refc <= 0)
        {
            delete pbuff;
        }
        pbuff = nullptr;
    }
}