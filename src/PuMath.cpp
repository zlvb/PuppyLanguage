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

#include "PuMap.h"
#include <ctype.h>

// 判断字符串是否可表示为一个整数
extern "C" int is_int(const char *str)
{
    if (!str) return 0;

    // 判断是否有符号
    if (str[0] == '-' || str[0] == '+')
    {
        ++str;
    }

    while (*str != '\0')
    {
        if (!isdigit(*str)) return 0;
        ++str;
    }
    return 1;
}

// 判断字符串是否可表示为一个浮点数
extern "C" int is_float(const char *str)
{
    int point = 0;
    if (!str) return 0;

    // 判断是否有符号
    if (str[0] == '-' || str[0] == '+')
    {
        ++str;
    }

    while (*str != '\0')
    {
        if (!isdigit(*str))
        {
            if (*str == '.' && point == 0)
            {
                point=1;
            }
            else
            {
                return 0;
            }
        }
        ++str;
    }
    return 1;
}

