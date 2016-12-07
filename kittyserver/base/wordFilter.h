#ifndef _WORD_FILTER_H
#define _WORD_FILTER_H

#include <fstream>
#include "zRegex.h"
#include "zSingleton.h" 

/**
 * \brief 禁言过虑类
 *
 */
class wordFilter : public Singleton<wordFilter>
{
	friend class Singleton<wordFilter>;
	private:
		static wordFilter *instance;
		std::map<std::string, std::string> forbidWords;
		wordFilter()
		{
			init();
		}
		void init();
	public:
		bool doFilter(char *text, unsigned int cmdLen);
        bool hasForbitWord(const char *text);
        bool doFilter(std::string & strstring);
};



#endif
