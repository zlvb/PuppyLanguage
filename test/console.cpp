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

#include <stdio.h>
#include <stdlib.h>
#include "../puppy.h"
extern void bi_return_null_func(Pu *L);
// 用于控制台模式下的print函数
void print(Pu *L, int argnum, pu_value *v)
{
	if (v==NULL)
	{
		printf("\n");
        bi_return_null_func(L);
		return;
	}
	
	int i=0;
	while (i<argnum)
	{
		char buff[1024]={0};
		pu_val2str(L,&(v[i]),buff, sizeof(buff));
		printf("%s ",buff);
		i++;
	}
	printf("\n");

    bi_return_null_func(L);
}

// 用于控制台模式下的help函数
void help(Pu *L, int , pu_value *)
{
	printf (
		"len(object)\n\tReturn the size of a expression\n"
		"write(...)\n\tPrint the contents or write contents to file\n"
		"print(...)\n\tPrint the contents with a line-break at the end\n"
		"read(...)\n\tGet the user input and return or read from file\n"
		"open(filename)\n\tOpen a file and return the file handle for read and write\n"
		"close(file_handle)\n\tClose a file\n"
		"rand()\n\tReturn a random integer\n"
		"time()\n\tReturn the current time\n"
		"sleep(number)\n\tPause the script for \'number\' milliseconds\n"
		"type(object)\n\tReturn the type of a expression\n"
		"eval(str)\n\tEvaluate the \'str\' as a script\n"
		"quit()\n\tEnd the script\n"
		"exit()\n\tQuit the program\n"
		"str(object)\n\tConvert the object to string\n"
		"num(object)\n\tConvert the object to number\n"
		"coro_create(func,...)\n\tCreate a coroutine and return it\n"
		"coro_resume(coroutine)\n\tResume a coroutine\n"
		"coro_yield()\n\tYield the current coroutine\n"
		"help()\n\tDisplay this infomation\n");

    bi_return_null_func(L);
}

bool g_bExit = false;
// 用于退出控制台的exit函数
void exit_program(Pu *L, int, pu_value *)
{
	g_bExit = true;
    bi_return_null_func(L);
}

#define PU_MAXINPUT	65536
#define pu_readline(b) (fgets(b, PU_MAXINPUT, stdin) != NULL)
extern void regbuiltin(Pu *L);
void pu_console(Pu *L)
{
	regbuiltin(L);
	printf(
		"%s  Copyright (C) 2014 Zhang li\n"
		"type \"help()\" for more infomation\n"
		, pu_version());
	pu_reg_func(L, "print", print);
	pu_reg_func(L, "help", help);
	pu_reg_func(L, "exit", exit_program);
	for (;!g_bExit;)
	{
		static char buff[PU_MAXINPUT];
		putchar('>');
		if (pu_readline(buff))
		{
			int ret = pu_eval(L, buff);
			if (ret == -2) // input incompleted
            {
				putchar('>');
            }
		}
	}
}



