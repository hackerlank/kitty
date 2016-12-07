#pragma once
#include <set>

template <class T, unsigned int a = 10>
class zSort
{
	std::set<T> data;
	typedef typename std::set<T>::iterator data_iter;

	unsigned int max;
	public:
	zSort(unsigned int m=a):max(m){}
	void setMax(unsigned int m){max = m;}

	data_iter begin(){ return data.begin();}
	data_iter end(){return data.end();}
	void insert(const T &v)
	{
		data_iter it = std::find(data.begin(), data.end(), v);
		if (it!=data.end()) data.erase(it);

		data.insert(v);
		if (!data.empty() && data.size()>max)
		{
			data_iter i = data.end();
			i--;
			data.erase(i);
		}
	}

	unsigned int getData(T *mem, unsigned int num)
	{
		unsigned int i=0;
		for (data_iter it=data.begin(); it!=data.end() && i<num; it++,i++)
		{
			mem[i] = *it;
		}
		return i;
	}

	unsigned int getPlace(const T &t)
	{
		unsigned int i=1;
		for (data_iter it=data.begin(); it!=data.end(); it++,i++)
		{
			if (*it==t) return i;
		}
		return 0;
	}

	const T *find(const T &v)
	{
		for (data_iter it=data.begin(); it!=data.end(); it++)
		{
			if (*it==v) return &(*it);
		}
		return 0;
	}

	void clear(){data.clear();}

	void erase(const T &v)
	{
		data_iter it = std::find(data.begin(), data.end(), v);
		if (it!=data.end()) data.erase(it);
	}

	unsigned int getDataByPlace(T &t, unsigned int num)
	{
		unsigned int i=1;
		for (data_iter it=data.begin(); it!=data.end() && i<=num; it++,i++)
		{
			if (i == num)
			{
				t = *it;
				return i;
			}
		}
		return 0;
	}

	unsigned int size(){ return data.size();}
};
