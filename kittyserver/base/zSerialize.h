#ifndef _SERIALIZE_H
#define _SERIALIZE_H
#include <string>
#include <string.h>
#include <map>
#include <vector>
#include <set>
#include <deque>
#include <list>
#include "zType.h"
#include "zFirNew.h"
#include "zNoncopyable.h"

enum zSerializeOP
{
	SERIALIZE_SAVE, 	//序列化 保存
	SERIALIZE_LOAD,		//序列化 加载
};

enum zArchiveHead
{
	ARCHIVE_HEAD,		//有档案头
	ARCHIVE_COUNT,		//只有数目
	ARCHIVE_VERSION,	//只有版本号
	ARCHIVE_NOHEAD,		//没有档案头
};

#define MEMBER_MAXNUM 65535

/*
 *\brief 二进制档案模板
 *	这里档案不是通用的，每种档案只存一种可序列化对象，要存储不同的对象需要实例化不同的档案类。
 *	档案可以有档案头（包括版本和对象个数信息）。
 *	一个档案不能同时进行输入输出
 */
template <typename _SerializableT>
class zArchive : private zNoncopyable
{
public:
	typedef _SerializableT SerializeT;
	typedef zArchive<SerializeT> ArchiveT;
	friend class CSerializeManager;

private:
	friend bool _SerializableT::_Serialize(ArchiveT& archive);

	DWORD _count;
	
	zArchiveHead headType;

	DWORD *count;
	DWORD version;
	DWORD head_size;

	char* buf;			//缓存
	size_t __curPos;
	size_t &curPos;		//缓存位置

	zSerializeOP operation;

	std::map<DWORD,void*> params_p;		//用于序列化的时候传递参数
	std::map<DWORD,QWORD> params_n;		

	/*
	 *\brief 在序列化函数中可以获取当前的操作
	 */
	zSerializeOP getOperation() const
	{
		return operation;
	}

#define _SERIALIZE_BASETYPE(T,n)\
	if(operation == SERIALIZE_SAVE) \
		*((T*)(buf + curPos)) = n; \
	else \
		n = *((T*)(buf + curPos)); \
	curPos += sizeof(T); \
	return *this;

#define _SERIALIZE_BASETYPE_CONST(T,n) \
	if(operation == SERIALIZE_SAVE) \
		*((T*)(buf + curPos)) = n; \
	return *this;

#define __SERIALIZE_BASETYPE_FUNCS(T) \
	ArchiveT& operator &(T& n) { \
		_SERIALIZE_BASETYPE(T,n) \
	} \
	ArchiveT& operator <<(const T& n) { \
		_SERIALIZE_BASETYPE_CONST(T,n) \
	}

	/*
	 *\brief 基本数据的序列化函数，直接赋值
	 */
	__SERIALIZE_BASETYPE_FUNCS(char)

	__SERIALIZE_BASETYPE_FUNCS(BYTE)
	
	__SERIALIZE_BASETYPE_FUNCS(bool)

	__SERIALIZE_BASETYPE_FUNCS(short)
	
	__SERIALIZE_BASETYPE_FUNCS(WORD)
	
	__SERIALIZE_BASETYPE_FUNCS(int)
	
	__SERIALIZE_BASETYPE_FUNCS(DWORD)
	
	__SERIALIZE_BASETYPE_FUNCS(long)

	__SERIALIZE_BASETYPE_FUNCS(QWORD)

	__SERIALIZE_BASETYPE_FUNCS(long long)

	__SERIALIZE_BASETYPE_FUNCS(unsigned long long)
	
	__SERIALIZE_BASETYPE_FUNCS(float)

	__SERIALIZE_BASETYPE_FUNCS(double)

	__SERIALIZE_BASETYPE_FUNCS(long double)

	/*
	 *\brief 序列化结构，数组。直接内存拷贝。
	 */
	void Serialize(void* _buf,size_t size)
	{
		if(operation == SERIALIZE_SAVE)
			bcopy(_buf,buf + curPos,size);
		else
			bcopy(buf + curPos,_buf,size);
		curPos += size;
	}

	ArchiveT& operator <<(const std::string& str)
	{
		if(operation == SERIALIZE_SAVE)
		{
			*((DWORD*)(buf + curPos)) = str.length();
			curPos += sizeof(DWORD);
			strncpy(buf + curPos,str.c_str(),str.length());
			curPos += str.length();
		}
		return *this;
	}

	/*
	 *\brief 序列化基本类型，结构体，数组
	 */
	template <typename T>
	ArchiveT& operator &(T& t)
	{
		if(operation == SERIALIZE_SAVE)
		{
			bcopy(&t, &buf[curPos], sizeof(T));
			curPos += sizeof(T);		
		}
		else
		{
			bcopy(&buf[curPos], &t, sizeof(T));
			curPos += sizeof(T);
		}
		return *this;
	}

	/*
	 *\brief 序列化字符串。所需空间为字符串长度加4。（特化）
	 */
	ArchiveT& operator &(std::string& str)
	{
		DWORD len = 0;
		// 这里为了兼容以前的版本，采用以前存储方式(长度+内容)
		if(operation == SERIALIZE_SAVE)
		{
			len = str.length();
			*((DWORD*)(buf + curPos)) = len;
			curPos += sizeof(DWORD);
			strncpy(buf + curPos,str.c_str(),len);
		}
		else
		{
			len = *((DWORD*)(buf + curPos));
			curPos += sizeof(DWORD);
			str.assign(buf + curPos,len);
		}
		curPos += len;
		return *this;
	}

	/*
	 *\brief 序列化容器 vec。（特化）
	 */
	template <typename T>
	ArchiveT& operator &(std::vector<T>& cont)
	{
		typedef typename std::vector<T>::value_type value_type;
		typedef typename std::vector<T>::iterator   iterator;
		size_t   size = 0;
		if(operation == SERIALIZE_SAVE)
		{
			size = cont.size();
			*((size_t*)(buf + curPos)) = size;
			curPos += sizeof(size_t);
			iterator it = cont.begin();
			for (; it != cont.end(); ++it)
			{
				bcopy(&(*it), &buf[curPos], sizeof(value_type));
				curPos += sizeof(value_type);
			}			
		}
		else
		{
			size = *((size_t*)(buf + curPos));
			curPos += sizeof(size_t);
			for (size_t i = 0; i < size; ++i)
			{
				cont.insert(cont.end(), *(value_type*)&buf[curPos]);
				curPos += sizeof(value_type);
			}
		}
		return *this;
	}

	/*
	 *\brief 序列化容器 list。（特化）
	 */
	template <typename T>
	ArchiveT& operator &(std::list<T>& cont)
	{
		typedef typename std::list<T>::value_type value_type;
		typedef typename std::list<T>::iterator   iterator;
		size_t   size = 0;
		if(operation == SERIALIZE_SAVE)
		{
			size = cont.size();
			*((size_t*)(buf + curPos)) = size;
			curPos += sizeof(size_t);
			iterator it = cont.begin();
			for (; it != cont.end(); ++it)
			{
				bcopy(&(*it), &buf[curPos], sizeof(value_type));
				curPos += sizeof(value_type);
			}			
		}
		else
		{
			size = *((size_t*)(buf + curPos));
			curPos += sizeof(size_t);
			for (size_t i = 0; i < size; ++i)
			{
				cont.insert(cont.end(), *(value_type*)&buf[curPos]);
				curPos += sizeof(value_type);
			}
		}
		return *this;
	}

	/*
	 *\brief 序列化容器 set。（特化）
	 */
	template <typename T>
	ArchiveT& operator &(std::set<T>& cont)
	{
		typedef typename std::set<T>::value_type value_type;
		typedef typename std::set<T>::iterator   iterator;
		size_t   size = 0;
		if(operation == SERIALIZE_SAVE)
		{
			size = cont.size();
			*((size_t*)(buf + curPos)) = size;
			curPos += sizeof(size_t);
			iterator it = cont.begin();
			for (; it != cont.end(); ++it)
			{
				bcopy(&(*it), &buf[curPos], sizeof(value_type));
				curPos += sizeof(value_type);
			}			
		}
		else
		{
			size = *((size_t*)(buf + curPos));
			curPos += sizeof(size_t);
			for (size_t i = 0; i < size; ++i)
			{
				cont.insert(cont.end(), *(value_type*)&buf[curPos]);
				curPos += sizeof(value_type);
			}
		}
		return *this;
	}

	/*
	 *\brief 序列化容器 deque。（特化）
	 */
	template <typename T>
	ArchiveT& operator &(std::deque<T>& cont)
	{
		typedef typename std::deque<T>::value_type value_type;
		typedef typename std::deque<T>::iterator   iterator;
		size_t   size = 0;
		if(operation == SERIALIZE_SAVE)
		{
			size = cont.size();
			*((size_t*)(buf + curPos)) = size;
			curPos += sizeof(size_t);
			iterator it = cont.begin();
			for (; it != cont.end(); ++it)
			{
				bcopy(&(*it), &buf[curPos], sizeof(value_type));
				curPos += sizeof(value_type);
			}			
		}
		else
		{
			size = *((size_t*)(buf + curPos));
			curPos += sizeof(size_t);
			for (size_t i = 0; i < size; ++i)
			{
				cont.insert(cont.end(), *(value_type*)&buf[curPos]);
				curPos += sizeof(value_type);
			}
		}
		return *this;
	}

	/*
	 *\brief 序列化容器 map。（特化）
	 */
	template <typename T1, typename T2, typename Compare>
	ArchiveT& operator &(std::map<T1, T2, Compare>& cont)
	{
		typedef typename std::map<T1, T2, Compare>::value_type value_type;
		typedef typename std::map<T1, T2, Compare>::iterator   iterator;

		size_t   size = 0;
		if(operation == SERIALIZE_SAVE)
		{
			size = cont.size();
			*((size_t*)(buf + curPos)) = size;
			curPos += sizeof(size_t);
			iterator it = cont.begin();
			for (; it != cont.end(); ++it)
			{
				bcopy(&(it->first), &buf[curPos], sizeof(T1));
				curPos += sizeof(T1);

				bcopy(&(it->second), &buf[curPos], sizeof(T2));
				curPos += sizeof(T2);
			}			
		}
		else
		{
			size = *((size_t*)(buf + curPos));
			curPos += sizeof(size_t);
			for (size_t i = 0; i < size; ++i)
			{
				cont.insert(std::make_pair(*(T1*)&buf[curPos], *(T2*)&buf[curPos + sizeof(T1)]));
				curPos += sizeof(T1) + sizeof(T2);
			}
		}
		return *this;
	}

