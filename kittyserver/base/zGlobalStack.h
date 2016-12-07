/**
 * \file	zGlobalStack.h
 * \version  	$Id: zGlobalStack.h 13 2013-03-20 02:35:18Z  $
 * \author  	, @ztgame.com 
 * \date 	2010年05月06日 07时38分51秒 CST
 * \brief 定义insturment_function, 实现堆栈另存	
 *
 * 
 */

#define MAX_STACK 10000

extern "C"
{
	void __cyg_profile_func_enter(void*, void*) __attribute__((no_instrument_function));
	void __cyg_profile_func_exit(void*, void*) __attribute__((no_instrument_function));
}

extern __thread  unsigned long __thread_stack[MAX_STACK];
extern __thread  long __stack_index;
extern __thread  int __stack_valid;

