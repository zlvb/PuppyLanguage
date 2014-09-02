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
		:count(0),buff(0),cur(-1)
	{
	}

	inline void push( T i )
	{
		if (count <= cur+1)
			resize();

		buff[++cur] = i;
	}

	~PuStack()
	{
		if (buff)
			free(buff);
	}

	inline void pop()
	{
		--cur; 
	}

	inline void clear()
	{
		cur = -1;
	}

	inline bool empty() const
	{
		return size() == 0;
	}

	inline int size() const
	{
		return cur+1;
	}

	inline T top() const
	{
		return buff[cur];
	}

	inline T bottom() const
	{
		return buff[0];
	}

	inline void resize()
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

	inline void release()
	{
		if (buff)
			free(buff);
		buff = 0;
		count = 0;
		cur = -1;
	}

	int count;
	T *buff;
	int cur;
};

#endif