	/*
	 *\brief 序列化基本类型，结构体，数组
	 */
	template <typename T>
	DWORD serialize(WORD id, T& t, bool isNeed=true)
	{
		if(operation == SERIALIZE_SAVE)
		{
			if (!isNeed) return curPos;
			*((WORD*)(buf + curPos)) = id;
			curPos += sizeof(WORD);
			bcopy(&t, &buf[curPos], sizeof(T));
			curPos += sizeof(T);
		}
		else
		{
			WORD saveID = *((WORD*)(buf + curPos));
			if (saveID != id) return curPos;
			curPos += sizeof(WORD);
			if (isNeed)
				bcopy(&buf[curPos], &t, sizeof(T));
			curPos += sizeof(T);
		}
		return curPos;
	}

	/*
	 *\brief 序列化字符串。所需空间为字符串长度加4。（特化）
	 */
	DWORD serialize(WORD id, std::string& str, bool isNeed=true)
	{
		DWORD len = 0;
		// 这里为了兼容以前的版本，采用以前存储方式(长度+内容)
		if(operation == SERIALIZE_SAVE)
		{
			if (!isNeed) return curPos;
			*((WORD*)(buf + curPos)) = id;
			curPos += sizeof(WORD);

			len = str.length();
			*((DWORD*)(buf + curPos)) = len;
			curPos += sizeof(DWORD);
			strncpy(buf + curPos,str.c_str(),len);
		}
		else
		{
			WORD saveID = *((WORD*)(buf + curPos));
			if (saveID != id) return curPos;
			curPos += sizeof(WORD);

			len = *((DWORD*)(buf + curPos));
			curPos += sizeof(DWORD);
			if (isNeed)
				str.assign(buf + curPos,len);
		}
		curPos += len;
		return curPos;
	}
	template<typename T>
	DWORD serialize(WORD id,T *value,size_t size ,bool isNeed = true)
	{
		if(operation == SERIALIZE_SAVE)
		{
			if (!isNeed) return curPos;
			*((WORD*)(buf + curPos)) = id;
			curPos += sizeof(WORD);
			bcopy(value, &buf[curPos], size);
			curPos += size;
		}
		else
		{
			WORD saveID = *((WORD*)(buf + curPos));
			if (saveID != id) return curPos;
			curPos += sizeof(WORD);
			if (isNeed)
				bcopy(&buf[curPos], value, size);
			curPos += size;
		}
		return curPos;

	}
	/*
	 *\brief 序列化容器 vec。（特化）
	 */
	template <typename T>
	DWORD serialize(WORD id, std::vector<T>& cont, bool isNeed=true)
	{
		typedef typename std::vector<T>::value_type value_type;
		typedef typename std::vector<T>::iterator   iterator;
		size_t   size = 0;
		if(operation == SERIALIZE_SAVE)
		{
			if (!isNeed) return curPos;
			*((WORD*)(buf + curPos)) = id;
			curPos += sizeof(WORD);

			size = cont.size();
			*((size_t*)(buf + curPos)) = size;
			curPos += sizeof(size_t);
			iterator it = cont.begin();
			for (; it != cont.end(); ++it)
			{
				bcopy(&(*it), &buf[curPos], sizeof(value_type));
				curPos += sizeof(value_type);
			}
		}
		else
		{
			WORD saveID = *((WORD*)(buf + curPos));
			if (saveID != id) return curPos;
			curPos += sizeof(WORD);

			size = *((size_t*)(buf + curPos));
			curPos += sizeof(size_t);
			for (size_t i = 0; i < size; ++i)
			{
				if (isNeed)
					cont.insert(cont.end(), *(value_type*)&buf[curPos]);
				curPos += sizeof(value_type);
			}
		}
		return curPos;
	}

	/*
	 *\brief 序列化容器 list。（特化）
	 */
	template <typename T>
	DWORD serialize(WORD id, std::list<T>& cont, bool isNeed=true)
	{
		typedef typename std::list<T>::value_type value_type;
		typedef typename std::list<T>::iterator   iterator;
		size_t   size = 0;
		if(operation == SERIALIZE_SAVE)
		{
			if (!isNeed) return curPos;
			*((WORD*)(buf + curPos)) = id;
			curPos += sizeof(WORD);

			size = cont.size();
			*((size_t*)(buf + curPos)) = size;
			curPos += sizeof(size_t);
			iterator it = cont.begin();
			for (; it != cont.end(); ++it)
			{
				bcopy(&(*it), &buf[curPos], sizeof(value_type));
				curPos += sizeof(value_type);
			}
		}
		else
		{
			WORD saveID = *((WORD*)(buf + curPos));
			if (saveID != id) return curPos;
			curPos += sizeof(WORD);

			size = *((size_t*)(buf + curPos));
			curPos += sizeof(size_t);
			for (size_t i = 0; i < size; ++i)
			{
				if (isNeed)
					cont.insert(cont.end(), *(value_type*)&buf[curPos]);
				curPos += sizeof(value_type);
			}
		}
		return curPos;
	}

	/*
	 *\brief 序列化容器 set。（特化）
	 */
	template <typename T>
	DWORD serialize(WORD id, std::set<T>& cont, bool isNeed=true)
	{
		typedef typename std::set<T>::value_type value_type;
		typedef typename std::set<T>::iterator   iterator;
		size_t   size = 0;
		if(operation == SERIALIZE_SAVE)
		{
			if (!isNeed) return curPos;
			*((WORD*)(buf + curPos)) = id;
			curPos += sizeof(WORD);

			size = cont.size();
			*((size_t*)(buf + curPos)) = size;
			curPos += sizeof(size_t);
			iterator it = cont.begin();
			for (; it != cont.end(); ++it)
			{
				bcopy(&(*it), &buf[curPos], sizeof(value_type));
				curPos += sizeof(value_type);
			}
		}
		else
		{
			WORD saveID = *((WORD*)(buf + curPos));
			if (saveID != id) return curPos;
			curPos += sizeof(WORD);

			size = *((size_t*)(buf + curPos));
			curPos += sizeof(size_t);
			for (size_t i = 0; i < size; ++i)
			{
				if (isNeed)
					cont.insert(cont.end(), *(value_type*)&buf[curPos]);
				curPos += sizeof(value_type);
			}
		}
		return curPos;
	}

	/*
	 *\brief 序列化容器 deque。（特化）
	 */
	template <typename T>
	DWORD serialize(WORD id, std::deque<T>& cont, bool isNeed=true)
	{
		typedef typename std::deque<T>::value_type value_type;
		typedef typename std::deque<T>::iterator   iterator;
		size_t   size = 0;
		if(operation == SERIALIZE_SAVE)
		{
			if (!isNeed) return curPos;
			*((WORD*)(buf + curPos)) = id;
			curPos += sizeof(WORD);

			size = cont.size();
			*((size_t*)(buf + curPos)) = size;
			curPos += sizeof(size_t);
			iterator it = cont.begin();
			for (; it != cont.end(); ++it)
			{
				bcopy(&(*it), &buf[curPos], sizeof(value_type));
				curPos += sizeof(value_type);
			}
		}
		else
		{
			WORD saveID = *((WORD*)(buf + curPos));
			if (saveID != id) return curPos;
			curPos += sizeof(WORD);

			size = *((size_t*)(buf + curPos));
			curPos += sizeof(size_t);
			for (size_t i = 0; i < size; ++i)
			{
				if (isNeed)
					cont.insert(cont.end(), *(value_type*)&buf[curPos]);
				curPos += sizeof(value_type);
			}
		}
		return curPos;
	}

	/*
	 *\brief 序列化容器 map。（特化）
	 */
	template <typename T1, typename T2, typename Compare>
	DWORD serialize(WORD id, std::map<T1, T2, Compare>& cont, bool isNeed=true)
	{
		typedef typename std::map<T1, T2, Compare>::value_type value_type;
		typedef typename std::map<T1, T2, Compare>::iterator   iterator;

		size_t   size = 0;
		if(operation == SERIALIZE_SAVE)
		{
			if (!isNeed) return curPos;
			*((WORD*)(buf + curPos)) = id;
			curPos += sizeof(WORD);

			size = cont.size();
			*((size_t*)(buf + curPos)) = size;
			curPos += sizeof(size_t);
			iterator it = cont.begin();
			for (; it != cont.end(); ++it)
			{
				bcopy(&(it->first), &buf[curPos], sizeof(T1));
				curPos += sizeof(T1);

				bcopy(&(it->second), &buf[curPos], sizeof(T2));
				curPos += sizeof(T2);
			}			
		}
		else
		{
			WORD saveID = *((WORD*)(buf + curPos));
			if (saveID != id) return curPos;
			curPos += sizeof(WORD);

			size = *((size_t*)(buf + curPos));
			curPos += sizeof(size_t);
			for (size_t i = 0; i < size; ++i)
			{
				if (isNeed)
					cont.insert(std::make_pair(*(T1*)&buf[curPos], *(T2*)&buf[curPos + sizeof(T1)]));
				curPos += sizeof(T1) + sizeof(T2);
			}
		}
		return curPos;
	}

