/**
 * \file	zObjectFactory.h
 * \version  	$Id: zObjectFactory.h 13 2013-03-20 02:35:18Z  $
 * \author  	, @ztgame.com 
 * \date 	2006年11月08日 05时35分22秒 CST
 * \brief 	基于模板的对象工厂
 *
 * 
 */

#ifndef _OBJECT_FACTORY_H
#define _OBJECT_FACTORY_H
#include <map>
#include "zTypelist.h"
#include "zFunctor.h"
#include "zSequence.h"

using namespace Fir;

template <typename R, typename TL>
struct CreatorT;

template <typename R, typename P1>
struct CreatorT<R, Seq<P1> >
{
	R* operator()(P1 p1)
	{
		return FIR_NEW R(p1);
	}
};

template <typename R, typename P1, typename P2>
struct CreatorT<R, Seq<P1, P2> >
{
	R* operator()(P1 p1, P2 p2)
	{
		return FIR_NEW R(p1, p2);
	}
};

template <typename R, typename P1, typename P2, typename P3>
struct CreatorT<R, Seq<P1, P2, P3> >
{
	R* operator()(P1 p1, P2 p2, P3 p3)
	{
		return FIR_NEW R(p1, p2, p3);
	}
};

template <typename R, typename P1, typename P2, typename P3, typename P4>
struct CreatorT<R, Seq<P1, P2, P3, P4> >
{
	R* operator()(P1 p1, P2 p2, P3 p3, P4 p4)
	{
		return FIR_NEW R(p1, p2, p3, p4);
	}
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
struct CreatorT<R, Seq<P1, P2, P3, P4, P5> >
{
	R* operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5)
	{
		return FIR_NEW R(p1, p2, p3, p4, p5);
	}
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
struct CreatorT<R, Seq<P1, P2, P3, P4, P5, P6> >
{
	R* operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6)
	{
		return FIR_NEW R(p1, p2, p3, p4, p5, p6);
	}
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
struct CreatorT<R, Seq<P1, P2, P3, P4, P5, P6, P7> >
{
	R* operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7)
	{
		return FIR_NEW R(p1, p2, p3, p4, p5, p6, p7);
	}
};

template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
struct CreatorT<R, Seq<P1, P2, P3, P4, P5, P6, P7, P8> >
{
	R* operator()(P1 p1, P2 p2, P3 p3, P4 p4, P5 p5, P6 p6, P7 p7, P8 p8)
	{
		return FIR_NEW R(p1, p2, p3, p4, p5, p6, p7);
	}
};


struct FactoryImplBase
{
	typedef NullType Param1;
	typedef NullType Param2;
	typedef NullType Param3;
	typedef NullType Param4;
	typedef NullType Param5;
	typedef NullType Param6;
	typedef NullType Param7;
	typedef NullType Param8;
};

template <typename AO, typename IT, typename TL>
struct FactoryImpl;

template <typename AO, typename IT>
struct FactoryImpl<AO, IT, NullType> : public FactoryImplBase
{
	virtual ~FactoryImpl(){}
	virtual AO* createObject(const IT& id) = 0;
};

template <typename AO, typename IT, typename P1>
struct FactoryImpl<AO, IT, Seq<P1> > : public FactoryImplBase
{
	typedef  P1 Param1;

	virtual ~FactoryImpl(){}
	virtual AO* createObject(const IT& id, Param1) = 0;
};

template <typename AO, typename IT, typename P1, typename P2>
struct FactoryImpl<AO, IT, Seq<P1, P2> > : public FactoryImplBase
{
	typedef  P1 Param1;
	typedef  P2 Param2;

	virtual ~FactoryImpl(){}
	virtual AO* createObject(const IT& id, Param1, Param2) = 0;
};

template <typename AO, typename IT, typename P1, typename P2, typename P3>
struct FactoryImpl<AO, IT, Seq<P1, P2, P3> > : public FactoryImplBase
{
	typedef  P1 Param1;
	typedef  P2 Param2;
	typedef  P3 Param3;

	virtual ~FactoryImpl(){}
	virtual AO* createObject(const IT& id, Param1, Param2, Param3) = 0;
};

template <typename AO, typename IT, typename P1, typename P2, typename P3, typename P4>
struct FactoryImpl<AO, IT, Seq<P1, P2, P3, P4> > : public FactoryImplBase
{
	typedef  P1 Param1;
	typedef  P2 Param2;
	typedef  P3 Param3;
	typedef  P4 Param4;

	virtual ~FactoryImpl(){}
	virtual AO* createObject(const IT& id, Param1, Param2, Param3, Param4) = 0;
};

template <typename AO, typename IT, typename P1, typename P2, typename P3, typename P4, typename P5>
struct FactoryImpl<AO, IT, Seq<P1, P2, P3, P4, P5> > : public FactoryImplBase
{
	typedef  P1 Param1;
	typedef  P2 Param2;
	typedef  P3 Param3;
	typedef  P4 Param4;
	typedef  P5 Param5;

	virtual ~FactoryImpl(){}
	virtual AO* createObject(const IT& id, Param1, Param2, Param3, Param4, Param5) = 0;
};

template <typename AO, typename IT, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
struct FactoryImpl<AO, IT, Seq<P1, P2, P3, P4, P5, P6> > : public FactoryImplBase
{
	typedef  P1 Param1;
	typedef  P2 Param2;
	typedef  P3 Param3;
	typedef  P4 Param4;
	typedef  P5 Param5;
	typedef  P6 Param6;

