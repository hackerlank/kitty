/**
 * \file	zFunctor.h
 * \version  	$Id: zFunctor.h 13 2013-03-20 02:35:18Z  $
 * \author  	, @ztgame.com 
 * \date 	2006年11月09日 00时12分55秒 CST
 * \brief 	基于Typelist模板的仿函数wrapper
 *
 * 
 */

#ifndef  _FUNCTOR_H
#define  _FUNCTOR_H

#include "zTypelist.h"
#include "zSequence.h"
#include "Fir.h"
#include <memory>

namespace Fir
{
	template <typename R>
		struct FunctorImplBase
		{
			typedef R ResultType;
			typedef NullType Param1;
			typedef NullType Param2;
			typedef NullType Param3;
			typedef NullType Param4;
			typedef NullType Param5;
			typedef NullType Param6;
			typedef NullType Param7;
			typedef NullType Param8;


			virtual ~FunctorImplBase()
			{
			}

			virtual FunctorImplBase* doClone() const  = 0;

			template <typename X>
				static X* clone(X* pObj)
				{
					if (!pObj) return NULL;
					X* pClone = static_cast<X*>(pObj->doClone());
					return pClone;
				}
		};

	template <typename R, typename TL>
		class FunctorImpl;

		template <typename R>
		class FunctorImpl<R, NullType> : public FunctorImplBase<R>
		{
			public:
				typedef R ResultType;
				virtual R operator()() = 0;
		};

	template <typename R, typename P1>
		class FunctorImpl<R, Seq<P1> > : public FunctorImplBase<R>
		{
			public:
				typedef R ResultType;
				typedef P1 Param1;
				virtual R operator()(P1) = 0;
		};

	template <typename R, typename P1, typename P2>
		class FunctorImpl<R, Seq<P1, P2> > : public FunctorImplBase<R>
		{
			public:
				typedef R ResultType;
				typedef P1 Param1;
				typedef P2 Param2;
				virtual R operator()(P1, P2) = 0;
		};

	template <typename R, typename P1, typename P2, typename P3>
		class FunctorImpl<R, Seq<P1, P2, P3> > : public FunctorImplBase<R>
		{
			public:
				typedef R ResultType;
				typedef P1 Param1;
				typedef P2 Param2;
				typedef P3 Param3;

				virtual R operator()(P1, P2, P3) = 0;
		};

	template <typename R, typename P1, typename P2, typename P3, typename P4>
		class FunctorImpl<R, Seq<P1, P2, P3, P4> > : public FunctorImplBase<R>
		{
			public:
				typedef R ResultType;
				typedef P1 Param1;
				typedef P2 Param2;
				typedef P3 Param3;
				typedef P4 Param4;

				virtual R operator()(P1, P2, P3, P4) = 0;
		};

	template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5>
		class FunctorImpl<R, Seq<P1, P2, P3, P4, P5> > : public FunctorImplBase<R>
		{
			public:
				typedef R ResultType;
				typedef P1 Param1;
				typedef P2 Param2;
				typedef P3 Param3;
				typedef P4 Param4;
				typedef P5 Param5;

				virtual R operator()(P1, P2, P3, P4, P5) = 0;
		};

	template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6>
		class FunctorImpl<R, Seq<P1, P2, P3, P4, P5, P6> > : public FunctorImplBase<R>
		{
			public:
				typedef R ResultType;
				typedef P1 Param1;
				typedef P2 Param2;
				typedef P3 Param3;
				typedef P4 Param4;
				typedef P5 Param5;
				typedef P6 Param6;

				virtual R operator()(P1, P2, P3, P4, P5, P6) = 0;
		};

	template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7>
		class FunctorImpl<R, Seq<P1, P2, P3, P4, P5, P6, P7> > : public FunctorImplBase<R>
		{
			public:
				typedef R ResultType;
				typedef P1 Param1;
				typedef P2 Param2;
				typedef P3 Param3;
				typedef P4 Param4;
				typedef P5 Param5;
				typedef P6 Param6;
				typedef P6 Param7;

				virtual R operator()(P1, P2, P3, P4, P5, P6, P7) = 0;
		};

