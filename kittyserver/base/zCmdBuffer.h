#pragma once

#include "zCmdBufferAlloc.h"
#include "zSocket.h"
#include "zMisc.h"

class zCmdBuffer
{
	public:
		//ctor
		zCmdBuffer();

		//dtor
		virtual ~zCmdBuffer();

	public:
		//获取有效数据大小
		const unsigned int size() const;

		//重设缓冲区
		void reset();

		//获取有效数据首地址
		const unsigned char* data() const;

		//写数据,若缓冲区不足则返回false
		bool write(const void* data, const unsigned int size);

		//在有效数据的尾部构造对象,返回指向添加后的对象指针,缓冲区不足返回NULL
		template < class TailT >
			TailT* constructTail();

		//尾部是否有足够大小
		bool tailAvailable(const unsigned int size) const;

		//尾部指针向高地址移动
		bool tailForward(const unsigned int size);

		//尾部指针向低地址移动
		bool tailBackward(const unsigned int size);

		//重载operator =
		zCmdBuffer& operator = (const zCmdBuffer& other);

		//回收
		virtual void dispose() = 0;

	private:
		//尾部缓冲区大小
		const unsigned int tailBufferSize() const;

		//获取有效数据首地址指针
		unsigned char* data();

		//有效数据首游标
		unsigned int headPtr;

		//有效数据尾游标
		unsigned int tailPtr;

		//缓冲区
		unsigned char buffer[zSocket::MAX_DATASIZE];
};

//zCmdBuffer -- 堆分配
class zCmdHeapBuffer : public zCmdBuffer
{
	public:
		typedef zCmdBufferHeapAllocT< zCmdHeapBuffer > AllocT;
		inline static zCmdHeapBuffer* create() { return AllocT::create(); }
		virtual void dispose() { AllocT::dispose(this); }
};

//zCmdBuffer -- 栈分配
class zCmdStackBuffer : public zCmdBuffer
{
	virtual void dispose() {}
};

//zCmdBufferAdapter
template < class CmdT, class ImplBufferT = zCmdStackBuffer>
class zCmdBufferAdapterT
{
	public:
		typedef CmdT CmdType;

		//ctor
		zCmdBufferAdapterT();

		//dtor
		~zCmdBufferAdapterT();
	public:
		//--------------转调pBuffer的成员---------------------

		//获取有效数据大小
		inline const unsigned int size() const
		{
			return this->pBuffer->size();
		}

		//获取有效数据首地址指针
		const void* data() const
		{
			return const_cast<const ImplBufferT*>(this->pBuffer)->data();
		}

		//写数据,若缓冲区不足则返回false
		inline bool write(const void* pData, const unsigned int size)
		{
			return this->pBuffer->write(pData, size);
		}

		//在有效数据的尾部构造对象,返回指向添加后的对象指针,缓冲区不足返回NULL
		template < class TailT >
			inline TailT* constructTail()
			{
				return this->pBuffer->constructTail<TailT>();
			}

		//---------------自身成员-----------------------
		//重载operator->
		inline CmdT* operator->()
		{
			return this->pCmd;
		}

		//重载operator->
		inline const CmdT* operator->() const
		{
			return this->pCmd;
		}

		//重载类型转换
		inline operator const CmdT*() const
		{
			return this->pCmd;
		}

		//重载类型转换
		inline operator CmdT*()
		{
			return this->pCmd;
		}

		//获得实际的buffer
		inline ImplBufferT* getBuffer()
		{
			return this->pBuffer;
		}

		//尾部指针向高地址移动
		inline bool tailForward(const unsigned int size)
		{
			return this->pBuffer->tailForward(size);
		}

		//尾部指针向低地址移动
		inline bool tailBackward(const unsigned int size)
		{
			return this->pBuffer->tailBackward(size);
		}

		//尾部是否有足够大小
		inline bool tailAvailable(const unsigned int size) const
		{
			return this->pBuffer->tailAvailable(size);
		}

		//获得实际的Buffer
		inline const ImplBufferT* getBuffer() const
		{
			return this->pBuffer;
		}
	private:
		//实现类
		ImplBufferT *pBuffer;

		//命令
		CmdT *pCmd;
};

//zCmdBufferAdapter,针对栈分配特化
template < class CmdT >
class zCmdBufferAdapterT< CmdT, zCmdStackBuffer >
{
	public:
		//ctor
		zCmdBufferAdapterT()
		{
			this->pCmd = this->buffer.constructTail<CmdT>();
		}

