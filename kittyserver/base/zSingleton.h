/**
 * \file
 * \version  $Id: zSingleton.h 62 2013-04-22 13:00:09Z chengxie $
 * \author  , @ztgame.com 
 * \date 2005年12月01日 21时24分24秒 CST
 * \brief 
 *
 * 
 */

#ifndef _SINGLETON_H
#define _SINGLETON_H

#include "Fir.h"

namespace Fir {

		template <typename T>
		class Singleton
		{
			private:
				// 禁用拷贝构造函数
				Singleton(const Singleton&);

				// 禁用赋值操作
				const Singleton & operator= (const Singleton &);
			protected:

				Singleton( void )
				{
				}

				~Singleton( void )
				{
				}

			public:

				static T& getMe(void)
				{
					static T s_singleton;
					return s_singleton;
				}

		};
}

#endif

