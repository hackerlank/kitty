/**
 * \file	firstring.h
 * \version  	$Id: firstring.cpp 51 2013-04-16 00:37:19Z  $
 * \author  	,
 * \date 	2013年03月19日 17时06分46秒
 * \brief   指针安全的字符串函数	
 *
 * 
 */

#include "firstring.h"

char* firstrncpy(char* dest, const char* str, size_t n)
{
	if (str==NULL || dest==NULL) return dest;
	return strncpy(dest, str, n);
}

int firstrncasecmp(const char* s1, const char* s2, size_t n)
{
	if (s1 == NULL && s2==NULL) return 0;
	if (s1 == NULL || s2==NULL) return -1;

	return strncasecmp(s1,s2,n);
}

int firstrcasecmp(const char* s1, const char* s2)
{
	if (s1 == NULL && s2==NULL) return 0;
	if (s1 == NULL || s2==NULL) return -1;

	return strcasecmp(s1,s2);
}
