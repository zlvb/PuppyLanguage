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

#include "error.h"
#include "state.h"

void error(Pu *L, const char *err_msg, int line)
{
	if (line)
	{
		printf("error: %s\nlines: %d\n", err_msg, line);
	}
	else if (L && L->token && TOKEN.line > 0)
	{
		printf("error: %s\nlines: %d\nfile: %s\n", err_msg, TOKEN.line, TOKEN.filename->c_str());
	}
	else
	{
		printf("error: %s\n", err_msg);
	}

#if defined(_DEBUG) && defined(_MSC_VER)
	_asm {int 3}
#endif
	QUIT_SCRIPT;
}

void error(Pu *L, int id, int line)
{
	L->lasterr = id;
	if (L->err_handle)
	{
		L->err_handle(id, geterrcode(id));
		QUIT_SCRIPT;
	}
	else
	{
		error(L, geterrcode(id), line);
	}

}