	public:
		//--------------转调pBuffer的成员---------------------

		//获取有效数据大小
		inline const unsigned int size() const
		{
			return this->buffer.size();
		}

		//获取有效数据首地址指针
		const void* data() const
		{
			return this->buffer.data();
		}

		//写数据,若缓冲区不足则返回false
		inline bool write(const void* pData, const unsigned int size)
		{
			return this->buffer.write(pData, size);
		}

		//在有效数据的尾部构造对象,返回指向添加后的对象指针,缓冲区不足返回NULL
		template < class TailT >
			inline TailT* constructTail()
			{
				return this->buffer.constructTail<TailT>();
			}

		//---------------自身成员-----------------------
		//重载operator->
		inline CmdT* operator->()
		{
			return this->pCmd;
		}

		//重载operator->
		inline const CmdT* operator->() const
		{
			return this->pCmd;
		}

		//重载类型转换
		inline operator const CmdT*() const
		{
			return this->pCmd;
		}

		//重载类型转换
		inline operator CmdT*()
		{
			return this->pCmd;
		}

		//获得实际的buffer
		inline zCmdStackBuffer* getBuffer()
		{
			return &this->buffer;
		}

		//获得实际的Buffer
		inline const zCmdStackBuffer* getBuffer() const
		{
			return &this->buffer;
		}

		//尾部指针向高地址移动
		inline bool tailForward(const unsigned int size)
		{
			return this->buffer.tailForward(size);
		}

		//尾部指针向低地址移动
		inline bool tailBackward(const unsigned int size)
		{
			return this->buffer.tailBackward(size);
		}

		//尾部是否有足够大小
		inline bool tailAvailable(const unsigned int size) const
		{
			return this->buffer.tailAvailable(size);
		}

		//重设缓冲区
		inline void reset()
		{
			this->buffer.reset();
		}
	private:
		//实现类
		zCmdStackBuffer buffer;

		//命令
		CmdT *pCmd;
};
//--------------------impl-----------------------

inline zCmdBuffer::zCmdBuffer() : headPtr(0),tailPtr(0)
{
	bzero(this->buffer, sizeof(this->buffer));
}

inline zCmdBuffer::~zCmdBuffer()
{
}

inline unsigned char* zCmdBuffer::data()
{
	return this->buffer + this->headPtr;
}

inline const unsigned char* zCmdBuffer::data() const
{
	return this->buffer + this->headPtr;
}

inline const unsigned int zCmdBuffer::size() const
{
	return this->tailPtr - this->headPtr;
}

inline void zCmdBuffer::reset()
{
	bzero(this->buffer, sizeof(this->buffer));
	this->headPtr = 0;
	this->tailPtr = 0;
}

inline bool zCmdBuffer::tailAvailable(const unsigned int size) const
{
	return (this->size() + size <= sizeof(this->buffer))
		&& (sizeof(this->buffer) - this->tailPtr >= size);
}

inline const unsigned int zCmdBuffer::tailBufferSize() const
{
	return sizeof(this->buffer) - this->tailPtr;
}

inline bool zCmdBuffer::tailForward(const unsigned int size)
{
	if (!tailAvailable(size))
		return false;

	this->tailPtr += size;
	return true;
}

inline bool zCmdBuffer::tailBackward(const unsigned int size)
{
	if (size > this->tailPtr - this->headPtr)
		return false;

	this->tailPtr -= size;
	return true;
}

template < class TailT >
inline TailT* zCmdBuffer::constructTail()
{
	if (!this->tailAvailable(sizeof(TailT)))
		return NULL;

	TailT* pTail = reinterpret_cast<TailT*>(&this->buffer[this->tailPtr]);
	constructInPlace(pTail);
	this->tailPtr += sizeof(TailT);
	return pTail;
}

template < class CmdT, class ImplBufferT >
inline zCmdBufferAdapterT<CmdT, ImplBufferT>::zCmdBufferAdapterT()
{
	this->pBuffer = ImplBufferT::create();
	this->pCmd = this->pBuffer->constructTail<CmdT>();
}

template < class CmdT, class ImplBufferT >
inline zCmdBufferAdapterT<CmdT, ImplBufferT>::~zCmdBufferAdapterT()
{
	this->pBuffer->dispose();
}
