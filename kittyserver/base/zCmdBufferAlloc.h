#pragma once

#include <iostream>

//buffer分配策略  -  堆分配
template <class ImplBufferT >
class zCmdBufferHeapAllocT
{
	public:
		//获取一个ImplBufferT
		static ImplBufferT* create();

		//回收一个ImplBufferT
		static void dispose(ImplBufferT* pBuffer);
};

template <class ImplBufferT >
inline ImplBufferT* zCmdBufferHeapAllocT<ImplBufferT>::create()
{
	return new ImplBufferT;
}

template <class ImplBufferT>
inline void zCmdBufferHeapAllocT<ImplBufferT>::dispose(ImplBufferT* pBuffer)
{
	delete pBuffer;
	pBuffer = NULL;
}
