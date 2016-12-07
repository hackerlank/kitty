/**
 * \file
 * \version  $Id: zNoncopyable.h 13 2013-03-20 02:35:18Z  $
 * \author  ,@163.com
 * \date 2004年11月29日 11时00分18秒 CST
 * \brief 定义zNoncopyable类，使所有的子类禁用拷贝构造函数和赋值符号
 *
 * 
 */


#ifndef _zNoncopyable_h_
#define _zNoncopyable_h_

/**
 * \brief 使所有的子类禁用拷贝构造函数和赋值符号
 *
 */
class zNoncopyable
{

	protected:

		/**
		 * \brief 缺省构造函数
		 *
		 */
		zNoncopyable() {};

		/**
		 * \brief 缺省析构函数
		 *
		 */
		~zNoncopyable() {};

	private:

		/**
		 * \brief 拷贝构造函数，没有实现，禁用掉了
		 *
		 */
		zNoncopyable(const zNoncopyable&);

		/**
		 * \brief 赋值操作符号，没有实现，禁用掉了
		 *
		 */
		const zNoncopyable & operator= (const zNoncopyable &);

};

#endif