	/*
	 *\brief 序列化容器 一级map，二级容器为vector（特化）
	 */
	template <typename T1, typename T2, typename Compare>
	DWORD serialize(WORD id, std::map<T1, std::vector<T2>, Compare>& cont, bool isNeed=true)
	{
		typedef typename std::map<T1, std::vector<T2>, Compare>::value_type value_type;
		typedef typename std::map<T1, std::vector<T2>, Compare>::iterator   iterator;
		typedef typename std::vector<T2>::value_type value_type_2;
		typedef typename std::vector<T2>::iterator iterator_2;

		size_t   size = 0;
		size_t   size_2 = 0;
		if(operation == SERIALIZE_SAVE)
		{
			if (!isNeed) return curPos;
			*((WORD*)(buf + curPos)) = id;
			curPos += sizeof(WORD);

			size = cont.size();
			*((size_t*)(buf + curPos)) = size;
			curPos += sizeof(size_t);
			iterator it = cont.begin();
			for (; it != cont.end(); ++it)
			{
				bcopy(&(it->first), &buf[curPos], sizeof(T1));
				curPos += sizeof(T1);
				
				size_2 = it->second.size();
				*((size_t*)(buf + curPos)) = size_2;
				curPos += sizeof(size_t);
				iterator_2 it_2 = it->second.begin();
				for (; it_2 != it->second.end(); ++it_2)
				{
					bcopy(&(*it_2), &buf[curPos], sizeof(value_type_2));
					curPos += sizeof(value_type_2);
				}
			}			
		}
		else
		{
			WORD saveID = *((WORD*)(buf + curPos));
			if (saveID != id) return curPos;
			curPos += sizeof(WORD);

			size = *((size_t*)(buf + curPos));
			curPos += sizeof(size_t);
			for (size_t i = 0; i < size; ++i)
			{
				T1 key;
				bcopy(&buf[curPos], &key, sizeof(T1));
				curPos += sizeof(T1);

				size_2 = *((size_t*)(buf + curPos));
				curPos += sizeof(size_t);
				for (size_t j = 0; j < size_2; ++j)
				{
					if (isNeed)
						cont[key].insert(cont[key].end(), *(value_type_2*)&buf[curPos]);
					curPos += sizeof(value_type_2);
				}
			}
		}
		return curPos;
	}

	/*
	 *\brief 序列化容器 一级map，二级容器为map（特化）
	 */
	template <typename T1, typename T2, typename T3, typename Compare, typename Compare2>
	DWORD serialize(WORD id, std::map<T1, std::map<T2, T3, Compare2>, Compare>& cont, bool isNeed=true)
	{
		typedef typename std::map<T1, std::map<T2, T3>, Compare2>::value_type value_type;
		typedef typename std::map<T1, std::map<T2, T3>, Compare2>::iterator   iterator;
		typedef typename std::map<T2, T3, Compare2>::value_type value_type_2;
		typedef typename std::map<T2, T3, Compare2>::iterator iterator_2;

			size_t   size = 0;
			size_t   size_2 = 0;
			if(operation == SERIALIZE_SAVE)
			{
				if (!isNeed) return curPos;
				*((WORD*)(buf + curPos)) = id;
				curPos += sizeof(WORD);

				size = cont.size();
				*((size_t*)(buf + curPos)) = size;
				curPos += sizeof(size_t);
				iterator it = cont.begin();
				for (; it != cont.end(); ++it)
				{
					bcopy(&(it->first), &buf[curPos], sizeof(T1));
					curPos += sizeof(T1);
					
					size_2 = it->second.size();
					*((size_t*)(buf + curPos)) = size_2;
					curPos += sizeof(size_t);
					iterator_2 it_2 = it->second.begin();
					for (; it_2 != it->second.end(); ++it_2)
					{
						bcopy(&(it_2->first), &buf[curPos], sizeof(T2));
						curPos += sizeof(T2);

						bcopy(&(it_2->second), &buf[curPos], sizeof(T3));
						curPos += sizeof(T3);
					}
				}			
			}
			else
			{
				WORD saveID = *((WORD*)(buf + curPos));
				if (saveID != id) return curPos;
				curPos += sizeof(WORD);

				size = *((size_t*)(buf + curPos));
				curPos += sizeof(size_t);
				for (size_t i = 0; i < size; ++i)
				{
					T1 key;
					bcopy(&buf[curPos], &key, sizeof(T1));
					curPos += sizeof(T1);

					size_2 = *((size_t*)(buf + curPos));
					curPos += sizeof(size_t);
					for (size_t j = 0; j < size_2; ++j)
					{
						if (isNeed)
							cont[key].insert(std::make_pair(*(T2*)&buf[curPos], *(T3*)&buf[curPos + sizeof(T2)]));
						curPos += sizeof(T2) + sizeof(T3);
					}
				}
			}
			return curPos;
		}

		/*
		 *\brief 序列化容器 一级map，二级容器为list特化）
		 */
		template <typename T1, typename T2, typename Compare>
		DWORD serialize(WORD id, std::map<T1, std::list<T2>, Compare>& cont, bool isNeed=true)
		{
			typedef typename std::map<T1, std::list<T2>, Compare>::value_type value_type;
			typedef typename std::map<T1, std::list<T2>, Compare>::iterator   iterator;
			typedef typename std::list<T2>::value_type value_type_2;
			typedef typename std::list<T2>::iterator iterator_2;

			size_t   size = 0;
			size_t   size_2 = 0;
			if(operation == SERIALIZE_SAVE)
			{
				if (!isNeed) return curPos;
				*((WORD*)(buf + curPos)) = id;
				curPos += sizeof(WORD);

				size = cont.size();
				*((size_t*)(buf + curPos)) = size;
				curPos += sizeof(size_t);
				iterator it = cont.begin();
				for (; it != cont.end(); ++it)
				{
					bcopy(&(it->first), &buf[curPos], sizeof(T1));
					curPos += sizeof(T1);
					
					size_2 = it->second.size();
					*((size_t*)(buf + curPos)) = size_2;
					curPos += sizeof(size_t);
					iterator_2 it_2 = it->second.begin();
					for (; it_2 != it->second.end(); ++it_2)
					{
						bcopy(&(*it_2), &buf[curPos], sizeof(value_type_2));
						curPos += sizeof(value_type_2);
					}
				}			
			}
			else
			{
				WORD saveID = *((WORD*)(buf + curPos));
				if (saveID != id) return curPos;
				curPos += sizeof(WORD);

				size = *((size_t*)(buf + curPos));
				curPos += sizeof(size_t);
				for (size_t i = 0; i < size; ++i)
				{
					T1 key;
					bcopy(&buf[curPos], &key, sizeof(T1));
					curPos += sizeof(T1);

					size_2 = *((size_t*)(buf + curPos));
					curPos += sizeof(size_t);
					for (size_t j = 0; j < size_2; ++j)
					{
						if (isNeed)
							cont[key].insert(cont[key].end(), *(value_type_2*)&buf[curPos]);
						curPos += sizeof(value_type_2);
					}
				}
			}
			return curPos;
		}

		DWORD serializeEnd()
		{
			if(operation == SERIALIZE_SAVE)
			{
				*((WORD*)(buf + curPos)) = MEMBER_MAXNUM;
				curPos += sizeof(WORD);
			}
			else
			{
				curPos += sizeof(WORD);
			}
			return curPos;
		}

		/*
		 *\brief 序列化容器 一级map，二级容器为deque特化）
		 */
		template <typename T1, typename T2, typename Compare>
		DWORD serialize(WORD id, std::map<T1, std::deque<T2>, Compare>& cont, bool isNeed=true)
		{
			typedef typename std::map<T1, std::deque<T2>, Compare>::value_type value_type;
			typedef typename std::map<T1, std::deque<T2>, Compare>::iterator   iterator;
			typedef typename std::deque<T2>::value_type value_type_2;
			typedef typename std::deque<T2>::iterator iterator_2;

			size_t   size = 0;
			size_t   size_2 = 0;
			if(operation == SERIALIZE_SAVE)
			{
				if (!isNeed) return curPos;
				*((WORD*)(buf + curPos)) = id;
				curPos += sizeof(WORD);

				size = cont.size();
				*((size_t*)(buf + curPos)) = size;
				curPos += sizeof(size_t);
				iterator it = cont.begin();
				for (; it != cont.end(); ++it)
				{
					bcopy(&(it->first), &buf[curPos], sizeof(T1));
					curPos += sizeof(T1);
					
					size_2 = it->second.size();
					*((size_t*)(buf + curPos)) = size_2;
					curPos += sizeof(size_t);
					iterator_2 it_2 = it->second.begin();
					for (; it_2 != it->second.end(); ++it_2)
					{
						bcopy(&(*it_2), &buf[curPos], sizeof(value_type_2));
						curPos += sizeof(value_type_2);
					}
				}			
			}
			else
			{
				WORD saveID = *((WORD*)(buf + curPos));
				if (saveID != id) return curPos;
				curPos += sizeof(WORD);

				size = *((size_t*)(buf + curPos));
				curPos += sizeof(size_t);
				for (size_t i = 0; i < size; ++i)
				{
					T1 key;
					bcopy(&buf[curPos], &key, sizeof(T1));
					curPos += sizeof(T1);

					size_2 = *((size_t*)(buf + curPos));
					curPos += sizeof(size_t);
					for (size_t j = 0; j < size_2; ++j)
					{
						if (isNeed)
							cont[key].insert(cont[key].end(), *(value_type_2*)&buf[curPos]);
						curPos += sizeof(value_type_2);
					}
				}
			}
			return curPos;
		}


		/*
		 *\brief 序列化容器 一级map，二级容器为set特化）
		 */
		template <typename T1, typename T2, typename Compare>
		DWORD serialize(WORD id, std::map<T1, std::set<T2>, Compare>& cont, bool isNeed=true)
		{
			typedef typename std::map<T1, std::set<T2>, Compare>::value_type value_type;
			typedef typename std::map<T1, std::set<T2>, Compare>::iterator   iterator;
			typedef typename std::set<T2>::value_type value_type_2;
			typedef typename std::set<T2>::iterator iterator_2;

			size_t   size = 0;
			size_t   size_2 = 0;
			if(operation == SERIALIZE_SAVE)
			{
				if (!isNeed) return curPos;
				*((WORD*)(buf + curPos)) = id;
				curPos += sizeof(WORD);

				size = cont.size();
				*((size_t*)(buf + curPos)) = size;
				curPos += sizeof(size_t);
				iterator it = cont.begin();
				for (; it != cont.end(); ++it)
				{
					bcopy(&(it->first), &buf[curPos], sizeof(T1));
					curPos += sizeof(T1);
					
					size_2 = it->second.size();
					*((size_t*)(buf + curPos)) = size_2;
				curPos += sizeof(size_t);
				iterator_2 it_2 = it->second.begin();
				for (; it_2 != it->second.end(); ++it_2)
				{
					bcopy(&(*it_2), &buf[curPos], sizeof(value_type_2));
					curPos += sizeof(value_type_2);
				}
			}			
		}
		else
		{
			WORD saveID = *((WORD*)(buf + curPos));
			if (saveID != id) return curPos;
			curPos += sizeof(WORD);

			size = *((size_t*)(buf + curPos));
			curPos += sizeof(size_t);
			for (size_t i = 0; i < size; ++i)
			{
				T1 key;
				bcopy(&buf[curPos], &key, sizeof(T1));
				curPos += sizeof(T1);

				size_2 = *((size_t*)(buf + curPos));
				curPos += sizeof(size_t);
				for (size_t j = 0; j < size_2; ++j)
				{
					if (isNeed)
						cont[key].insert(cont[key].end(), *(value_type_2*)&buf[curPos]);
					curPos += sizeof(value_type_2);
				}
			}
		}
		return curPos;
	}

