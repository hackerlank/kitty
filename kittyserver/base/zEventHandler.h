/**
 * \file	zEventHandler.h
 * \version  	$Id: zEventHandler.h 13 2013-03-20 02:35:18Z  $
 * \author  	, @ztgame.com 
 * \date 	2006年05月21日 17时53分22秒 CST
 * \brief 	定义FSM事件处理的接口
 *
 * 
 */

#ifndef _EVENT_HANDLER_H
#define _EVENT_HANDLER_H

class Event;

template<typename T>
class EventHandler
{
	public: 
		typedef int  (T::*Action)(Event*);
		Action handler;
		T* instance;
		
		explicit EventHandler(T* value)
		{       
			instance = value;
		}       

		virtual ~EventHandler(){};

		virtual void action(Event* ev) 
		{       
			(instance->*handler)(ev);
		}       
};

#endif