	virtual ~FactoryImpl(){}
	virtual AO* createObject(const IT& id, Param1, Param2, Param3, Param4, Param5, Param6) = 0;
};

template <typename AO, typename IT, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
struct FactoryImpl<AO, IT, Seq<P1, P2, P3, P4, P5, P6, P7> > : public FactoryImplBase
{
	typedef  P1 Param1;
	typedef  P2 Param2;
	typedef  P3 Param3;
	typedef  P4 Param4;
	typedef  P5 Param5;
	typedef  P6 Param6;
	typedef  P7 Param7;

	virtual ~FactoryImpl(){}
	virtual AO* createObject(const IT& id, Param1, Param2, Param3, Param4, Param5, Param6, Param7) = 0;
};

template <typename AO, typename IT, typename P1, typename P2, typename P3, typename P4, 
		 typename P5, typename P6, typename P7, typename P8>
struct FactoryImpl<AO, IT, Seq<P1, P2, P3, P4, P5, P6, P7, P8> > : public FactoryImplBase
{
	typedef  P1 Param1;
	typedef  P2 Param2;
	typedef  P3 Param3;
	typedef  P4 Param4;
	typedef  P5 Param5;
	typedef  P6 Param6;
	typedef  P7 Param7;
	typedef  P8 Param8;

	virtual ~FactoryImpl(){}
	virtual AO* createObject(const IT& id, Param1, Param2, Param3, Param4, Param5, Param6, Param7, Param8) = 0;
};

template
<
	typename AbstractObject,
	typename IdentifierType,
	typename TL = NullType
>
class Factory 
{
	typedef FactoryImpl<AbstractObject, IdentifierType, TL> Impl;
	typedef typename Impl::Param1 Param1;
	typedef typename Impl::Param2 Param2;
	typedef typename Impl::Param3 Param3;
	typedef typename Impl::Param4 Param4;
	typedef typename Impl::Param5 Param5;
	typedef typename Impl::Param6 Param6;
	typedef typename Impl::Param7 Param7;
	typedef typename Impl::Param8 Param8;

	typedef Functor<AbstractObject*, TL> ObjectCreator;

	public:
	Factory() : associations()
	{
	}
	~Factory()
	{
		associations.erase(associations.begin(), associations.end());
	}

	bool Register(const IdentifierType& id, ObjectCreator creator)
	{
		return associations.insert(typename AssocMap::value_type(id, creator)).second!=0;
	}

	template <typename PtrObj, typename MemFun>
		bool Register(const IdentifierType& id, const PtrObj obj, MemFun fun)
		{
			ObjectCreator creator(obj, fun);

			return associations.insert(typename AssocMap::value_type(id, creator)).second!=0;
		}

	AbstractObject* CreateObject(const IdentifierType& id)
	{
		typename AssocMap::const_iterator i= associations.find(id);

		if (i !=associations.end())
		{
			return (i->second)();
		}

		return NULL;
	}

	AbstractObject* CreateObject(const IdentifierType& id, Param1 p1)
	{
		typename AssocMap::const_iterator i= associations.find(id);

		if (i !=associations.end())
		{
			return (i->second)(p1);
		}

		return NULL;
	}

	AbstractObject* CreateObject(const IdentifierType& id, Param1 p1, Param2 p2)
	{
		typename AssocMap::const_iterator i= associations.find(id);

		if (i !=associations.end())
		{
			return (i->second)(p1, p2);
		}

		return NULL;
	}

	AbstractObject* CreateObject(const IdentifierType& id, Param1 p1, Param2 p2, Param3 p3)
	{
		typename AssocMap::const_iterator i= associations.find(id);

		if (i !=associations.end())
		{
			return (i->second)(p1, p2, p3);
		}

		return NULL;
	}

	AbstractObject* CreateObject(const IdentifierType& id, Param1 p1, Param2 p2, Param3 p3, Param4 p4)
	{
		typename AssocMap::const_iterator i= associations.find(id);

		if (i !=associations.end())
		{
			return (i->second)(p1, p2, p3, p4);
		}

		return NULL;
	}

	AbstractObject* CreateObject(const IdentifierType& id, Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5)
	{
		typename AssocMap::const_iterator i= associations.find(id);

		if (i !=associations.end())
		{
			return (i->second)(p1, p2, p3, p4, p5);
		}

		return NULL;
	}

	AbstractObject* CreateObject(const IdentifierType& id, Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6)
	{
		typename AssocMap::const_iterator i= associations.find(id);

		if (i !=associations.end())
		{
			return (i->second)(p1, p2, p3, p4, p5, p6);
		}

		return NULL;
	}

	AbstractObject* CreateObject(const IdentifierType& id, Param1 p1, Param2 p2, Param3 p3, 
			Param4 p4, Param5 p5, Param6 p6, Param7 p7)
	{
		typename AssocMap::const_iterator i= associations.find(id);

		if (i !=associations.end())
		{
			return (i->second)(p1, p2, p3, p4, p5, p6, p7);
		}

		return NULL;
	}

	AbstractObject* CreateObject(const IdentifierType& id, Param1 p1, Param2 p2, Param3 p3, 
			Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8)
	{
		typename AssocMap::const_iterator i= associations.find(id);

		if (i !=associations.end())
		{
			return (i->second)(p1, p2, p3, p4, p5, p6, p7, p8);
		}

		return NULL;
	}


	typedef std::map<IdentifierType, ObjectCreator> AssocMap;
	AssocMap associations;
};
#endif