	/*
	 *\brief 序列化容器 一级vector，二级容器为map（特化）
	 */
	template <typename T1, typename T2, typename Compare>
	DWORD serialize(WORD id, std::vector<std::map<T1, T2, Compare> >& cont, bool isNeed=true)
	{
		typedef typename std::vector<std::map<T1, T2, Compare> >::value_type value_type;
		typedef typename std::vector<std::map<T1, T2, Compare> >::iterator   iterator;
		typedef typename std::map<T1, T2, Compare>::value_type value_type_2;
		typedef typename std::map<T1, T2, Compare>::iterator iterator_2;

		size_t   size = 0;
		size_t   size_2 = 0;
		if(operation == SERIALIZE_SAVE)
		{
			if (!isNeed) return curPos;
			*((WORD*)(buf + curPos)) = id;
			curPos += sizeof(WORD);

			size = cont.size();
			*((size_t*)(buf + curPos)) = size;
			curPos += sizeof(size_t);
			iterator it = cont.begin();
			for (; it != cont.end(); ++it)
			{

				size_2 = it->size();
				*((size_t*)(buf + curPos)) = size_2;
				curPos += sizeof(size_t);
				iterator_2 it_2 = it->begin();
				for (; it_2 != it->end(); ++it_2)
				{
					bcopy(&(it_2->first), &buf[curPos], sizeof(T1));
					curPos += sizeof(T1);

					bcopy(&(it_2->second), &buf[curPos], sizeof(T2));
					curPos += sizeof(T2);
				}
			}			
		}
		else
		{
			WORD saveID = *((WORD*)(buf + curPos));
			if (saveID != id) return curPos;
			curPos += sizeof(WORD);

			size = *((size_t*)(buf + curPos));
			curPos += sizeof(size_t);
			for (size_t i = 0; i < size; ++i)
			{
				size_2 = *((size_t*)(buf + curPos));
				curPos += sizeof(size_t);
				value_type valueType;
				for (size_t j = 0; j < size_2; ++j)
				{
					valueType.insert(std::make_pair(*(T1*)&buf[curPos], *(T2*)&buf[curPos + sizeof(T1)]));
					curPos += sizeof(T1) + sizeof(T2);
				}
				if (isNeed)
					cont.insert(cont.end(), valueType);
			}
		}
		return curPos;
	}

	/*
	 *\brief 序列化容器 一级vector，二级容器为vector（特化）
	 */
	template <typename T1>
	DWORD serialize(WORD id, std::vector<std::vector<T1> >& cont, bool isNeed=true)
	{
		typedef typename std::vector<std::vector<T1> >::value_type value_type;
		typedef typename std::vector<std::vector<T1> >::iterator   iterator;
		typedef typename std::vector<T1>::value_type value_type_2;
		typedef typename std::vector<T1>::iterator iterator_2;

		size_t   size = 0;
		size_t   size_2 = 0;
		if(operation == SERIALIZE_SAVE)
		{
			if (!isNeed) return curPos;
			*((WORD*)(buf + curPos)) = id;
			curPos += sizeof(WORD);

			size = cont.size();
			*((size_t*)(buf + curPos)) = size;
			curPos += sizeof(size_t);
			iterator it = cont.begin();
			for (; it != cont.end(); ++it)
			{

				size_2 = it->size();
				*((size_t*)(buf + curPos)) = size_2;
				curPos += sizeof(size_t);
				iterator_2 it_2 = it->begin();
				for (; it_2 != it->end(); ++it_2)
				{
					bcopy(&(*it_2), &buf[curPos], sizeof(T1));
					curPos += sizeof(T1);
				}
			}			
		}
		else
		{
			WORD saveID = *((WORD*)(buf + curPos));
			if (saveID != id) return curPos;
			curPos += sizeof(WORD);

			size = *((size_t*)(buf + curPos));
			curPos += sizeof(size_t);
			for (size_t i = 0; i < size; ++i)
			{
				size_2 = *((size_t*)(buf + curPos));
				curPos += sizeof(size_t);
				value_type valueType;
				for (size_t j = 0; j < size_2; ++j)
				{
					valueType.insert(valueType.end(), *(value_type_2*)&buf[curPos]);
					curPos += sizeof(value_type_2);
				}
				if (isNeed)
					cont.insert(cont.end(), valueType);
			}
		}
		return curPos;
	}

	/*
	 *\brief 序列化容器 一级vector，二级容器为list（特化）
	 */
	template <typename T1>
	DWORD serialize(WORD id, std::vector<std::list<T1> >& cont, bool isNeed=true)
	{
		typedef typename std::vector<std::list<T1> >::value_type value_type;
		typedef typename std::vector<std::list<T1> >::iterator   iterator;
		typedef typename std::list<T1>::value_type value_type_2;
		typedef typename std::list<T1>::iterator iterator_2;

		size_t   size = 0;
		size_t   size_2 = 0;
		if(operation == SERIALIZE_SAVE)
		{
			if (!isNeed) return curPos;
			*((WORD*)(buf + curPos)) = id;
			curPos += sizeof(WORD);

			size = cont.size();
			*((size_t*)(buf + curPos)) = size;
			curPos += sizeof(size_t);
			iterator it = cont.begin();
			for (; it != cont.end(); ++it)
			{

				size_2 = it->size();
				*((size_t*)(buf + curPos)) = size_2;
				curPos += sizeof(size_t);
				iterator_2 it_2 = it->begin();
				for (; it_2 != it->end(); ++it_2)
				{
					bcopy(&(*it_2), &buf[curPos], sizeof(T1));
					curPos += sizeof(T1);
				}
			}			
		}
		else
		{
			WORD saveID = *((WORD*)(buf + curPos));
			if (saveID != id) return curPos;
			curPos += sizeof(WORD);

			size = *((size_t*)(buf + curPos));
			curPos += sizeof(size_t);
			for (size_t i = 0; i < size; ++i)
			{
				size_2 = *((size_t*)(buf + curPos));
				curPos += sizeof(size_t);
				value_type valueType;
				for (size_t j = 0; j < size_2; ++j)
				{
					valueType.insert(valueType.end(), *(value_type_2*)&buf[curPos]);
					curPos += sizeof(value_type_2);
				}
				if (isNeed)
					cont.insert(cont.end(), valueType);
			}
		}
		return curPos;
	}

	/*
	 *\brief 序列化容器 一级vector，二级容器为deque（特化）
	 */
	template <typename T1>
	DWORD serialize(WORD id, std::vector<std::deque<T1> >& cont, bool isNeed=true)
	{
		typedef typename std::vector<std::deque<T1> >::value_type value_type;
		typedef typename std::vector<std::deque<T1> >::iterator   iterator;
		typedef typename std::deque<T1>::value_type value_type_2;
		typedef typename std::deque<T1>::iterator iterator_2;

		size_t   size = 0;
		size_t   size_2 = 0;
		if(operation == SERIALIZE_SAVE)
		{
			if (!isNeed) return curPos;
			*((WORD*)(buf + curPos)) = id;
			curPos += sizeof(WORD);

			size = cont.size();
			*((size_t*)(buf + curPos)) = size;
			curPos += sizeof(size_t);
			iterator it = cont.begin();
			for (; it != cont.end(); ++it)
			{

				size_2 = it->size();
				*((size_t*)(buf + curPos)) = size_2;
				curPos += sizeof(size_t);
				iterator_2 it_2 = it->begin();
				for (; it_2 != it->end(); ++it_2)
				{
					bcopy(&(*it_2), &buf[curPos], sizeof(T1));
					curPos += sizeof(T1);
				}
			}			
		}
		else
		{
			WORD saveID = *((WORD*)(buf + curPos));
			if (saveID != id) return curPos;
			curPos += sizeof(WORD);

			size = *((size_t*)(buf + curPos));
			curPos += sizeof(size_t);
			for (size_t i = 0; i < size; ++i)
			{
				size_2 = *((size_t*)(buf + curPos));
				curPos += sizeof(size_t);
				value_type valueType;
				for (size_t j = 0; j < size_2; ++j)
				{
					valueType.insert(valueType.end(), *(value_type_2*)&buf[curPos]);
					curPos += sizeof(value_type_2);
				}
				if (isNeed)
					cont.insert(cont.end(), valueType);
			}
		}
		return curPos;
	}
	
	/*
	 *\brief 序列化容器 一级vector，二级容器为set（特化）
	 */
	template <typename T1>
	DWORD serialize(WORD id, std::vector<std::set<T1> >& cont, bool isNeed=true)
	{
		typedef typename std::vector<std::set<T1> >::value_type value_type;
		typedef typename std::vector<std::set<T1> >::iterator   iterator;
		typedef typename std::set<T1>::value_type value_type_2;
		typedef typename std::set<T1>::iterator iterator_2;

		size_t   size = 0;
		size_t   size_2 = 0;
		if(operation == SERIALIZE_SAVE)
		{
			if (!isNeed) return curPos;
			*((WORD*)(buf + curPos)) = id;
			curPos += sizeof(WORD);

			size = cont.size();
			*((size_t*)(buf + curPos)) = size;
			curPos += sizeof(size_t);
			iterator it = cont.begin();
			for (; it != cont.end(); ++it)
			{

				size_2 = it->size();
				*((size_t*)(buf + curPos)) = size_2;
				curPos += sizeof(size_t);
				iterator_2 it_2 = it->begin();
				for (; it_2 != it->end(); ++it_2)
				{
					bcopy(&(*it_2), &buf[curPos], sizeof(T1));
					curPos += sizeof(T1);
				}
			}			
		}
		else
		{
			WORD saveID = *((WORD*)(buf + curPos));
			if (saveID != id) return curPos;
			curPos += sizeof(WORD);

			size = *((size_t*)(buf + curPos));
			curPos += sizeof(size_t);
			for (size_t i = 0; i < size; ++i)
			{
				size_2 = *((size_t*)(buf + curPos));
				curPos += sizeof(size_t);
				value_type valueType;
				for (size_t j = 0; j < size_2; ++j)
				{
					valueType.insert(valueType.end(), *(value_type_2*)&buf[curPos]);
					curPos += sizeof(value_type_2);
				}
				if (isNeed)
					cont.insert(cont.end(), valueType);
			}
		}
		return curPos;
	}

