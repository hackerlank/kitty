/**
 * \file	firstring.h
 * \version  	$Id: firstring.h 51 2013-04-16 00:37:19Z  $
 * \author  	,
 * \date 	2013年03月19日 17时06分46秒
 * \brief   指针安全的字符串函数	
 *
 * 
 */

#ifndef FIR_STRING_H
#define FIR_STRING_H

#include <string.h> 

char* firstrncpy(char* dest, const char* str, size_t n);
int firstrncasecmp(const char* s1, const char* s2, size_t n);
int firstrcasecmp(const char* s1, const char* s2);

#endif

