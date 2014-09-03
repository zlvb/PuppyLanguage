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
#include <string.h>
#include <stdlib.h>

#define FIXALI32(n) (((n) + 32-1) & ~(32-1))
#define FIXALI8(n) (((n) + 8-1) & ~(8-1))

struct PuBuffer : public PuMemObj
{
	PuBuffer(const char *s, int l=0):refc(1),count(0)
	{
		if (l==0)
			length = int(strlen(s));
		else
			length = l;
		count = FIXALI32(length+1);
		buffer = (char*)malloc(count);
		memset(buffer,0,length+1);
		memcpy(buffer,s, length);
	}

	inline PuBuffer():buffer(NULL),length(0),refc(1),count(0)
	{}

	~PuBuffer()
	{
		if (buffer)
			free(buffer);
	}

	char *buffer;
	int length;
	int refc;
	int count;
	
};


#ifndef _DEBUG
#define release_buff(zlstr)			\
{									\
	if ((zlstr).pbuff)				\
	{								\
		--(zlstr).pbuff->refc;		\
		if ((zlstr).pbuff->refc <= 0)	\
		{							\
			delete (zlstr).pbuff;		\
		}							\
		(zlstr).pbuff = NULL;			\
	}								\
}
#define RELEASE_BUFF release_buff
#else
#define RELEASE_BUFF PuString::release_buff
#endif

struct PuString : public PuMemObj
{
	PuString():pbuff(NULL),hash_Key(0)
	{
	
	}

	PuString(const char *x):pbuff(NULL),hash_Key(0)
	{
		*this = x;
	}

	inline PuString(const PuString &x):pbuff(NULL),hash_Key(0)
	{
		*this = x;
	}

	~PuString()
	{
		RELEASE_BUFF((*this));
	}

	bool operator==( const char *x ) const
	{
		return (pbuff->buffer == x)? true : strcmp(c_str(),x)==0;
	}

	bool operator!=( const char *x ) const
	{
		return (pbuff->buffer != x)? true : strcmp(c_str(),x)!=0;
	}

	int length() const
	{
		return (pbuff) ? pbuff->length : 0;
	}

	char operator[](int idx) const 
	{
		return pbuff->buffer[idx];
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
	PuString &operator+=(char x);

	const char *c_str() const
	{
		return (pbuff)? pbuff->buffer: "";
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

#ifdef _DEBUG
	static void release_buff(PuString &zlstr) 
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

	PuBuffer *pbuff;
	mutable unsigned int hash_Key;
};


#endif