	/*
	 *\brief 序列化容器 一级list，二级容器为map（特化）
	 */
	template <typename T1, typename T2, typename Compare>
	DWORD serialize(WORD id, std::list<std::map<T1, T2, Compare> >& cont, bool isNeed=true)
	{
		typedef typename std::list<std::map<T1, T2, Compare> >::value_type value_type;
		typedef typename std::list<std::map<T1, T2, Compare> >::iterator   iterator;
		typedef typename std::map<T1, T2, Compare>::value_type value_type_2;
		typedef typename std::map<T1, T2, Compare>::iterator iterator_2;

		size_t   size = 0;
		size_t   size_2 = 0;
		if(operation == SERIALIZE_SAVE)
		{
			if (!isNeed) return curPos;
			*((WORD*)(buf + curPos)) = id;
			curPos += sizeof(WORD);

			size = cont.size();
			*((size_t*)(buf + curPos)) = size;
			curPos += sizeof(size_t);
			iterator it = cont.begin();
			for (; it != cont.end(); ++it)
			{

				size_2 = it->size();
				*((size_t*)(buf + curPos)) = size_2;
				curPos += sizeof(size_t);
				iterator_2 it_2 = it->begin();
				for (; it_2 != it->end(); ++it_2)
				{
					bcopy(&(it_2->first), &buf[curPos], sizeof(T1));
					curPos += sizeof(T1);

					bcopy(&(it_2->second), &buf[curPos], sizeof(T2));
					curPos += sizeof(T2);
				}
			}			
		}
		else
		{
			WORD saveID = *((WORD*)(buf + curPos));
			if (saveID != id) return curPos;
			curPos += sizeof(WORD);

			size = *((size_t*)(buf + curPos));
			curPos += sizeof(size_t);
			for (size_t i = 0; i < size; ++i)
			{
				size_2 = *((size_t*)(buf + curPos));
				curPos += sizeof(size_t);
				value_type valueType;
				for (size_t j = 0; j < size_2; ++j)
				{
					valueType.insert(std::make_pair(*(T1*)&buf[curPos], *(T2*)&buf[curPos + sizeof(T1)]));
					curPos += sizeof(T1) + sizeof(T2);
				}
				if (isNeed)
					cont.insert(cont.end(), valueType);
			}
		}
		return curPos;
	}

	/*
	 *\brief 序列化容器 一级list，二级容器为vector（特化）
	 */
	template <typename T1>
	DWORD serialize(WORD id, std::list<std::vector<T1> >& cont, bool isNeed=true)
	{
		typedef typename std::list<std::vector<T1> >::value_type value_type;
		typedef typename std::list<std::vector<T1> >::iterator   iterator;
		typedef typename std::vector<T1>::value_type value_type_2;
		typedef typename std::vector<T1>::iterator iterator_2;

		size_t   size = 0;
		size_t   size_2 = 0;
		if(operation == SERIALIZE_SAVE)
		{
			if (!isNeed) return curPos;
			*((WORD*)(buf + curPos)) = id;
			curPos += sizeof(WORD);

			size = cont.size();
			*((size_t*)(buf + curPos)) = size;
			curPos += sizeof(size_t);
			iterator it = cont.begin();
			for (; it != cont.end(); ++it)
			{

				size_2 = it->size();
				*((size_t*)(buf + curPos)) = size_2;
				curPos += sizeof(size_t);
				iterator_2 it_2 = it->begin();
				for (; it_2 != it->end(); ++it_2)
				{
					bcopy(&(*it_2), &buf[curPos], sizeof(T1));
					curPos += sizeof(T1);
				}
			}			
		}
		else
		{
			WORD saveID = *((WORD*)(buf + curPos));
			if (saveID != id) return curPos;
			curPos += sizeof(WORD);

			size = *((size_t*)(buf + curPos));
			curPos += sizeof(size_t);
			for (size_t i = 0; i < size; ++i)
			{
				size_2 = *((size_t*)(buf + curPos));
				curPos += sizeof(size_t);
				value_type valueType;
				for (size_t j = 0; j < size_2; ++j)
				{
					valueType.insert(valueType.end(), *(value_type_2*)&buf[curPos]);
					curPos += sizeof(value_type_2);
				}
				if (isNeed)
					cont.insert(cont.end(), valueType);
			}
		}
		return curPos;
	}

	/*
	 *\brief 序列化容器 一级list，二级容器为list（特化）
	 */
	template <typename T1>
	DWORD serialize(WORD id, std::list<std::list<T1> >& cont, bool isNeed=true)
	{
		typedef typename std::list<std::list<T1> >::value_type value_type;
		typedef typename std::list<std::list<T1> >::iterator   iterator;
		typedef typename std::list<T1>::value_type value_type_2;
		typedef typename std::list<T1>::iterator iterator_2;

		size_t   size = 0;
		size_t   size_2 = 0;
		if(operation == SERIALIZE_SAVE)
		{
			if (!isNeed) return curPos;
			*((WORD*)(buf + curPos)) = id;
			curPos += sizeof(WORD);

			size = cont.size();
			*((size_t*)(buf + curPos)) = size;
			curPos += sizeof(size_t);
			iterator it = cont.begin();
			for (; it != cont.end(); ++it)
			{

				size_2 = it->size();
				*((size_t*)(buf + curPos)) = size_2;
				curPos += sizeof(size_t);
				iterator_2 it_2 = it->begin();
				for (; it_2 != it->end(); ++it_2)
				{
					bcopy(&(*it_2), &buf[curPos], sizeof(T1));
					curPos += sizeof(T1);
				}
			}			
		}
		else
		{
			WORD saveID = *((WORD*)(buf + curPos));
			if (saveID != id) return curPos;
			curPos += sizeof(WORD);

			size = *((size_t*)(buf + curPos));
			curPos += sizeof(size_t);
			for (size_t i = 0; i < size; ++i)
			{
				size_2 = *((size_t*)(buf + curPos));
				curPos += sizeof(size_t);
				value_type valueType;
				for (size_t j = 0; j < size_2; ++j)
				{
					valueType.insert(valueType.end(), *(value_type_2*)&buf[curPos]);
					curPos += sizeof(value_type_2);
				}
				if (isNeed)
					cont.insert(cont.end(), valueType);
			}
		}
		return curPos;
	}

	/*
	 *\brief 序列化容器 一级list，二级容器为deque（特化）
	 */
	template <typename T1>
	DWORD serialize(WORD id, std::list<std::deque<T1> >& cont, bool isNeed=true)
	{
		typedef typename std::list<std::deque<T1> >::value_type value_type;
		typedef typename std::list<std::deque<T1> >::iterator   iterator;
		typedef typename std::deque<T1>::value_type value_type_2;
		typedef typename std::deque<T1>::iterator iterator_2;

		size_t   size = 0;
		size_t   size_2 = 0;
		if(operation == SERIALIZE_SAVE)
		{
			if (!isNeed) return curPos;
			*((WORD*)(buf + curPos)) = id;
			curPos += sizeof(WORD);

			size = cont.size();
			*((size_t*)(buf + curPos)) = size;
			curPos += sizeof(size_t);
			iterator it = cont.begin();
			for (; it != cont.end(); ++it)
			{

				size_2 = it->size();
				*((size_t*)(buf + curPos)) = size_2;
				curPos += sizeof(size_t);
				iterator_2 it_2 = it->begin();
				for (; it_2 != it->end(); ++it_2)
				{
					bcopy(&(*it_2), &buf[curPos], sizeof(T1));
					curPos += sizeof(T1);
				}
			}			
		}
		else
		{
			WORD saveID = *((WORD*)(buf + curPos));
			if (saveID != id) return curPos;
			curPos += sizeof(WORD);

			size = *((size_t*)(buf + curPos));
			curPos += sizeof(size_t);
			for (size_t i = 0; i < size; ++i)
			{
				size_2 = *((size_t*)(buf + curPos));
				curPos += sizeof(size_t);
				value_type valueType;
				for (size_t j = 0; j < size_2; ++j)
				{
					valueType.insert(valueType.end(), *(value_type_2*)&buf[curPos]);
					curPos += sizeof(value_type_2);
				}
				if (isNeed)
					cont.insert(cont.end(), valueType);
			}
		}
		return curPos;
	}
	
	/*
	 *\brief 序列化容器 一级list，二级容器为set（特化）
	 */
	template <typename T1>
	DWORD serialize(WORD id, std::list<std::set<T1> >& cont, bool isNeed=true)
	{
		typedef typename std::list<std::set<T1> >::value_type value_type;
		typedef typename std::list<std::set<T1> >::iterator   iterator;
		typedef typename std::set<T1>::value_type value_type_2;
		typedef typename std::set<T1>::iterator iterator_2;

		size_t   size = 0;
		size_t   size_2 = 0;
		if(operation == SERIALIZE_SAVE)
		{
			if (!isNeed) return curPos;
			*((WORD*)(buf + curPos)) = id;
			curPos += sizeof(WORD);

			size = cont.size();
			*((size_t*)(buf + curPos)) = size;
			curPos += sizeof(size_t);
			iterator it = cont.begin();
			for (; it != cont.end(); ++it)
			{

				size_2 = it->size();
				*((size_t*)(buf + curPos)) = size_2;
				curPos += sizeof(size_t);
				iterator_2 it_2 = it->begin();
				for (; it_2 != it->end(); ++it_2)
				{
					bcopy(&(*it_2), &buf[curPos], sizeof(T1));
					curPos += sizeof(T1);
				}
			}			
		}
		else
		{
			WORD saveID = *((WORD*)(buf + curPos));
			if (saveID != id) return curPos;
			curPos += sizeof(WORD);

			size = *((size_t*)(buf + curPos));
			curPos += sizeof(size_t);
			for (size_t i = 0; i < size; ++i)
			{
				size_2 = *((size_t*)(buf + curPos));
				curPos += sizeof(size_t);
				value_type valueType;
				for (size_t j = 0; j < size_2; ++j)
				{
					valueType.insert(valueType.end(), *(value_type_2*)&buf[curPos]);
					curPos += sizeof(value_type_2);
				}
				if (isNeed)
					cont.insert(cont.end(), valueType);
			}
		}
		return curPos;
	}
	
