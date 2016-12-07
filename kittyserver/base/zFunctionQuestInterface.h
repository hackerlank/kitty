#ifndef _FUNCTIONQUESTINTERFACE_H_
#define _FUNCTIONQUESTINTERFACE_H_

template <typename T>
class FunctionQuestInterface
{
private:
	DWORD m_id;	//任务编号
public:	
	FunctionQuestInterface(const DWORD id) : m_id(id) {};
	virtual ~FunctionQuestInterface() {};
	DWORD getID() { return m_id; };		//获取任务编号
	virtual WORD getFinishNum(T* lpUser, tm& tv_cur, time_t curTime) = 0;	//查询当天完成次数
	virtual bool isQuestState(T* lpUser, tm& tv_cur, time_t curTime) = 0;	//是否在任务状态中
};

#endif


