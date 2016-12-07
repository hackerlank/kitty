/**
   * \file
    * \version     $Id: dumpXml.cpp 60836 2012-08-07 02:00:28Z liuxin3 $
     * \author      liuxin
      * \date        2012-08-07
       * \brief       XML文件读取系统
        * 
	 */

#include "Fir.h"
#include "zXMLParser.h"
#include <iostream>
#include <vector>
#include <list>
#include <vector>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "zSerialize.h"
#include "zMisc.h"
#include "zType.h"
#include <cstdlib>

#define VARS_SAVE_PATH "xmlTools/"
using namespace std;

enum enumType
{
	DATA_TYPE_NONE,
	DATA_TYPE_CLASS,
	DATA_TYPE_BYTE,
	DATA_TYPE_WORD,
	DATA_TYPE_DWORD,
	DATA_TYPE_VEC,
	DATA_TYPE_MAP,
	DATA_TYPE_DVEC,
	DATA_TYPE_DMAP,
	DATA_TYPE_DSET,
	DATA_TYPE_STR,
	DATA_TYPE_TIME,
	DATA_TYPE_TIMER,
};

class attrbute
{
	public:
		attrbute(char *_name)
		{
			type = DATA_TYPE_NONE;
			bzero(name,128);
			strncpy(name,_name,128);
		}	
		virtual ~attrbute(){}
		char name[128];
		enumType type;
		virtual void dumpHeadFile(std::ostream &out) = 0;
		virtual void dumpCppFile(std::ostream &out) = 0;
};

class byteAttr : public attrbute
{
	public:
		byteAttr(char *_name) : attrbute(_name)
		{
			type = DATA_TYPE_BYTE;
		}
		void dumpHeadFile(std::ostream &out);
		void dumpCppFile(std::ostream &out);	
};

class wordAttr : public attrbute
{
	public:
		wordAttr(char *_name) : attrbute(_name)
		{
			type = DATA_TYPE_WORD;
		}
		void dumpHeadFile(std::ostream &out);
		void dumpCppFile(std::ostream &out);	
};

class dwordAttr : public attrbute
{
	public:
		dwordAttr(char *_name) : attrbute(_name)
		{
			type = DATA_TYPE_DWORD;
		}
		void dumpHeadFile(std::ostream &out);
		void dumpCppFile(std::ostream &out);	
};

class dvecAttr : public attrbute
{
	public:
		dvecAttr(char *_name) : attrbute(_name)
		{
			type = DATA_TYPE_DVEC;
		}
		void dumpHeadFile(std::ostream &out);
		void dumpCppFile(std::ostream &out);	
};


class dmapAttr : public attrbute
{
	public:
		dmapAttr(char *_name) : attrbute(_name)
		{
			type = DATA_TYPE_DMAP;
		}
		void dumpHeadFile(std::ostream &out);
		void dumpCppFile(std::ostream &out);	
};

class timeAttr : public attrbute
{
	public:
		timeAttr(char *_name) : attrbute(_name)
		{
			type = DATA_TYPE_TIME;
		}
		void dumpHeadFile(std::ostream &out);
		void dumpCppFile(std::ostream &out);

};

class dsetAttr : public attrbute
{
	public:
		dsetAttr(char *_name) : attrbute(_name)
		{
			type = DATA_TYPE_DSET;
		}
		void dumpHeadFile(std::ostream &out);
		void dumpCppFile(std::ostream &out);	
};

class strAttr : public attrbute
{
	public:
		strAttr(char *_name) : attrbute(_name)
		{
			type = DATA_TYPE_STR;
		}		
		void dumpHeadFile(std::ostream &out);
		void dumpCppFile(std::ostream &out);
};