	/*
	 *\brief 序列化容器 一级deque，二级容器为map（特化）
	 */
	template <typename T1, typename T2, typename Compare>
	DWORD serialize(WORD id, std::deque<std::map<T1, T2, Compare> >& cont, bool isNeed=true)
	{
		typedef typename std::deque<std::map<T1, T2, Compare> >::value_type value_type;
		typedef typename std::deque<std::map<T1, T2, Compare> >::iterator   iterator;
		typedef typename std::map<T1, T2, Compare>::value_type value_type_2;
		typedef typename std::map<T1, T2, Compare>::iterator iterator_2;

		size_t   size = 0;
		size_t   size_2 = 0;
		if(operation == SERIALIZE_SAVE)
		{
			if (!isNeed) return curPos;
			*((WORD*)(buf + curPos)) = id;
			curPos += sizeof(WORD);

			size = cont.size();
			*((size_t*)(buf + curPos)) = size;
			curPos += sizeof(size_t);
			iterator it = cont.begin();
			for (; it != cont.end(); ++it)
			{

				size_2 = it->size();
				*((size_t*)(buf + curPos)) = size_2;
				curPos += sizeof(size_t);
				iterator_2 it_2 = it->begin();
				for (; it_2 != it->end(); ++it_2)
				{
					bcopy(&(it_2->first), &buf[curPos], sizeof(T1));
					curPos += sizeof(T1);

					bcopy(&(it_2->second), &buf[curPos], sizeof(T2));
					curPos += sizeof(T2);
				}
			}			
		}
		else
		{
			WORD saveID = *((WORD*)(buf + curPos));
			if (saveID != id) return curPos;
			curPos += sizeof(WORD);

			size = *((size_t*)(buf + curPos));
			curPos += sizeof(size_t);
			for (size_t i = 0; i < size; ++i)
			{
				size_2 = *((size_t*)(buf + curPos));
				curPos += sizeof(size_t);
				value_type valueType;
				for (size_t j = 0; j < size_2; ++j)
				{
					valueType.insert(std::make_pair(*(T1*)&buf[curPos], *(T2*)&buf[curPos + sizeof(T1)]));
					curPos += sizeof(T1) + sizeof(T2);
				}
				if (isNeed)
					cont.insert(cont.end(), valueType);
			}
		}
		return curPos;
	}

	/*
	 *\brief 序列化容器 一级deque，二级容器为vector（特化）
	 */
	template <typename T1>
	DWORD serialize(WORD id, std::deque<std::vector<T1> >& cont, bool isNeed=true)
	{
		typedef typename std::deque<std::vector<T1> >::value_type value_type;
		typedef typename std::deque<std::vector<T1> >::iterator   iterator;
		typedef typename std::vector<T1>::value_type value_type_2;
		typedef typename std::vector<T1>::iterator iterator_2;

		size_t   size = 0;
		size_t   size_2 = 0;
		if(operation == SERIALIZE_SAVE)
		{
			if (!isNeed) return curPos;
			*((WORD*)(buf + curPos)) = id;
			curPos += sizeof(WORD);

			size = cont.size();
			*((size_t*)(buf + curPos)) = size;
			curPos += sizeof(size_t);
			iterator it = cont.begin();
			for (; it != cont.end(); ++it)
			{

				size_2 = it->size();
				*((size_t*)(buf + curPos)) = size_2;
				curPos += sizeof(size_t);
				iterator_2 it_2 = it->begin();
				for (; it_2 != it->end(); ++it_2)
				{
					bcopy(&(*it_2), &buf[curPos], sizeof(T1));
					curPos += sizeof(T1);
				}
			}			
		}
		else
		{
			WORD saveID = *((WORD*)(buf + curPos));
			if (saveID != id) return curPos;
			curPos += sizeof(WORD);

			size = *((size_t*)(buf + curPos));
			curPos += sizeof(size_t);
			for (size_t i = 0; i < size; ++i)
			{
				size_2 = *((size_t*)(buf + curPos));
				curPos += sizeof(size_t);
				value_type valueType;
				for (size_t j = 0; j < size_2; ++j)
				{
					valueType.insert(valueType.end(), *(value_type_2*)&buf[curPos]);
					curPos += sizeof(value_type_2);
				}
				if (isNeed)
					cont.insert(cont.end(), valueType);
			}
		}
		return curPos;
	}

	/*
	 *\brief 序列化容器 一级deque，二级容器为list（特化）
	 */
	template <typename T1>
	DWORD serialize(WORD id, std::deque<std::list<T1> >& cont, bool isNeed=true)
	{
		typedef typename std::deque<std::list<T1> >::value_type value_type;
		typedef typename std::deque<std::list<T1> >::iterator   iterator;
		typedef typename std::list<T1>::value_type value_type_2;
		typedef typename std::list<T1>::iterator iterator_2;

		size_t   size = 0;
		size_t   size_2 = 0;
		if(operation == SERIALIZE_SAVE)
		{
			if (!isNeed) return curPos;
			*((WORD*)(buf + curPos)) = id;
			curPos += sizeof(WORD);

			size = cont.size();
			*((size_t*)(buf + curPos)) = size;
			curPos += sizeof(size_t);
			iterator it = cont.begin();
			for (; it != cont.end(); ++it)
			{

				size_2 = it->size();
				*((size_t*)(buf + curPos)) = size_2;
				curPos += sizeof(size_t);
				iterator_2 it_2 = it->begin();
				for (; it_2 != it->end(); ++it_2)
				{
					bcopy(&(*it_2), &buf[curPos], sizeof(T1));
					curPos += sizeof(T1);
				}
			}			
		}
		else
		{
			WORD saveID = *((WORD*)(buf + curPos));
			if (saveID != id) return curPos;
			curPos += sizeof(WORD);

			size = *((size_t*)(buf + curPos));
			curPos += sizeof(size_t);
			for (size_t i = 0; i < size; ++i)
			{
				size_2 = *((size_t*)(buf + curPos));
				curPos += sizeof(size_t);
				value_type valueType;
				for (size_t j = 0; j < size_2; ++j)
				{
					valueType.insert(valueType.end(), *(value_type_2*)&buf[curPos]);
					curPos += sizeof(value_type_2);
				}
				if (isNeed)
					cont.insert(cont.end(), valueType);
			}
		}
		return curPos;
	}

	/*
	 *\brief 序列化容器 一级deque，二级容器为deque（特化）
	 */
	template <typename T1>
	DWORD serialize(WORD id, std::deque<std::deque<T1> >& cont, bool isNeed=true)
	{
		typedef typename std::deque<std::deque<T1> >::value_type value_type;
		typedef typename std::deque<std::deque<T1> >::iterator   iterator;
		typedef typename std::deque<T1>::value_type value_type_2;
		typedef typename std::deque<T1>::iterator iterator_2;

		size_t   size = 0;
		size_t   size_2 = 0;
		if(operation == SERIALIZE_SAVE)
		{
			if (!isNeed) return curPos;
			*((WORD*)(buf + curPos)) = id;
			curPos += sizeof(WORD);

			size = cont.size();
			*((size_t*)(buf + curPos)) = size;
			curPos += sizeof(size_t);
			iterator it = cont.begin();
			for (; it != cont.end(); ++it)
			{

				size_2 = it->size();
				*((size_t*)(buf + curPos)) = size_2;
				curPos += sizeof(size_t);
				iterator_2 it_2 = it->begin();
				for (; it_2 != it->end(); ++it_2)
				{
					bcopy(&(*it_2), &buf[curPos], sizeof(T1));
					curPos += sizeof(T1);
				}
			}			
		}
		else
		{
			WORD saveID = *((WORD*)(buf + curPos));
			if (saveID != id) return curPos;
			curPos += sizeof(WORD);

			size = *((size_t*)(buf + curPos));
			curPos += sizeof(size_t);
			for (size_t i = 0; i < size; ++i)
			{
				size_2 = *((size_t*)(buf + curPos));
				curPos += sizeof(size_t);
				value_type valueType;
				for (size_t j = 0; j < size_2; ++j)
				{
					valueType.insert(valueType.end(), *(value_type_2*)&buf[curPos]);
					curPos += sizeof(value_type_2);
				}
				if (isNeed)
					cont.insert(cont.end(), valueType);
			}
		}
		return curPos;
	}
	
	/*
	 *\brief 序列化容器 一级deque，二级容器为set（特化）
	 */
	template <typename T1>
	DWORD serialize(WORD id, std::deque<std::set<T1> >& cont, bool isNeed=true)
	{
		typedef typename std::deque<std::set<T1> >::value_type value_type;
		typedef typename std::deque<std::set<T1> >::iterator   iterator;
		typedef typename std::set<T1>::value_type value_type_2;
		typedef typename std::set<T1>::iterator iterator_2;

		size_t   size = 0;
		size_t   size_2 = 0;
		if(operation == SERIALIZE_SAVE)
		{
			if (!isNeed) return curPos;
			*((WORD*)(buf + curPos)) = id;
			curPos += sizeof(WORD);

			size = cont.size();
			*((size_t*)(buf + curPos)) = size;
			curPos += sizeof(size_t);
			iterator it = cont.begin();
			for (; it != cont.end(); ++it)
			{

				size_2 = it->size();
				*((size_t*)(buf + curPos)) = size_2;
				curPos += sizeof(size_t);
				iterator_2 it_2 = it->begin();
				for (; it_2 != it->end(); ++it_2)
				{
					bcopy(&(*it_2), &buf[curPos], sizeof(T1));
					curPos += sizeof(T1);
				}
			}			
		}
		else
		{
			WORD saveID = *((WORD*)(buf + curPos));
			if (saveID != id) return curPos;
			curPos += sizeof(WORD);

			size = *((size_t*)(buf + curPos));
			curPos += sizeof(size_t);
			for (size_t i = 0; i < size; ++i)
			{
				size_2 = *((size_t*)(buf + curPos));
				curPos += sizeof(size_t);
				value_type valueType;
				for (size_t j = 0; j < size_2; ++j)
				{
					valueType.insert(valueType.end(), *(value_type_2*)&buf[curPos]);
					curPos += sizeof(value_type_2);
				}
				if (isNeed)
					cont.insert(cont.end(), valueType);
			}
		}
		return curPos;
	}

