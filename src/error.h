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

#ifndef _ERROR_H_
#define _ERROR_H_

struct Pu;

static const char *errtable[] = {
"\" mismatched",//0
"() mismatched",//1
"only the array and string can be accessed by index",//2
"index must be a integer",//3
"[] mismatched",//4
"array index overflow",//5
"invalid type, modify the string failure",//6
"unknown identifiers",//7
"read operator does not specify the variable name",//8
"only digital can subtraction between",//9
"only digital can divide between",//10
"only digital can mod between",//11
"digital can not be multiplied by string",//12
"value can not be compared",//13
"unsupported operation",//14
"function can not nested",//15
"undefined label",//16
"keyword \'end\' mismatched",//17
"function redefined",//18
"include recursive",//19
"can not open file",//20
"invalid argument's type",//21
"can not modify the const value", //22
"a number is required", //23
"a string is required", //24
"requires arguments", //25
"keyword \'end\' mismatched", //26
"function have got wrong number of arguments",// 27
"identifier is too long",//28
"syntax error", // 29
"only number and string can calc hash", // 30
"key must be a number or a string",//31
"key not exist",//32
};


inline const char *geterrcode(int id)
{
    return errtable[id];
}

void error(Pu *L, const char *s, int l=0);
void error(Pu *L, int id, int l=0);

#endif
