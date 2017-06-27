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
    puppy是一个小型语言解释器，支持所有的常规操作，如赋值、循环、条件判断等
    并支持将使用c/c++编写的函数导入到脚本中
*/

#ifndef __Pu_H__
#define __Pu_H__

#include <stdint.h>

typedef int64_t    PU_INT;
typedef double        PU_FLOAT;
typedef PU_FLOAT    PU_NUMBER;

typedef struct Pu Pu;
typedef struct __pu_var __pu_var;
typedef __pu_var* pu_var;

// 可以让脚本调用的函数类型
typedef void (*ScriptFunc)(Pu*, int argnum, pu_var*);

// 操作结果
typedef enum PURESULT{
    PU_FAILED = -1,// 失败
    PU_SUCCESS = 0// 成功
}PURESULT;

// 值类型
typedef enum PUVALUETYPE{
    NIL = 1000,    // 空
    NUM,    // 数字
    STR,        // 字符串
    ARRAY,        // 数组
    MAP,        // 字典
    CORO,        // 协程对象
    FILEHANDLE,    // 文件句柄
    BOOLEANT,        // 布尔
    INTEGER,        // 整数
    FUN,            // 函数
    CFUN,           // C函数
    CPTR,       // 指针
}PUVALUETYPE;

// 值是由脚本系统创建的，还是用户调用pu_new_value创建的
typedef enum PUVALUECREATEDBY{
    PU_SYSTEM,// 脚本系统
    PU_USER// 用户
}PUVALUECREATEDBY;// 修改由脚本系统创建的值会导致失败


#ifdef __cplusplus
extern "C"{
#endif

/**
*
*    获得解释器版本
*    返回值：
*    解释器版本
*    
*/
const char *pu_version();

/**
*
*    生成一个脚本解释器对象
*    返回值：
*    Pu 指针
*    
*/
Pu *pu_open();


/**
*
*    关闭一个脚本解释器对象
*    参数：    
*    Pu *L 脚本解释器对象
*
*/
void pu_close(Pu *L);


/**
*
*    加载脚本
*    参数：    
*    Pu *L 脚本解释器对象
*    const char *fname 脚本文件名
*/
PURESULT pu_load(Pu *L, const char *fname);

/*
 *    载入字符串
 */
PURESULT pu_loadbuff(Pu *L, const char *str);

/**
*
*    执行脚本
*    参数：    
*    Pu *L 脚本解释器对象
*/
void pu_run(Pu *L);


/**
*
*    根据源码生成字节码
*    参数：
*    Pu *L 脚本解释器对象
*    const char *fname 源代码文件名
*
*/
void pu_makebytecode(Pu *L, const char *fname);

/**
*
*    设置脚本函数返回值
*    参数：    
*    Pu *L 脚本解释器对象
*    pu_value *v 返回值
*
*/
void pu_set_return_value(Pu *L, pu_var v);
pu_var pu_get_return_value(Pu *L);

/**
*
*    注册一个函数让脚本调用
*    参数：    
*    Pu *L 脚本解释器对象
*    const char *funcname 在脚本中的函数名字
*    ScriptFunc func 函数指针
*    int argnum 参数个数
*
*/
void pu_reg_func(Pu *L, const char *funcname, ScriptFunc pfunc);

/**
*
*    把值转化成字符串
*    参数：    
*    const pu_value v 与脚本交互的值类型的对象
*    返回：
*    值所代表的字符串
*
*/
const char *pu_str(pu_var v);

/**
*
*    把值转化成数字
*    参数：    
*    const pu_value v 与脚本交互的值类型的对象
*    返回：
*    值所代表的数值
*
*/
PU_NUMBER pu_num(pu_var v);

/**
*
*    把值转化成整数
*    参数：    
*    const pu_value v 与脚本交互的值类型的对象
*    返回：
*    值所代表的整数
*
*/
PU_INT pu_int(pu_var v);

/**
*
*    把值转化成指针
*    参数：    
*    const pu_value v 与脚本交互的值类型的对象
*    返回：
*    值所代表的指针
*
*/
void *pu_ptr(pu_var v);

/**
*
*    获得数组下标所指向的值
*    参数：    
*    const pu_value v 与脚本交互的值类型的对象
*    int idx 数组下标
*    返回：
*    数组成员
*
*/
pu_var pu_arr(pu_var v, int idx);

/**
*
*    获得值的类型
*    参数：    
*    const pu_value v 与脚本交互的值类型的对象
*    返回：
*    类型
*
*/
PUVALUETYPE pu_type(pu_var v);

/**
*
*    把值设置为字符串
*    参数：    
*    const pu_value v 与脚本交互的值类型的对象
*    const char *str 要设置的字符串
*
*/
PURESULT pu_set_str(pu_var v, const char *str);

/**
*
*    把值设置为数字
*    参数：    
*    const pu_value v 与脚本交互的值类型的对象
*    int num 要设置的数字
*
*/
PURESULT pu_set_num(pu_var v, PU_NUMBER num);

/**
*
*    把值设置为整数
*    参数：    
*    const pu_value v 与脚本交互的值类型的对象
*    int num 要设置的整数
*
*/
PURESULT pu_set_int(pu_var v, PU_INT num);

/**
*
*    把值设置为指针
*    参数：    
*    const pu_value v 与脚本交互的值类型的对象
*    void *ptr 要设置的指针
*
*/
PURESULT pu_set_ptr(pu_var v, void *ptr);

/**
*
*    把值设置为数组
*    参数：    
*    const pu_value v 与脚本交互的值类型的对象
*    const int num 要设置的数字
*
*/
PURESULT pu_set_arr(pu_var varr, int idx, pu_var v);

// 值是由脚本系统创建的，还是用户调用pu_new_value创建的
PUVALUECREATEDBY pu_value_created_by(pu_var v);

// 在数组的最后插入值
PURESULT pu_push_arr(pu_var varr, pu_var v);


// 在数组的最后删除值
PURESULT pu_pop_arr(pu_var varr);

// 得到值的长度
int pu_len(pu_var v);


/**
*
*    产生新的值
*    参数：    
*    Pu *L 脚本解释器对象
*    返回：
*    值对象
*
*/
pu_var pu_new_value(Pu *L);

/**
*
*    删除一个值
*    参数：    
*    const pu_value v 与脚本交互的值类型的对象
*
*/
PURESULT pu_del_value(pu_var v);

/**
*
*    调用脚本中的函数
*    参数：    
*    Pu *L 脚本解释器对象
*    const char *function_name 函数名
*    const pu_value varr 参数数组
*
*/
pu_var pu_call(Pu *L, const char *function_name, pu_var varr);

/**
*
*    获得脚本中的全局变量
*    参数：    
*    Pu *L 脚本解释器对象
*    const char *varname 变量名
*    返回值：
*    脚本中的变量
*
*/
pu_var pu_global(Pu *L, const char *varname);


/*
 *    立即执行一个字符串
 */
int pu_eval(Pu *L, const char *str);


/*
 *    得到一个值的字符串形式
 */
void pu_val2str(Pu *L, pu_var *v, char *buff, int buffsize);


#ifdef __cplusplus
}
#endif

#endif