	/*
	 *\brief 初始化
	 */
	void _init()
	{
		version = _SerializableT::_SerializeVersion;
		
		switch(headType)
		{
			case ARCHIVE_HEAD:
				{
					if(operation == SERIALIZE_SAVE)
						*((DWORD*)(buf+curPos)) = version;
					else
						version = *((DWORD*)(buf+curPos));
					curPos += sizeof(DWORD);
					count = (DWORD*)(buf+curPos);
					if(operation == SERIALIZE_SAVE) *count = 0;
					curPos += sizeof(DWORD);
				}
				break;
			case ARCHIVE_VERSION:
				{
					if(operation == SERIALIZE_SAVE)
						*((DWORD*)(buf+curPos)) = version;
					else
						version = *((DWORD*)(buf+curPos));
					curPos += sizeof(DWORD);
					count = &_count;
				}
				break;
			case ARCHIVE_COUNT:
				{
					count = (DWORD*)(buf+curPos);
					if(operation == SERIALIZE_SAVE) *count = 0;
					curPos += sizeof(DWORD);
				}
				break;
			case ARCHIVE_NOHEAD:
			default:
				{
					count = &_count;
				}
				break;
		}
	}

	/*
	 *\brief 构造函数 用于生成子档案
	 */
	zArchive(void const* _buf,size_t& _curPos,zArchiveHead _headType,zSerializeOP _op,DWORD)
		:_count(0),headType(_headType),buf((char*)_buf),curPos(_curPos),operation(_op)
	{
		_init();
	}
public:
	/*
	 *\brief 构造函数
	 *\param _buf 缓存  必须要保证足够容纳所有的对象。
	 *\param _curPos 当前缓存位置
	 *\param _headType 档案头类型
	 *\param _op 可以进行的操作
	 */
	zArchive(void const* _buf,size_t _curPos,zArchiveHead _headType = ARCHIVE_NOHEAD,zSerializeOP _op = SERIALIZE_SAVE)
		:_count(0),headType(_headType),buf((char*)_buf),__curPos(_curPos),curPos(__curPos),operation(_op)
	{
		_init();
	}
	
	/*
	 *\brief 设置为旧版本(非序列化存储) 版本号为0
	 */
	void setOldVersion()
	{
		version = 0;
	}

	/*
	 *\brief 获取档案头类型
	 */
	zArchiveHead getHeadType() const
	{
		return headType;
	}

	/*
	 *\brief 清空 用于保存时缓存不够的情况(调用此函数以前预留出的空间失效，数量也不会加到缓存中)
	 */
	void flush(size_t _curPos = 0)
	{
		if(operation == SERIALIZE_LOAD) return;
		curPos = _curPos;
		_count = *count;
		count = &_count;
	}

	/*
	 *\brief 预留出指定数目基本类型的内存空间，返回指针
	 */
	template <typename T>
	T* getSaveSpace(DWORD _num = 1)
	{
		if(!_num) return NULL;
		T* ret = (T*)(buf + curPos);
		curPos += sizeof(T) * _num;
		return ret;
	}

	/*
	 *\brief 设置额外参数，传递给被序列化对象
	 */
	void setParamP(int _key,void* _param)
	{
		params_p[_key] = _param;
	}
	void* getParamP(int _key)
	{
		return params_p[_key];
	}

	void setParamN(int _key,QWORD _param)
	{
		params_n[_key] = _param;
	}
	QWORD getParamN(int _key)
	{
		return params_n[_key];
	}

	/*
	 *\brief 获取序列化对象数目
	 */
	DWORD getCount() const
	{
		return *count;
	}

	/*
	 *\brief 获取版本
	 */
	DWORD getVersion() const
	{
		return version;
	}

	/*
	 *\brief 获取当前缓存位置
	 */
	size_t getPos() const
	{
		return curPos;
	}

	/*
	 *\brief 跳过一定空间 不加载
	 *\param size 每个对象占用空间大小
	 *\param num 跳过对象数目，为0跳过所有
	 */
	void ignore(size_t size,DWORD num = 0)
	{
		if(num) curPos += size * num;
		else curPos += size * getCount();
	}

	/*
	 *\brief 将对象存入档案
	 *\return true 成功 false 失败
	 */
	bool operator <<(const SerializeT& obj)
	{
		if(operation == SERIALIZE_LOAD) return false;
		size_t _pos = curPos;
		if(const_cast<SerializeT&>(obj)._Serialize(*this))
		{
			++(*count);
			return true;
		}
		curPos = _pos; 	//保存失败回退
		return false;
	}

	/*
	 *\brief 将对象存入档案，参数为const对象指针
	 */
	bool operator <<(const SerializeT * objPtr)
	{
		if(!objPtr) return false;
		return operator<<(*objPtr);
	}

	/*
	 *brief 将对象存入档案，参数为对象指针
	 */
	bool operator <<(SerializeT* objPtr)
	{
		if(!objPtr) return false;
		return operator<<(*objPtr);
	}

	/*
	 *\brief 将对象从档案中取出
	 *\return true 成功 false 失败
	 */
	bool operator >>(SerializeT& obj)
	{
		if(operation == SERIALIZE_SAVE) return false;
		return obj._Serialize(*this);
	}

	/*
	 *\brief 将对象从档案中取出，参数为对象指针。当对象指针为NULL时，此操作会分配一个对象。
	 */
	bool operator >>(SerializeT*& objPtr)
	{
		if(objPtr) return operator>>(*objPtr);
		objPtr = FIR_NEW SerializeT;
		if(objPtr)
		{
			if(operator>>(*objPtr))
				return true;
			else
			{
				SAFE_DELETE(objPtr);
				return false;
			}
		}
		return false;
	}

	bool operator &(SerializeT& obj)
	{
		if(operation == SERIALIZE_SAVE)
			return operator<<(obj);
		else
			return operator>>(obj);
	}

	bool operator &(SerializeT*& objPtr)
	{
		if(operation == SERIALIZE_SAVE)
			return operator<<(objPtr);
		else
			return operator>>(objPtr);
	}

};

/*
 * \brief 可序列化接口，要序列化的对象必须实现此接口
 * \param SerializeT 类名
 * \param _version 类的版本(从1开始)
 */
#define FIR_SERIALIZE_DEC(SerializeT,_version) \
	friend class zArchive<SerializeT>; \
	static const DWORD _SerializeVersion = (_version); \
	bool _Serialize(zArchive<SerializeT>& archive)\

