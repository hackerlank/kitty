#include "CmdAnalysis.h"
#include <string.h>

CmdAnalysis::CmdAnalysis(DWORD i, const char *disc, DWORD time_secs)
:_log_timer(time_secs)
{
	id = i;
	bzero(_disc, sizeof(disc));
	strncpy(_disc, disc, sizeof(_disc)-1);
	off();
}

void CmdAnalysis::on()
{
	_switch = true;
}

void CmdAnalysis::off()
{
	_switch = false;
	clear();
}

void CmdAnalysis::clear()
{
	cmdList.clear();
}

void CmdAnalysis::add(const BYTE &cmd, const BYTE &para , const DWORD &size)
{   
	if(!_switch) return;

	_mutex.lock();
	cmdList[(cmd<<8)+para].num++;
	cmdList[(cmd<<8)+para].size += size;

	zRTime ct;
	if(_log_timer(ct))
	{   
		showLog();
		clear();
	}
	_mutex.unlock();
}

void CmdAnalysis::showLog()
{   
	Fir::logger->debug("[cmdCount]-----------%s------------", _disc);
	for (cmd_iter it=cmdList.begin(); it!=cmdList.end(); it++)
		Fir::logger->debug("[cmdCount]%u:%u, %u, %u, %lu", id, it->first>>8, it->first&0x00ff, it->second.num, it->second.size);
	Fir::logger->debug("[cmdCount]-----------%s------------", _disc);
}
