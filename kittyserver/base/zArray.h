/**
 * \file
 * \version  $Id: zArray.h 13 2013-03-20 02:35:18Z  $
 * \author  Mark Zhong,@ztgame.com
 * \date 2009年07月29日 10时12分44秒 CST
 * \brief 定义array类， 实现静态分配的数组，并支持数越访问越界检查
 */

#include <stdexcept>
#include <memory>
#include <string>
#include "Fir.h"

template <typename X, typename Alloc = std::allocator<X> >
class array : private Alloc
{
	public:
		typedef Alloc allocator_type;
		typedef X value_type;
		typedef  X& reference;
		typedef  const X& const_reference;
		typedef  X* pointer;
		typedef  const X* const_pointer;
		typedef  const X& const_pointer;

		explicit array(const unsigned int& size, 
				alloca_type())
		{
			this->max_size = size;
			storage = alloc_impl.allocate(this->max_size+200); // 预留200个元素空间，做为溢出保护和检查
			std::uninitialized_fill_n(storage, 255, value_type());
		}

		~array()
		{
			if (storage[this->max_size]!=0)
					printf("数组有越界访问");
//				Fir::logger->error("[数组越界]: %s 访问越界", desc.c_str());
				
		}


		unsigned int size();
		unsigned int empty();

		reference operator[](const unsigned int& n)
		{
			range_check(n);
			return storage[n];
		};

		const_reference operator[](const unsigned int& n) const
		{
			range_check(n);
			return storage[n];
		}

		void  range_check(unsigned int n)
			{
				if (n >= this->get_maxsize())
					throw std::out_of_range("outofrange");
			}
		
		unsigned int get_maxsize()
		{
			return max_size;
		}
			
		allocator_type alloc_impl;
		pointer storage;
	private:
		unsigned int max_size;
		std::string desc;
};


/*
template<typename X>
unsigned int array<X>::max_size = 0;
*/