	template <typename R, typename P1, typename P2, typename P3, typename P4, typename P5, typename P6, typename P7, typename P8>
		class FunctorImpl<R, Seq<P1, P2, P3, P4, P5, P6, P7, P8> > : public FunctorImplBase<R>
		{
			public:
				typedef R ResultType;
				typedef P1 Param1;
				typedef P2 Param2;
				typedef P3 Param3;
				typedef P4 Param4;
				typedef P5 Param5;
				typedef P6 Param6;
				typedef P7 Param7;
				typedef P8 Param8;

				virtual R operator()(P1, P2, P3, P4, P5, P6, P7, P8) = 0;
		};

#define CLONE_IMPL(Cls) virtual Cls* doClone() const \
	{return FIR_NEW Cls(*this);}

	template <typename ParentFunctor, typename Fun>
		class FunctorHandler : public ParentFunctor::Impl
	{
		typedef typename ParentFunctor::Impl Base;

		public:	
		typedef typename Base::ResultType ResultType;
		typedef typename Base::Param1 Param1;
		typedef typename Base::Param2 Param2;
		typedef typename Base::Param3 Param3;
		typedef typename Base::Param4 Param4;
		typedef typename Base::Param1 Param5;
		typedef typename Base::Param2 Param6;
		typedef typename Base::Param3 Param7;
		typedef typename Base::Param4 Param8;

		FunctorHandler(const Fun& fun) : f(fun) {}

		CLONE_IMPL(FunctorHandler);

		ResultType operator()()
		{
			return f();
		}

		ResultType operator()(Param1 p1)
		{
			return f(p1);
		}

		ResultType operator()(Param1 p1, Param2 p2)
		{
			return f(p1, p2);
		}

		ResultType operator()(Param1 p1, Param2 p2, Param3 p3)
		{
			return f(p1, p2, p3);
		}

		ResultType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4)
		{
			return f(p1, p2, p3, p4);
		}


