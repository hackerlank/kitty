/**
 * \file	zSequence.h
 * \version  	$Id: zSequence.h 13 2013-03-20 02:35:18Z  $
 * \author  	, @ztgame.com 
 * \date 	2006年11月08日 22时21分22秒 CST
 * \brief 	a typelist wrapper
 *
 * 
 */

#ifndef _TL_SEQUENCE_H
#define _TL_SEQUENCE_H
#include "zTypelist.h"

namespace Fir
{
	template <
		typename T01 = NullType, typename T02=NullType, typename T03=NullType, typename T04=NullType,
		typename T05 = NullType, typename T06=NullType, typename T07=NullType, typename T08=NullType
		>
		struct Seq;

	template<typename T01>
		struct Seq<T01>
		{
			typedef Typelist<T01, NullType> Type;
		};

	template<typename T01, typename T02>
		struct Seq<T01, T02>
		{
			typedef Typelist<T01, Typelist<T02, NullType> > Type;
		};

	template<typename T01, typename T02, typename T03>
		struct Seq<T01, T02, T03>
		{
			typedef Typelist<T01, Typelist<T02, Typelist<T03, NullType> > > Type;
		};

	template<typename T01, typename T02, typename T03, typename T04>
		struct Seq<T01, T02, T03, T04>
		{
			typedef Typelist<T01, Typelist<T02, Typelist<T03, Typelist<T04, NullType> > > > Type;
		};

	template<typename T01, typename T02, typename T03, typename T04, typename T05>
		struct Seq<T01, T02, T03, T04, T05>
		{
			typedef Typelist<T01, Typelist<T02, Typelist<T03, Typelist<T04, Typelist <T05, NullType> > > > > Type;
		};

	template<typename T01, typename T02, typename T03, typename T04, typename T05, typename T06>
		struct Seq<T01, T02, T03, T04, T05, T06>
		{
			typedef Typelist<T01, 
					Typelist<T02, 
					Typelist<T03, 
					Typelist<T04, 
					Typelist<T05, 
					Typelist<T06, NullType> 
						> > > >	>
						Type;
		};

	template<typename T01, typename T02, typename T03, typename T04, typename T05, typename T06, typename T07>
		struct Seq<T01, T02, T03, T04, T05, T06, T07>
		{
			typedef Typelist<T01, 
					Typelist<T02, 
					Typelist<T03, 
					Typelist<T04, 
					Typelist<T05, 
					Typelist<T06, 
					Typelist<T07, NullType> 
						> > > > > >
						Type;
		};

	template<typename T01, typename T02, typename T03, typename T04, typename T05, typename T06, typename T07, typename T08>
		struct Seq
		{
			typedef Typelist<T01, 
					Typelist<T02, 
					Typelist<T03, 
					Typelist<T04, 
					Typelist<T05, 
					Typelist<T06, 
					Typelist<T07, 
					Typelist<T08, NullType> 
						> > > >	> >	>
						Type;
		};
		// */

};

#endif
