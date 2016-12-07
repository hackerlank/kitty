/**
 * \file	zTypelist.h
 * \version  	$Id: zTypelist.h 13 2013-03-20 02:35:18Z  $
 * \author  	, @ztgame.com 
 * \date 	2006年11月08日 20时36分26秒 CST
 * \brief 	一个简化版的typelist实现
 *
 * Typelists将来自外星球的强大威力带到C++中，让我们体验这种强大吧－－摘自Modern C++ Design
 * 
 */

#ifndef _TYPELIST_H
#define _TYPELIST_H

namespace Fir
{

	class NullType {};

	/**
	 * \brief 以下几行，就是来自火星的代码
	 *
	 */
	template <typename H, typename T>
		struct Typelist
		{
			typedef H Head;
			typedef T Tail;
		};


	/**
	 * \brief 计算Typelist的长度
	 *
	 */
	template <typename TL> struct Length;
	template <> struct Length<NullType>
	{
		enum {value = 0};
	};

	template <typename H, typename T> struct Length< Typelist<H, T> >
	{
		enum {value = 1 + Length<T>::value };
	};


	/**
	 * \brief 按索引值，取得对应的类型
	 *
	 */
	template <typename TL, unsigned int index> struct TypeAt;

	template <typename H, typename T>
		struct TypeAt<Typelist<H, T>, 0>
		{
			typedef H Result;
		};

	template <typename H, typename T, unsigned int i>
		struct TypeAt<Typelist<H, T>, i>
		{
			typedef typename TypeAt<T, i-1>::Result Result;
		};


	/**
	 * \brief 计算某类型，在Typelist中的索引值
	 *
	 */
	template <typename TL, typename T> struct IndexOf;
	template <typename T>
		struct IndexOf<NullType, T>
		{
			enum {value = -1};
		};

	template <typename T, typename H>
		struct IndexOf<Typelist<H, T>, T>
		{
			enum { value = 0 };
		};

	template <typename H, typename T, typename X>
		struct IndexOf<Typelist<H, T>, X>
		{
			private:
				enum {temp = IndexOf<T, X>::value};
			public:
				enum {value = temp==-1?-1:1+temp};
		};
};

#endif