		ResultType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5)
		{
			return f(p1, p2, p3, p4, p5);
		}

		ResultType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6)
		{
			return f(p1, p2, p3, p4, p5, p6);
		}

		ResultType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7)
		{
			return f(p1, p2, p3, p4, p5, p6, p7);
		}

		ResultType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8)
		{
			return f(p1, p2, p3, p4, p5, p6, p7, p8);
		}

		Fun f;
	};


	template <typename ParentFunctor, typename Fun>
		class PointerFunHandler : public ParentFunctor::Impl
	{
		typedef typename ParentFunctor::Impl Base;

		public:	
		typedef typename Base::ResultType ResultType;
		typedef typename Base::Param1 Param1;
		typedef typename Base::Param2 Param2;
		typedef typename Base::Param3 Param3;
		typedef typename Base::Param4 Param4;
		typedef typename Base::Param1 Param5;
		typedef typename Base::Param2 Param6;
		typedef typename Base::Param3 Param7;
		typedef typename Base::Param4 Param8;

		PointerFunHandler(const Fun& fun) : f(fun) {}

		CLONE_IMPL(PointerFunHandler);

		ResultType operator()()
		{
			return (*f)();
		}

		ResultType operator()(Param1 p1)
		{
			return (*f)(p1);
		}

		ResultType operator()(Param1 p1, Param2 p2)
		{
			return (*f)(p1, p2);
		}

		ResultType operator()(Param1 p1, Param2 p2, Param3 p3)
		{
			return (*f)(p1, p2, p3);
		}

		ResultType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4)
		{
			return (*f)(p1, p2, p3, p4);
		}


		ResultType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5)
		{
			return (*f)(p1, p2, p3, p4, p5);
		}

		ResultType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6)
		{
			return (*f)(p1, p2, p3, p4, p5, p6);
		}

		ResultType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7)
		{
			return (*f)(p1, p2, p3, p4, p5, p6, p7);
		}

		ResultType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8)
		{
			return (*f)(p1, p2, p3, p4, p5, p6, p7, p8);
		}

		Fun f;
	};


	template <typename ParentFunctor, typename PointerToObj, typename MemFun>
		class MemFunHandler : public ParentFunctor::Impl
	{
		typedef typename ParentFunctor::Impl Base;

		public:	
		typedef typename Base::ResultType ResultType;
		typedef typename Base::Param1 Param1;
		typedef typename Base::Param2 Param2;
		typedef typename Base::Param3 Param3;
		typedef typename Base::Param4 Param4;
		typedef typename Base::Param1 Param5;
		typedef typename Base::Param2 Param6;
		typedef typename Base::Param3 Param7;
		typedef typename Base::Param4 Param8;

		MemFunHandler(const PointerToObj& pObj, const MemFun& fun) : obj(pObj), f(fun) {}

		CLONE_IMPL(MemFunHandler);

		ResultType operator()()
		{
			return ((*obj).*f)();
		}

		ResultType operator()(Param1 p1)
		{
			return ((*obj).*f)(p1);
		}

		ResultType operator()(Param1 p1, Param2 p2)
		{
			return ((*obj).*f)(p1, p2);
		}

		ResultType operator()(Param1 p1, Param2 p2, Param3 p3)
		{
			return ((*obj).*f)(p1, p2, p3);
		}

		ResultType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4)
		{
			return ((*obj).*f)(p1, p2, p3, p4);
		}

		ResultType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5)
		{
			return ((*obj).*f)(p1, p2, p3, p4, p5);
		}

		ResultType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6)
		{
			return ((*obj).*f)(p1, p2, p3, p4, p5, p6);
		}

		ResultType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7)
		{
			return ((*obj).*f)(p1, p2, p3, p4, p5, p6, p7);
		}

		ResultType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8)
		{
			return ((*obj).*f)(p1, p2, p3, p4, p5, p6, p7, p8);
		}

		PointerToObj obj;
		MemFun f;
	};

	template <typename R = void, typename TL = NullType>
		class Functor
		{
			public:
				typedef FunctorImpl<R, TL> Impl;
				typedef R ResultType;
				typedef TL ParamList;
				typedef typename Impl::Param1 Param1;
				typedef typename Impl::Param2 Param2;
				typedef typename Impl::Param3 Param3;
				typedef typename Impl::Param4 Param4;
				typedef typename Impl::Param5 Param5;
				typedef typename Impl::Param6 Param6;
				typedef typename Impl::Param7 Param7;
				typedef typename Impl::Param8 Param8;

				Functor() : spImpl(0) 
				{
				}

				Functor(const Functor& rhs) : spImpl(Impl::clone(rhs.spImpl.get())) 
				{
				}
				
				Functor(std::auto_ptr<Impl> spImpl) : spImpl(spImpl) {}

				template <typename Fun>
					Functor(Fun fun) : spImpl(new FunctorHandler<Functor, Fun>(fun))
				{
				}


				/**
				 * \brief 注意：Functor不负责该函数指针的释放．请调用者自己在适当的地方进行释放
				 *
				 */
				template <typename Fun>
					Functor(Fun* fun) : spImpl(new PointerFunHandler<Functor, Fun*>(fun))
				{
				}

				template <typename PtrObj, typename MemFun>
					Functor(const PtrObj& obj, MemFun fun) : spImpl(new MemFunHandler<Functor, PtrObj, MemFun>(obj, fun))
				{
				}

				Functor& operator=(const Functor& rhs)
				{
					Functor copy(rhs);
					Impl* p = spImpl.release();
					spImpl.reset(copy.spImpl.release());
					copy.spImpl.reset(p);
					return *this;
				}

				ResultType operator()() const
				{
					return (*spImpl)();
				}

				ResultType operator()(Param1 p1) const
				{
					return (*spImpl)(p1);
				}

				ResultType operator()(Param1 p1, Param2 p2) const
				{
					return (*spImpl)(p1, p2);
				}

				ResultType operator()(Param1 p1, Param2 p2, Param3 p3) const
				{
					return (*spImpl)(p1, p2, p3);
				}

				ResultType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4) const
				{
					return (*spImpl)(p1, p2, p3, p4);
				}

				ResultType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5) const
				{
					return (*spImpl)(p1, p2, p3, p4, p5);
				}

				ResultType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6) const
				{
					return (*spImpl)(p1, p2, p3, p4, p5, p6);
				}

				ResultType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7) const
				{
					return (*spImpl)(p1, p2, p3, p4, p5, p6, p7);
				}

				ResultType operator()(Param1 p1, Param2 p2, Param3 p3, Param4 p4, Param5 p5, Param6 p6, Param7 p7, Param8 p8) const
				{
					return (*spImpl)(p1, p2, p3, p4, p5, p6, p7, p8);
				}

			private:
				std::auto_ptr<Impl> spImpl;
		};
};

#endif

