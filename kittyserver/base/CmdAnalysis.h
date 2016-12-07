#pragma once
#include "zNullCmd.h"
#include "Fir.h"
#include "zTime.h"
#include <map>

class CmdAnalysis
{
	public:
		CmdAnalysis(DWORD id, const char *disc,DWORD time_secs);
		void clear();

		void on();
		void off();

		void add(const BYTE &cmd, const BYTE &para , const DWORD &size);
		void showLog();

		DWORD id;
	private:
		struct CmdCountStruct
		{
			DWORD num;
			QWORD size;
		};
		std::map<WORD, CmdCountStruct> cmdList;
		typedef std::map<WORD, CmdCountStruct>::iterator cmd_iter;

		char _disc[256];
		zMutex _mutex;
		Timer _log_timer;
		bool _switch;//开关
};