#define FIR_SERIALIZE_IMP(SerializeT) \
	bool SerializeT::_Serialize(zArchive<SerializeT>& archive)\
{switch(0){case 0:


/*
 *\brief 这个宏用于生成子档案,在序列化函数中调用
 *\param subname 子档案名
 *\param headType 档案头类型
 */
#define FIR_SERIALIZE_CREATESUBARC(SerializeT,subname,headType) \
	zArchive<SerializeT> subname(archive.buf,archive.curPos,headType,archive.operation,0);\
	if(archive.version == 0) subname.version = 0; \
	subname.params_p = archive.params_p; \
	subname.params_n = archive.params_n; \

/*
 *\brief 这个宏用于序列化父类中数据,在子类的序列化函数中调用
 *\param baseClassT 父类名
 */
#define FIR_SERIALIZE_BASE(baseClassT) \
	do{\
		FIR_SERIALIZE_CREATESUBARC(baseClassT,barchive,ARCHIVE_VERSION) \
		if(!baseClassT::_Serialize(barchive)) return false; \
	}while(false)


/*
 *\brief 空序列化对象和空文档类 特殊用途
 */
struct zNullSerializeType
{
	FIR_SERIALIZE_DEC(zNullSerializeType,0)
	{
		return true;
	}
};
typedef zArchive<zNullSerializeType> zNullArchive;

/*
 *\brief 封装序列化操作的函数
 *	针对单一变量 结构体 容器进行序列化
 */
class CSerializeManager
{
public:
	/********************************************************************************************/
	/*									序列化方法												*/
	/********************************************************************************************/
	/*
	 *\brief 序列化所有类型 （结构体，数组，容器）基于zArchive
	 */
	template <typename T>
	static DWORD serialize(T& t, BYTE* out)
	{
		zArchive<zNullSerializeType> archive(&out[0],0,ARCHIVE_HEAD,SERIALIZE_SAVE);
		archive & t;
		return archive.getPos();
	}

	/*
	 *\brief 序列化基本类型 （结构体）不基于zArchive 效率更高 存储空间更小
	 */
	template <typename T>
	static DWORD serializeBaseType(T& t, BYTE* out)
	{
		size_t size = 0;
		bcopy(&t, &out[size], sizeof(T));
		size += sizeof(T);

		return size;
	}

	/*
	 *\brief 序列化容器数据(vector, list, deque, set, map) 不基于zArchive 效率更高 存储空间更小
	 */

	template <typename T>
	static DWORD serializeBaseType(std::vector<T>& cont, BYTE* out)
	{   
		typedef typename std::vector<T>::value_type value_type;
		typedef typename std::vector<T>::iterator   iterator;

		size_t   size = 0;
		size_t   num = cont.size();

		*(size_t*)&out[size] = num;
		size += sizeof(size_t);

		iterator it = cont.begin();
		for (; it != cont.end(); ++it)
		{
			bcopy(&(*it), &out[size], sizeof(value_type));
			size += sizeof(value_type);
		}

		return size;
	}

	template <typename T>
	static DWORD serializeBaseType(std::list<T>& cont, BYTE* out)
	{   
		typedef typename std::list<T>::value_type value_type;
		typedef typename std::list<T>::iterator   iterator;

		size_t   size = 0;
		size_t   num = cont.size();

		*(size_t*)&out[size] = num;
		size += sizeof(size_t);

		iterator it = cont.begin();
		for (; it != cont.end(); ++it)
		{
			bcopy(&(*it), &out[size], sizeof(value_type));
			size += sizeof(value_type);
		}

		return size;
	}

	template <typename T>
	static DWORD serializeBaseType(std::set<T>& cont, BYTE* out)
	{   
		typedef typename std::set<T>::value_type value_type;
		typedef typename std::set<T>::iterator   iterator;

		size_t   size = 0;
		size_t   num = cont.size();

		*(size_t*)&out[size] = num;
		size += sizeof(size_t);

		iterator it = cont.begin();
		for (; it != cont.end(); ++it)
		{
			bcopy(&(*it), &out[size], sizeof(value_type));
			size += sizeof(value_type);
		}

		return size;
	}

	template <typename T>
	static DWORD serializeBaseType(std::deque<T>& cont, BYTE* out)
	{   
		typedef typename std::deque<T>::value_type value_type;
		typedef typename std::deque<T>::iterator   iterator;

		size_t   size = 0;
		size_t   num = cont.size();

		*(size_t*)&out[size] = num;
		size += sizeof(size_t);

		iterator it = cont.begin();
		for (; it != cont.end(); ++it)
		{
			bcopy(&(*it), &out[size], sizeof(value_type));
			size += sizeof(value_type);
		}

		return size;
	}

	/*
	 *\brief 序列化容器数据(map) 不基于zArchive 效率更高 存储空间更小
	 */
	template <typename T1, typename T2, typename Compare>
	static DWORD serializeBaseType(std::map<T1, T2, Compare>& cont, BYTE* out)
	{
		typedef typename std::map<T1, T2, Compare>::value_type value_type;
		typedef typename std::map<T1, T2, Compare>::iterator   iterator;

		size_t size = 0;
		size_t num = cont.size();

		*(size_t*)&out[size] = num;
		size += sizeof(size_t);

		iterator it = cont.begin();
		for (; it != cont.end(); ++it)
		{
			bcopy(&(it->first), &out[size], sizeof(T1));
			size += sizeof(T1);

			bcopy(&(it->second), &out[size], sizeof(T2));
			size += sizeof(T2);
		}

		return size;
	}

	/********************************************************************************************/
	/*									反序列化方法											*/
	/********************************************************************************************/
	/*
	 *\brief 反序列化所有类型 （结构体，数组，容器）基于zArchive
	 */
	template <typename T>
	static DWORD unSerialize(T& t, BYTE* data)
	{
		zArchive<zNullSerializeType> archive(&data[0],0,ARCHIVE_HEAD,SERIALIZE_LOAD);
		archive & t;
		return archive.getPos();
	}

	/*
	 *\brief 反序列化基本类型 （结构体）不基于zArchive 效率更高 存储空间更小
	 */
	template <typename T>
	static DWORD unSerializeBaseType(T& t, const BYTE* data)
	{
		bcopy(&data[0], &t, sizeof(T));
		return sizeof(T);
	}

	/*
	 *\brief 反序列化容器数据(vector, list, deque, set) 不基于zArchive 效率更高 存储空间更小
	 */

	template <typename T>
	static DWORD unSerializeBaseType(std::vector<T>& cont, const BYTE* data)
	{       
		typedef typename std::vector<T>::value_type value_type;
		typedef typename std::vector<T>::iterator   iterator;
		typedef typename std::vector<T>::pointer    pointer;

		size_t   size = 0;
		size_t   num = *(size_t*)&data[size];
		size += sizeof(size_t);

		for (size_t i = 0; i < num; ++i)
		{
			cont.insert(cont.end(), *(value_type*)&data[size]);
			size += sizeof(value_type);
		}

		return size;
	} 

	template <typename T>
	static DWORD unSerializeBaseType(std::list<T>& cont, const BYTE* data)
	{       
		typedef typename std::list<T>::value_type value_type;
		typedef typename std::list<T>::iterator   iterator;
		typedef typename std::list<T>::pointer    pointer;

		size_t   size = 0;
		size_t   num = *(size_t*)&data[size];
		size += sizeof(size_t);

		for (size_t i = 0; i < num; ++i)
		{
			cont.insert(cont.end(), *(value_type*)&data[size]);
			size += sizeof(value_type);
		}

		return size;
	} 

	template <typename T>
	static DWORD unSerializeBaseType(std::set<T>& cont, const BYTE* data)
	{       
		typedef typename std::set<T>::value_type value_type;
		typedef typename std::set<T>::iterator   iterator;
		typedef typename std::set<T>::pointer    pointer;

		size_t   size = 0;
		size_t   num = *(size_t*)&data[size];
		size += sizeof(size_t);

		for (size_t i = 0; i < num; ++i)
		{
			cont.insert(cont.end(), *(value_type*)&data[size]);
			size += sizeof(value_type);
		}

		return size;
	} 

	template <typename T>
	static DWORD unSerializeBaseType(std::deque<T>& cont, const BYTE* data)
	{       
		typedef typename std::deque<T>::value_type value_type;
		typedef typename std::deque<T>::iterator   iterator;
		typedef typename std::deque<T>::pointer    pointer;

		size_t   size = 0;
		size_t   num = *(size_t*)&data[size];
		size += sizeof(size_t);

		for (size_t i = 0; i < num; ++i)
		{
			cont.insert(cont.end(), *(value_type*)&data[size]);
			size += sizeof(value_type);
		}

		return size;
	} 

	/*
	 *\brief 反序列化容器数据(map) 不基于zArchive 效率更高 存储空间更小
	 */
	template <typename T1, typename T2, typename Compare>
	static DWORD unSerializeBaseType(std::map<T1, T2, Compare>& cont,const BYTE* data)
	{
		typedef typename std::map<T1, T2, Compare>::value_type value_type;
		typedef typename std::map<T1, T2, Compare>::iterator   iterator;

		size_t  size = 0;
		size_t  num  = *(size_t*)&data[size];
		size += sizeof(size_t);

		for (size_t i = 0; i < num; ++i)
		{
			cont.insert(std::make_pair(*(T1*)&data[size], *(T2*)&data[size + sizeof(T1)]));
			size += sizeof(T1) + sizeof(T2);
		}

		return size;
	}
};
#define MEMBER_SERIALIZE_ADD(id, m)\
	case id:\
	archive.serialize(id, m);
#define MEMBER_SERIALIZE_ADD_ARRAY(id,m,size)\
	case id:\
	archive.serialize(id, m,size,true);
#define MEMBER_SERIALIZE_DEL(id, T ,m)\
	T m;\
	archive.serialize(id, m, false);\

#define MEMBER_SERIALIZE_END\
	archive.serializeEnd();\
}}

#define FIR_SERIALIZE_END_NULL }}

#define SERIALIZE_SAVELOAD_UNSIGNEDCHAR(T)\
	DWORD save(unsigned char *data)\
	{\
		zArchive<T> archive(data,0,ARCHIVE_HEAD,SERIALIZE_SAVE);\
		archive << (*this);\
		return archive.getPos();\
	}\
	DWORD load(unsigned char *data)\
	{\
		zArchive<T> archive(data,0,ARCHIVE_HEAD,SERIALIZE_LOAD);\
		archive >> (*this);\
		return archive.getPos();\
	}\

#define SERIALIZE_SAVELOAD_CHAR(T)\
	DWORD save(char *data)\
	{\
		zArchive<T> archive(data,0,ARCHIVE_HEAD,SERIALIZE_SAVE);\
		archive << (*this);\
		return archive.getPos();\
	}\
	DWORD load(char *data)\
	{\
		zArchive<T> archive(data,0,ARCHIVE_HEAD,SERIALIZE_LOAD);\
		archive >> (*this);\
		return archive.getPos();\
	}\

#define FIR_SERIALIZE_CMD_LOAD(type)\
	size += CSerializeManager::unSerializeBaseType(type,&data[size]);

#define FIR_SERIALIZE_CMD_SAVE(type)\
	size += CSerializeManager::serializeBaseType(type, &data[size]);

struct zArchiveCmd
{
	virtual ~zArchiveCmd(){}
	virtual void loadBuff(const BYTE *data, QWORD &size) = 0;
	virtual void saveBuff(BYTE *data, QWORD &size) = 0;
};

//玩家binary 加载存储VECTOR宏 vec 为存储或者读取的容器，MEMBER 为容器存储的VALUE类型 name为protobuf里对应的变量名
#define BINARY_SAVE_VEC(vec,MEMBER,name)\
	do{\
		if (!vec.empty())\
		{\
			auto it = vec.begin();\
			for (; it != vec.end(); ++it)\
			{\
				MEMBER* tmp = binary.add_##name();\
				tmp->CopyFrom(*it);\
			}\
		}\
	}while(0)\


#define BINARY_LOAD_VEC(vec,MEMBER,name)\
	do{\
		for (int i=0; i < binary.name##_size();++i)\
		{\
			const MEMBER & mem = binary.name(i);\
			MEMBER tmp;\
			tmp.CopyFrom(mem);\
			vec.push_back(tmp);\
		}\
	}while(0)\

// 玩家binary 加载存储MAP宏 map 为存储或者读取的容器，MEMBER 为容器存储的VALUE类型 name为protobuf里对应的变量名
#define BINARY_SAVE_MAP(map,MEMBER,name)\
	do{\
		if (!map.empty())\
		{\
			auto it = map.begin();\
			for (; it != map.end(); ++it)\
			{\
				MEMBER* tmp = binary.add_##name();\
				tmp->CopyFrom(it->second);\
			}\
		}\
	}while(0)

#define BINARY_LOAD_MAP(map,MEMBER,name)\
	do{\
		for (int i=0; i < binary.name##_size();++i)\
		{\
			const MEMBER & mem = binary.name(i);\
			MEMBER tmp;\
			tmp.CopyFrom(mem);\
			map[tmp.id()] = tmp;\
		}\
	}while(0)

// 玩家binary 加载存储结构体宏 member为存储或者读取的结构体 MEMBER 为 结构体类型 name 为protobuf里对应的变量名
#define BINARY_SAVE_POD(member,MEMBER,name)\
	do{\
		MEMBER *tmp = binary.mutable_##name();\
		tmp->CopyFrom(member);\
	}while(0)

#define BINARY_LOAD_POD(member,name)\
	do{\
		member.CopyFrom(binary.name());\
	}while(0)

// 玩家binary 加载存储整形结构 member 为存储或者读取的变量 name 为protobuf里对应的变量名
#define BINARY_SAVE_INT(member,name)\
	do{\
		binary.set_##name(member);\
	}while(0)

#define BINARY_LOAD_INT(member,name)\
	do{\
		member = binary.name();\
	}while(0)


#endif
