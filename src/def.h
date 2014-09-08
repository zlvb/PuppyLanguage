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

#ifndef __VM_DEF_H__
#define __VM_DEF_H__

#include "config.h"

struct Pu;
struct Token;
struct __pu_value;
typedef __pu_value* pu_value; 
struct FuncPos;

#define PUAPI extern "C"

// 可以让脚本调用的函数类型
typedef void (*ScriptFunc)(Pu*, int arg_num, const pu_value*);

// 错误处理回调
typedef void (*ErrHandle)(int err, const char *err_str);

// 处理输出的回调
typedef void (*OutputHandle)(const char *str);

#define QUIT_SCRIPT			{L->isquit=true; L->cur_token = L->tokens.size() - 1;NEXT_TOKEN}
#define TOKEN				(*(L->token))
#define NEXT_TOKEN			{L->token = &(L->tokens[L->cur_token++]);}
#define PREV_TOKEN			{L->cur_token-=2;NEXT_TOKEN;}

// 操作结果
typedef enum PURESULT{
	PU_FAILED = -1,// 失败
	PU_SUCCESS = 0// 成功
}PURESULT;

#ifdef _MSC_VER
#define PU_SNPRINTF(dest, size, fmt, ...) _snprintf(dest, size, fmt, __VA_ARGS__)
#else
#define PU_SNPRINTF(dest, size, fmt, ...) snprintf(dest, size, fmt, __VA_ARGS__)
#endif

#define MAKE_TEMP_VALUE(p) \
    p = new __pu_value(L);\
    p->readonly(true);\
    L->tempvals.push_back(p);

#define CHECK_EXP(v) \
    if (!v || v->type() == UNKNOWN)\
    {\
        error(L, 7);\
        return;\
    }
    
#define CHECK_EXP_RETURN(v, r) \
    if (!v || v->type() == UNKNOWN)\
    {\
        error(L, 7);\
        return r;\
    }

#define CHECK_EXP_RETURNERR(v) \
    if (!v || v->type() == UNKNOWN)\
    {\
        error(L, 7);\
        __pu_value *_r = NULL;\
        MAKE_TEMP_VALUE(_r);\
        return _r;\
    }
#endif