class dumpNode
{
	public:
		dumpNode(char *_name)
		{
			type = DATA_TYPE_NONE;
			bzero(name,128);
			strncpy(name,_name,128);
		}
		virtual ~dumpNode(){}
		char name[128];
		vector<dumpNode *> subNodeVec;
		virtual bool init(zXMLParser &xml, xmlNodePtr node) = 0;
		virtual void dumpHeadFile(std::ostream &out) = 0;
		virtual void dumpCppFile(std::ostream &out) = 0;
		virtual void dumpInitFile(std::ostream &out);
		void printNode();
		enumType type;
};

class classNode : public dumpNode
{
	public:
		classNode(char *_name) : dumpNode(_name)
		{
			type = DATA_TYPE_CLASS;
		}
		vector<attrbute *> attrVec;
		bool init(zXMLParser &xml, xmlNodePtr node);
		void dumpHeadFile(std::ostream &out);
		void dumpCppFile(std::ostream &out);
		void dumpInitFile(std::ostream &out);
};

class ByteNode : public dumpNode 
{
	public:
		ByteNode(char *_name) : dumpNode(_name)
		{
			type = DATA_TYPE_BYTE;
		}
		bool init(zXMLParser &xml, xmlNodePtr node)
		{
			return true;
		}
		void dumpHeadFile(std::ostream &out);
		void dumpCppFile(std::ostream &out);
};

class WordNode : public dumpNode 
{
	public:
		WordNode(char *_name) : dumpNode(_name)
		{
			type = DATA_TYPE_WORD;
		}
		bool init(zXMLParser &xml, xmlNodePtr node)
		{
			return true;
		}
		void dumpHeadFile(std::ostream &out);
		void dumpCppFile(std::ostream &out);
};

class DwordNode : public dumpNode 
{
	public:
		DwordNode(char *_name) : dumpNode(_name)
		{
			type = DATA_TYPE_DWORD;
		}
		bool init(zXMLParser &xml, xmlNodePtr node)
		{
			return true;
		}
		void dumpHeadFile(std::ostream &out);
		void dumpCppFile(std::ostream &out);
};

class vecNode : public dumpNode 
{
	public:
		vecNode(char *_name) : dumpNode(_name)
		{
			type = DATA_TYPE_VEC;
		}
		bool init(zXMLParser &xml, xmlNodePtr node);
		void dumpHeadFile(std::ostream &out);
		void dumpCppFile(std::ostream &out);
};

class mapNode : public dumpNode 
{
	public:
		mapNode(char *_name) : dumpNode(_name)
		{
			type = DATA_TYPE_MAP;
		}
		vector<attrbute *> attrVec;
		bool init(zXMLParser &xml, xmlNodePtr node);
		void dumpHeadFile(std::ostream &out);
		void dumpCppFile(std::ostream &out);
};

class dvecNode : public dumpNode 
{
	public:
		dvecNode(char *_name) : dumpNode(_name)
		{
			type = DATA_TYPE_DVEC;
		}
		bool init(zXMLParser &xml, xmlNodePtr node)
		{
			return true;
		}
		void dumpHeadFile(std::ostream &out);
		void dumpCppFile(std::ostream &out);
};

class dmapNode : public dumpNode  
{
	public:
		dmapNode(char *_name) : dumpNode(_name)
		{
			type = DATA_TYPE_DMAP;
		}
		bool init(zXMLParser &xml, xmlNodePtr node)
		{
			return true;
		}
		void dumpHeadFile(std::ostream &out);
		void dumpCppFile(std::ostream &out);
};

class dsetNode : public dumpNode  
{
	public:
		dsetNode(char *_name) : dumpNode(_name)
		{
			type = DATA_TYPE_DSET;
		}
		bool init(zXMLParser &xml, xmlNodePtr node)
		{
			return true;
		}
		void dumpHeadFile(std::ostream &out);
		void dumpCppFile(std::ostream &out);
};

class timerNode : public dumpNode
{
	public:
		timerNode(char *_name) : dumpNode(_name)
		{
			type = DATA_TYPE_TIMER;
		}
		bool init(zXMLParser &xml, xmlNodePtr node)
		{
			return true;
		}
		void dumpHeadFile(std::ostream &out);
		void dumpCppFile(std::ostream &out);
};	
