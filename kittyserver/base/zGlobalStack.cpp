/**
 * \file	zGlobalStack.h
 * \version  	$Id: zGlobalStack.cpp 13 2013-03-20 02:35:18Z  $
 * \author  	, @ztgame.com 
 * \date 	2010年05月06日 07时38分51秒 CST
 * \brief 定义insturment_function, 实现堆栈另存	
 *
 * 
 */

#include "zGlobalStack.h"

__thread unsigned long __thread_stack[MAX_STACK]={0}; //线程局部变量,用于保存不同线程的堆栈
__thread long __stack_index=-1;
__thread int __stack_valid=0;

void __cyg_profile_func_enter(void* func, void* caller)
{
	if (__stack_index+1>=MAX_STACK)  //判断这次进入对于全局堆栈来说是否有效
	{
		__stack_valid = 0; 
		return;
	}

	__thread_stack[++__stack_index] = (unsigned long)caller;
	__stack_valid=1;
	
	return;
}

void __cyg_profile_func_exit(void* func, void* caller)
{
	
	if (__stack_index>=0 && __stack_valid)
	{
		__thread_stack[__stack_index--] = 0;
	}
	
	return;
}

