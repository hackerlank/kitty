#ifndef RETAW_COUNTER_HPP
#define RETAW_COUNTER_HPP

#include "zType.h"

class Counter
{
public:
	enum CounterType
	{
		NORMAL = 0,//不自动清除, 默认取0，配合memset初始化
		YEAR,      //每年自动清除
		MONTH,     //每月自动清除
		WEEK,      //每周自动清除
		DAY,       //每日自动清除
	};
	
public:
	Counter();
	virtual ~Counter();
	
	virtual CounterType getType(const DWORD& item);
	
	virtual QWORD getCounter(const DWORD& item);
	virtual void clearCounter(const DWORD& item);
	virtual void addCounter(const DWORD& item, const QWORD& num = 1);   // +=
	virtual void setCounter(const DWORD& item, const QWORD& value = 1); // 赋值
	
	virtual void load() = 0;//读db
	virtual void save() = 0;//存入db
	
protected:
	virtual bool needClear(const DWORD& item);
	virtual void setType(const DWORD& item, CounterType type);

protected:
	struct Data
	{
		QWORD value;
		DWORD date;
		CounterType type;
	}__attribute__ ((packed));

	Data data[512];//暂定512个，不够时，可以在派生类中其它记录区(如data2[512])，重写相应的virtual functions
	DWORD dataSize;//已使用的计数器个数
};

class TimeTool
{
public:
	static DWORD getCurTimeSec();
	static DWORD getWeekDay(const DWORD& t = 0); //0表示周日
	static bool isSameYear(const DWORD& t1 , const DWORD& t2);
	static bool isSameMonth(const DWORD& t1, const DWORD& t2);
	static bool isSameWeek(const DWORD& t1, const DWORD& t2);//周一为每周第一天
	static bool isSameDay(const DWORD& t1, const DWORD& t2);
	static bool isSameHour(const DWORD& t1, const DWORD& t2);
private:
	static bool isLeapYear(const DWORD& year);
};

#endif
