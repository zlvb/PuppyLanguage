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
    if (pbuff->buffer[idx] != c)
    {
        if (pbuff->refc > 1)
        {
            int count = FIXALI8(length()+1);
            char *pb = (char*)malloc(count);
            memcpy(pb, c_str(), length());
            pb[length()] = 0;
            RELEASE_BUFF((*this));
            pbuff = new PuBuffer;
            pbuff->buffer = pb;
            pbuff->length = length();
            pbuff->count = count;
        }
        pbuff->buffer[idx]=c;
        hash_Key = 0;
    }
}

PuString & PuString::operator=( const char *x )
{
    if (pbuff)
    {
        if (pbuff->buffer == x)
            return *this;

        RELEASE_BUFF((*this));
    }
    hash_Key = 0;
    pbuff = new PuBuffer(x);
    return *this;
}

PuString & PuString::operator=( const PuString &x )
{
    if (pbuff)
    {
        if (pbuff == x.pbuff)
            return *this;

        RELEASE_BUFF((*this));
    }
    pbuff = x.pbuff;
    hash_Key = x.hash_Key;

    if (pbuff)
        ++pbuff->refc;

    return *this;
}

PuString PuString::operator+( const PuString &x ) const
{
    int len = length()+x.length();
    int count = FIXALI8(len+1);
    char *pb = (char*)malloc(count);
    memcpy(pb, c_str(), length());
    memcpy(pb+length(),x.c_str(),x.length());
    pb[len] = 0;
    PuBuffer *temp = new PuBuffer;
    temp->buffer = pb;
    temp->length = len;
    PuString ts;
    ts.pbuff = temp;
    ts.pbuff->count = count;
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
    int clen = (int)strlen(x);
    int len = length()+clen;
    int count = FIXALI8(len+1);
    char *pb = (char*)malloc(count);
    memcpy(pb, c_str(), length());
    memcpy(pb+length(),x,clen);
    pb[len] = 0;
    PuBuffer *temp = new PuBuffer;
    temp->buffer = pb;
    temp->length = len;
    PuString ts;
    ts.pbuff = temp;
    ts.pbuff->count = count;
    return ts;
}

bool PuString::operator==( const PuString &x ) const
{
    return (pbuff == x.pbuff)? true : strcmp(c_str(),x.c_str())==0;
}

bool PuString::operator!=( const PuString &x ) const
{
    return (pbuff != x.pbuff)? true : strcmp(c_str(),x.c_str())!=0;
}

PuString & PuString::operator+=( const PuString &x )
{
    int len = length() + x.length();

    if (pbuff == NULL || len+1 > pbuff->count)
    {
        int count = FIXALI32(len+1+128);
        char *pb = (char*)malloc(count);
        memcpy(pb,c_str(),length());
        memcpy(pb+length(),x.c_str(),x.length());
        pb[len] = 0;
        RELEASE_BUFF((*this));
        pbuff = new PuBuffer();
        pbuff->buffer = pb;
        pbuff->count = count;
    }
    else
    {
        memcpy(pbuff->buffer+length(),x.c_str(),x.length()+1);
    }
    
    pbuff->length = len;
    hash_Key = 0;
    return *this;
}

PuString & PuString::operator+=( const char *x )
{
    int clen = (int)strlen(x);
    int len = length() + clen;

    if (pbuff == NULL || len+1 > pbuff->count)
    {
        int count = FIXALI32(len+1+128);
        char *pb = (char*)malloc(count);
        memcpy(pb,c_str(),length());        
        memcpy(pb+length(),x,clen);
        pb[len] = 0;
        RELEASE_BUFF((*this));
        pbuff = new PuBuffer();
        pbuff->buffer = pb;
        pbuff->count = count;
    }
    else
    {
        memcpy(pbuff->buffer+length(),x,clen+1);
    }

    pbuff->length = len;
    hash_Key = 0;
    return *this;
}

#ifdef _DEBUG
void release_buff(PuString &zlstr) 
{
    if (zlstr.pbuff)
    {
        --zlstr.pbuff->refc;
        if (zlstr.pbuff->refc <= 0)
        {
            delete zlstr.pbuff;
        }
        zlstr.pbuff = NULL;
    }
}
#endif