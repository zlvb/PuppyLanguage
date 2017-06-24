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

#pragma once

#ifndef _UTIL_H_
#define _UTIL_H_

#ifdef _DEBUG

#include "state.h"
#include <stdio.h>
#include <utility>

template<typename... Args>
void __debug(Pu *L, const char *fn, int ln, const char *func, Args&&... args)
{
	if (L && L->token && TOKEN.filename)
	{
		const char *pusource_fn = TOKEN.filename->c_str();
		int pusource_ln = TOKEN.line;
		printf("[DEBUG] %s[%d](%s) | %s[%d] ", fn, ln, func,
		pusource_fn, pusource_ln);
	}
	else
	{
		printf("[DEBUG] %s[%d](%s) ", fn, ln, func);
	}

    printf(std::forward<Args>(args)...);
    putchar('\n');
}

inline void __debug(Pu *L, const char *fn, int ln, const char *func, const char *s)
{
	if (L && L->token && TOKEN.filename)
	{
		const char *pusource_fn = TOKEN.filename->c_str();
		int pusource_ln = TOKEN.line;
		printf("[DEBUG]  %s[%d](%s) | %s[%d] %s\n", fn, ln, func, 
		pusource_fn, pusource_ln, 
		s);
	}
	else
	{
		printf("[DEBUG]  %s[%d](%s) %s\n", fn, ln, func, s);
	}
}

#define debug(L, ...) __debug(L, __FILE__, __LINE__, __func__, __VA_ARGS__) 

#else

#define debug(...) 

#endif

#endif

