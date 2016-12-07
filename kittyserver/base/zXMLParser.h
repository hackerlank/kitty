/**
 * \file
 * \version  $Id: zXMLParser.h 13 2013-03-20 02:35:18Z  $
 * \author  ,okyhc@263.sina.com
 * \date 2004年11月03日 14时15分45秒 CST
 * \brief zXMLParser器定义文件
 *
 * 
 */

#ifndef _ZXMLPARSE_H_
#define _ZXMLPARSE_H_
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include <string>

/**
 * \brief zXMLParser定义
 * 
 * 主要提供了节点的浏览,和其属性的得到.
 */
class zXMLParser
{
	public:
		zXMLParser();
		~zXMLParser();

		bool initFile(const std::string &xmlFile);
		bool initFile(const char *xmlFile);
		bool initStr(const std::string &xmlStr);
		bool initStr(const char *xmlStr);
		bool init();
		void final();
		std::string & dump(std::string & s, bool format=false);
		std::string & dump(xmlNodePtr dumpNode,std::string & s,bool head=true);
		std::string & dumpNohead(std::string & s);
		xmlNodePtr getRootNode(const char *rootName);
		xmlNodePtr getChildNode(const xmlNodePtr parent, const char *childName);
		xmlNodePtr getNextNode(const xmlNodePtr node,const char *nextName);
		unsigned int getChildNodeNum(const xmlNodePtr parent, const char *childName);

		xmlNodePtr newRootNode(const char *rootName);
		xmlNodePtr newChildNode(const xmlNodePtr parent, const char *childName, const char *content);
		bool newNodeProp(const xmlNodePtr node,const char *propName,const char *prop);
		bool newNodeProp(const xmlNodePtr node,const char *propName,const char *prop,const char* encoding);

		bool isNodeName(const xmlNodePtr node, const std::string& name);

		bool getNodePropNum(const xmlNodePtr node,const char *propName,void *prop,int propSize);
		bool getNodePropTime(const xmlNodePtr node,const char *propName,time_t &prop);
		bool getNodePropStr(const xmlNodePtr node,const char *propName,void *prop,int propSize);
		bool getNodePropStr(const xmlNodePtr node,const char *propName,std::string &prop);
		
		bool getNodeContentNum(const xmlNodePtr node,void *content, int contentSize);
		bool getNodeContentTime(const xmlNodePtr node,time_t &prop);
		bool getNodeContentStr(const xmlNodePtr node,void *content, int contentSize);
		bool getNodeContentStr(const xmlNodePtr node,std::string &content);
	private:
		unsigned char *charConv(unsigned char *in, const char *fromEncoding, const char *toEncoding);
		xmlDocPtr doc;
};
#endif
