/**
 * \file
 * \version     $Id: dumpXml.cpp 60836 2012-08-07 02:00:28Z liuxin3 $
 * \author      liuxin
 * \date        2012-08-07
 * \brief       XML文件读取系统
 * 
 */

#include "node.h"

void byteAttr::dumpHeadFile(std::ostream &out)
{
	out << "BYTE " << name << ";" << endl;
}

void wordAttr::dumpHeadFile(std::ostream &out)
{
	out << "WORD " << name << ";" << endl;
}

void dwordAttr::dumpHeadFile(std::ostream &out)
{
	out << charid" " << name << ";" << endl;
}

void dvecAttr::dumpHeadFile(std::ostream &out)
{
	out << "std::vector<DWORD> " << name << ";" << endl;	 
}

void dmapAttr::dumpHeadFile(std::ostream &out)
{
	out << "std::map<DWORD,DWORD> " << name << ";" << endl;
}

void dsetAttr::dumpHeadFile(std::ostream &out)
{                       
	out << "std::set<DWORD> " << name << ";" << endl;
}

void strAttr::dumpHeadFile(std::ostream &out)
{
	out << "std::string " << name << ";" << endl;
}

void timeAttr::dumpHeadFile(std::ostream &out)
{
	out << "time_t " << name << ";" << endl;
}

void byteAttr::dumpCppFile(std::ostream &out)
{
	out << " xml.getNodePropNum(node, \"" << name << "\", &" << name << ", sizeof(" << name << "));" << endl; 
}

void wordAttr::dumpCppFile(std::ostream &out)
{
	out << " xml.getNodePropNum(node, \"" << name << "\", &" << name << ", sizeof(" << name << "));" << endl;
}

void dwordAttr::dumpCppFile(std::ostream &out)
{
	out << " xml.getNodePropNum(node, \"" << name << "\", &" << name << ", sizeof(" << name << "));" << endl;
}

void dvecAttr::dumpCppFile(std::ostream &out)
{
	out << endl;
	out << name << ".clear();" << endl;
	out << "{" << endl;
	out << "std::string str;" << endl;
	out << "std::vector<std::string> vs;" << endl;
	out << "vs.clear();" << endl;
	out << "xml.getNodePropStr(node, " << "\"" << name << "\"" << ", str);" << endl;
	out << "Fir::stringtok(vs, str.c_str(), \";\");" << endl;
	out << "for (DWORD i=0; i<vs.size(); ++i)" << endl;
	out << "{" << endl;
	out << name << ".push_back(atoi(vs[i].c_str()));" << endl;
	out << "}" << endl;
	out << "}" << endl;
}

void dmapAttr::dumpCppFile(std::ostream &out)
{
	out << endl;
	out << name << ".clear();" << endl;
	out << "{" << endl;
	out << "std::string str;" << endl;
	out << "std::vector<std::string> vs;" << endl;
	out << "vs.clear();" << endl;
	out << "xml.getNodePropStr(node, " << "\"" << name << "\"" << ", str);" << endl;
	out << "Fir::stringtok(vs, str.c_str(), \";\");" << endl;
	out << "for (DWORD i=0; i<vs.size(); ++i)" << endl;
	out << "{" << endl;
	out << charid" x = 0;" << endl;
	out << charid" y = 0;" << endl;
	out << "sscanf(vs[i].c_str(), \"%u-%u\", &x, &y);" << endl;
	out << name << ".insert(std::make_pair(x,y));" << endl;
	out << "}" << endl;
	out << "}" << endl;
}

void dsetAttr::dumpCppFile(std::ostream &out)
{
	out << endl;
	out << name << ".clear();" << endl;
	out << "{" << endl;
	out << "std::string str;" << endl;
	out << "std::vector<std::string> vs;" << endl;
	out << "vs.clear();" << endl;
	out << "xml.getNodePropStr(node, " << "\"" << name << "\"" << ", str);" << endl;
	out << "Fir::stringtok(vs, str.c_str(), \";\");" << endl;
	out << "for (DWORD i=0; i<vs.size(); ++i)" << endl;
	out << "{" << endl;
	out << name << ".insert(atoi(vs[i].c_str()));" << endl;
	out << "}" << endl;
	out << "}" << endl;
}

void strAttr::dumpCppFile(std::ostream &out)
{
	out << " xml.getNodePropStr(node, \"" << name << "\"," << name << ");" << endl;
}

void timeAttr::dumpCppFile(std::ostream &out)
{
	out << " xml.getNodePropTime(node, \"" << name << "\", " << name << ");" << endl;
}

void dumpNode::printNode()
{
	cout << name << endl;
	cout << type << endl;
	vector<dumpNode *>::iterator it = subNodeVec.begin();
	for (; it != subNodeVec.end(); ++it)
	{
		dumpNode *sNode = *it;
		cout << sNode->name << endl;
		cout << sNode->type << endl;
		vector<dumpNode *>::iterator sit  = sNode->subNodeVec.begin();
		for (; sit != sNode->subNodeVec.end(); ++sit)
		{
			(*sit)->printNode();
		}
	}
}

void dumpNode::dumpInitFile(std::ostream &out)
{
	vector<dumpNode *>::iterator it = subNodeVec.begin();
	for (; it != subNodeVec.end(); ++it)
	{       
		
		dumpNode *sNode = *it;
		vector<dumpNode *>::iterator sit  = sNode->subNodeVec.begin();
		for (; sit != sNode->subNodeVec.end(); ++sit)
		{
			(*sit)->dumpInitFile(out);
		}
	}

}

bool classNode::init(zXMLParser &xml, xmlNodePtr node)
{
	if (!node) return false;
	
	//先读属性
	xmlAttrPtr nptr = node->properties;
	while(nptr)
	{
		if (!strncmp((char *)nptr->name, "bd", 2))
		{
			attrVec.push_back(new byteAttr((char *)nptr->name));
		}else if (!strncmp((char *)nptr->name, "wd", 2))
		{
			attrVec.push_back(new wordAttr((char *)nptr->name));
		}else if (!strncmp((char *)nptr->name, "dw", 2))
		{
			attrVec.push_back(new dwordAttr((char *)nptr->name));
		}else if (!strncmp((char *)nptr->name, "dvec", 4))
		{
			attrVec.push_back(new dvecAttr((char *)nptr->name));
		}else if (!strncmp((char *)nptr->name, "dmap", 4))
		{
			attrVec.push_back(new dmapAttr((char *)nptr->name));
		}else if (!strncmp((char *)nptr->name, "dset", 4))
		{
			attrVec.push_back(new dsetAttr((char *)nptr->name));
		}else if (!strncmp((char *)nptr->name, "str", 3))
		{
			attrVec.push_back(new strAttr((char *)nptr->name));
		}else if (!strncmp((char *)nptr->name, "tm", 2))
		{
			 attrVec.push_back(new timeAttr((char *)nptr->name));
		}
		nptr = nptr->next;
	}

	//再读节点	
	xmlNodePtr memberNode = xml.getChildNode(node, NULL);
	while (memberNode)
	{
		if (!strncmp((char *)memberNode->name, "bd", 2))
		{
			subNodeVec.push_back(new ByteNode((char *)memberNode->name));
		}else if (!strncmp((char *)memberNode->name, "wd", 2))
		{
			subNodeVec.push_back(new WordNode((char *)memberNode->name));
		}else if (!strncmp((char *)memberNode->name, "dw", 2))
		{
			subNodeVec.push_back(new DwordNode((char *)memberNode->name));
		}else if (!strncmp((char *)memberNode->name, "dvec", 4))
		{
			subNodeVec.push_back(new dvecNode((char *)memberNode->name));
		}else if (!strncmp((char *)memberNode->name, "dmap", 4))
		{
			subNodeVec.push_back(new dmapNode((char *)memberNode->name));
		}else if (!strncmp((char *)memberNode->name, "dset", 4))
		{
			subNodeVec.push_back(new dsetNode((char *)memberNode->name));
		}else if (!strncmp((char *)memberNode->name, "map", 3))
		{
			dumpNode * pMapNode = new mapNode((char *)memberNode->name);
			pMapNode->init(xml,memberNode);
			subNodeVec.push_back(pMapNode);
		}else if (!strncmp((char *)memberNode->name, "vec", 3))
		{
			dumpNode * pVecNode = new vecNode((char *)memberNode->name);
			pVecNode->init(xml,memberNode);
			subNodeVec.push_back(pVecNode);	
		}else if (!strncmp((char *)memberNode->name, "timer",5))
		{
			subNodeVec.push_back(new timerNode((char *)memberNode->name));
		}
		memberNode = xml.getNextNode(memberNode,NULL);
	}
	return true;
}

bool vecNode::init(zXMLParser &xml, xmlNodePtr node)
{
	if (!node) return false;
	xmlNodePtr subNode = xml.getChildNode(node,NULL);
	if (subNode)
	{
		dumpNode *pRootNode = new classNode((char *)subNode->name);
		strncpy(pRootNode->name,(char *)subNode->name,sizeof(pRootNode->name));
		pRootNode->init(xml,subNode);
		subNodeVec.push_back(pRootNode);
		subNode = xml.getNextNode(subNode, NULL);
	}
	return true;
}

bool mapNode::init(zXMLParser &xml, xmlNodePtr node)
{
	if (!node) return false;

	//先读属性,只支持容器嵌套
	xmlAttrPtr nptr = node->properties;
	while(nptr)
	{
		if (!strncmp((char *)nptr->name, "dvec", 4))
		{
			attrVec.push_back(new dvecAttr((char *)nptr->name));
		}else if (!strncmp((char *)nptr->name, "dmap", 4))
		{
			attrVec.push_back(new dmapAttr((char *)nptr->name));
		}else if (!strncmp((char *)nptr->name, "dset", 4))
		{
			attrVec.push_back(new dsetAttr((char *)nptr->name));
		}
	}
	//再读节点      
	xmlNodePtr subNode = xml.getChildNode(node,NULL);
	if (subNode)
	{
		dumpNode *pRootNode = new classNode((char *)subNode->name);
		strncpy(pRootNode->name,(char *)subNode->name,sizeof(pRootNode->name));
		pRootNode->init(xml,subNode);
		subNodeVec.push_back(pRootNode);
		subNode = xml.getNextNode(subNode, NULL);
	}
	return true;
}

void classNode::dumpHeadFile(std::ostream &out)
{
	out << "struct" << " " << name << endl;
	out << "{" << endl;
	out << name << "()" <<endl;
	out << "{" << endl;
	out << "}" << endl;

	//先打印属性
	vector<attrbute *>::iterator ait = attrVec.begin();
	for (; ait != attrVec.end(); ++ait)
	{
		(*ait)->dumpHeadFile(out);
	}
	
	//再打印节点
	vector<dumpNode *>::iterator it = subNodeVec.begin();
	for (; it != subNodeVec.end(); ++it)
	{
		(*it)->dumpHeadFile(out);
	}
	out << "bool init(zXMLParser &xml, xmlNodePtr node);" << endl;
	out << "};" << endl;
}

void ByteNode::dumpHeadFile(std::ostream &out)
{
	out << "BYTE " << name << ";" << endl;
}

void WordNode::dumpHeadFile(std::ostream &out)
{
	out << "WORD " << name << ";" << endl;
}

void DwordNode::dumpHeadFile(std::ostream &out)
{
	out << charid" " << name << ";" << endl;
}

void vecNode::dumpHeadFile(std::ostream &out)
{
	if (!subNodeVec.empty())
	{
		dumpNode * pSubNode = (*subNodeVec.begin());
		pSubNode->dumpHeadFile(out);
		out << "std::vector<" << pSubNode->name << "> " << name << ";" << endl;
	}
}

void mapNode::dumpHeadFile(std::ostream &out)
{       
	if (!subNodeVec.empty())
	{
		dumpNode * pSubNode = (*subNodeVec.begin());
		pSubNode->dumpHeadFile(out);
		out << "std::map<" << charid"," << pSubNode->name << "> " << name << ";" << endl;
	}
}               

void dvecNode::dumpHeadFile(std::ostream &out)
{
	out << "std::vector<DWORD> " << name << ";" << endl;	 
}

void dmapNode::dumpHeadFile(std::ostream &out)
{
	out << "std::map<DWORD,DWORD> " << name << ";" << endl;
}

void dsetNode::dumpHeadFile(std::ostream &out)
{                       
	out << "std::set<DWORD> " << name << ";" << endl;
}               

void timerNode::dumpHeadFile(std::ostream &out)
{
	out << "struct" << " " << name << " : public ActTimer" <<endl;
	out << "{" << endl;
	out << name << "();" <<endl;
	out << "void v_start();" << endl;
	out << "void v_timer(DWORD cur);" << endl;
	out << "void v_end();" << endl;
	out << "}" << endl;
	out << name << " m_" << name << ";" << endl;
}

void classNode::dumpCppFile(std::ostream &out)
{
	out << "bool " << name << "::init(zXMLParser &xml, xmlNodePtr node)" << endl;
	out << "{" << endl;
	out << "if (!node) return false;" << endl;

	//先打印属性
	vector<attrbute *>::iterator ait = attrVec.begin();
	for (; ait != attrVec.end(); ++ait)
	{
		(*ait)->dumpCppFile(out);
	}
	
	vector<dumpNode *>::iterator it = subNodeVec.begin();
	for (; it != subNodeVec.end(); ++it)
	{
		(*it)->dumpCppFile(out);
	}
	out << "return true;" << endl;
	out << "}" << endl;
}

void ByteNode::dumpCppFile(std::ostream &out)
{
	std::string nodeName = std::string(name) + std::string("Node");
	out << "xmlNodePtr " << nodeName << "= xml.getChildNode(node,\"" << name << "\");" << endl;
	out << "if (" <<  nodeName << ")" << endl;
	out << "{" << endl;
	out << "xml.getNodePropNum(" << nodeName << ",\"value\",&" << name << ",sizeof(" << name << "));" << endl;
	out << "}" << endl;
}

void WordNode::dumpCppFile(std::ostream &out)
{
	std::string nodeName = std::string(name) + std::string("Node");
	out << "xmlNodePtr " << nodeName << "= xml.getChildNode(node,\"" << name << "\");" << endl;
	out << "if (" <<  nodeName << ")" << endl;
	out << "{" << endl;
	out << "xml.getNodePropNum(" << nodeName << ",\"value\",&" << name << ",sizeof(" << name << "));" << endl;
	out << "}" << endl;	
}

void DwordNode::dumpCppFile(std::ostream &out)
{
	std::string nodeName = std::string(name) + std::string("Node");
	out << "xmlNodePtr " << nodeName << "= xml.getChildNode(node,\"" << name << "\");" << endl;
	out << "if (" <<  nodeName << ")" << endl;
	out << "{" << endl;
	out << "xml.getNodePropNum(" << nodeName << ",\"value\",&" << name << ",sizeof(" << name << "));" << endl;
	out << "}" << endl;
}

void vecNode::dumpCppFile(std::ostream &out)
{
	std::string nodeName = std::string(name) + std::string("Node");
        out << "xmlNodePtr " << nodeName << "= xml.getChildNode(node,\"" << name << "\");" << endl;
        out << "if (" <<  nodeName << ")" << endl;
	out << "{" << endl;
	out << name << ".clear();" << endl;
	if (!subNodeVec.empty())
	{
		dumpNode * pSubNode = (*subNodeVec.begin());
		std::string childNodeName = std::string(pSubNode->name) + std::string("Node");
		out << "xmlNodePtr " << childNodeName << " = " << "xml.getChildNode(" << nodeName << ",\"" << pSubNode->name << "\");" << endl;
		out << "while(" << childNodeName << ")" << endl;
		out << "{" << endl;
		out << pSubNode->name << " " << "m_" << pSubNode->name << ";" << endl;
		out << "m_" << pSubNode->name << ".init(xml," << childNodeName << ");" << endl;
		out << name << ".push_back(" << "m_" << pSubNode->name << ");" << endl;
		out << childNodeName << " = " << "xml.getNextNode(" << childNodeName << ",\"" << pSubNode->name << "\");" << endl;
		out << "}" << endl;
	}	
	out << "}" << endl;	
}

void mapNode::dumpCppFile(std::ostream &out)
{
	std::string nodeName = std::string(name) + std::string("Node");
	out << "xmlNodePtr " << nodeName << "= xml.getChildNode(node,\"" << name << "\");" << endl;
	out << "if (" <<  nodeName << ")" << endl;
	out << "{" << endl;
	out << name << ".clear();" << endl;
	if (!subNodeVec.empty())
	{
		dumpNode * pSubNode = (*subNodeVec.begin());
		std::string childNodeName = std::string(pSubNode->name) + std::string("Node");
		out << "xmlNodePtr " << childNodeName << " = " << "xml.getChildNode(" << nodeName << ",\"" << pSubNode->name << "\");" << endl; 
		out << "while(" << childNodeName << ")" << endl;
		out << "{" << endl;
		out << pSubNode->name << " " << "m_" << pSubNode->name << ";" << endl;
		out << "m_" << pSubNode->name << ".init(xml," << childNodeName << ");" << endl;
		out << name << ".insert(std::make_pair(" << "m_" << pSubNode->name << ".dwID," << "m_" << pSubNode->name << "));" << endl; 
		out << childNodeName << " = " << "xml.getNextNode(" << childNodeName << ",\"" << pSubNode->name << "\");" << endl;
		out << "}" << endl;
	}       
	out << "}" << endl;
}

void dvecNode::dumpCppFile(std::ostream &out)
{
	std::string nodeName = std::string(name) + std::string("Node");
	out << "xmlNodePtr " << nodeName << "= xml.getChildNode(node,\"" << name << "\");" << endl;
	out << "if (" <<  nodeName << ")" << endl;
	out << "{" << endl;
	out << name << ".clear();" << endl;
	out << "std::string str;" << endl;
	out << "std::vector<std::string> vs;" << endl;
	out << "xml.getNodePropStr(" << nodeName << ", \"lists\", str);" << endl;
	out << "Fir::stringtok(vs, str.c_str(), \";\");" << endl;
	out << "for (DWORD i=0; i<vs.size(); ++i)" << endl;
	out << "{" << endl;
	out << name << ".push_back(atoi(vs[i].c_str()));" << endl;
	out << "}" << endl;
	out << "}" << endl;
	
}

void dmapNode::dumpCppFile(std::ostream &out)
{

	std::string nodeName = std::string(name) + std::string("Node");
	out << "xmlNodePtr " << nodeName << "= xml.getChildNode(node,\"" << name << "\");" << endl;
	out << "if (" <<  nodeName << ")" << endl;
	out << "{" << endl;
	out << name << ".clear();" << endl;
	out << "std::string str;" << endl;
	out << "std::vector<std::string> vs;" << endl;
	out << "xml.getNodePropStr(" << nodeName << ", \"lists\", str);" << endl;
	out << "Fir::stringtok(vs, str.c_str(), \";\");" << endl;
	out << "for (DWORD i=0; i<vs.size(); ++i)" << endl;
	out << "{" << endl;
	out << charid" x = 0;" << endl;
	out << charid" y = 0;" << endl;
	out << "sscanf(vs[i].c_str(), \"%u-%u\", &x, &y);" << endl;
	out << name << ".insert(std::make_pair(x,y));" << endl;
	out << "}" << endl;
	out << "}" << endl;
}

void dsetNode::dumpCppFile(std::ostream &out)
{
	std::string nodeName = std::string(name) + std::string("Node");
	out << "xmlNodePtr " << nodeName << "= xml.getChildNode(node,\"" << name << "\");" << endl;
	out << "if (" <<  nodeName << ")" << endl;
	out << "{" << endl;
	out << name << ".clear();" << endl;
	out << "std::string str;" << endl;
	out << "std::vector<std::string> vs;" << endl;
	out << "xml.getNodePropStr(" << nodeName << ", \"lists\", str);" << endl;
	out << "Fir::stringtok(vs, str.c_str(), \";\");" << endl;
	out << "for (DWORD i=0; i<vs.size(); ++i)" << endl;
	out << "{" << endl;
	out << name << ".insert(atoi(vs[i].c_str()));" << endl;
	out << "}" << endl;
	out << "}" << endl;

}

void timerNode::dumpCppFile(std::ostream &out)
{
	std::string nodeName = std::string(name) + std::string("Node");
	out << "xmlNodePtr " << nodeName << "= xml.getChildNode(node,\"" << name << "\");" << endl;
	out << "if (" <<  nodeName << ")" << endl;
	out << "{" << endl;
	out << name << ".initTimer(xml," << nodeName << ");" << endl;
	out << "}" << endl;
}

void classNode::dumpInitFile(std::ostream &out)
{
	dumpCppFile(out);
	vector<dumpNode *>::iterator it = subNodeVec.begin();
	for (; it != subNodeVec.end(); ++it)
	{

		dumpNode *sNode = *it;
		sNode->dumpInitFile(out);
		vector<dumpNode *>::iterator sit  = sNode->subNodeVec.begin();
		for (; sit != sNode->subNodeVec.end(); ++sit)
		{
			(*sit)->dumpInitFile(out);
		}
	}
}

int main()
{
	zXMLParser xml;
	if(!xml.initFile(Fir::global["configdir"] + "testXml.xml"))
	{       
		return 0;
	}       
	xmlNodePtr root = xml.getRootNode("config");
	if (root)
	{
		vector<dumpNode *> nodeVec;
		xmlNodePtr snode = xml.getChildNode(root, NULL); 
		while (snode)
		{
			dumpNode *pRootNode = new classNode((char *)snode->name);
			pRootNode->init(xml,snode);	
			nodeVec.push_back(pRootNode);
			snode = xml.getNextNode(snode, NULL);	
		}

		std::string _file_h;
		std::string _file_cpp;
		std::ostringstream os;
		os << VARS_SAVE_PATH;
		os << "stTest.h";
		_file_h = os.str();

		std::ostringstream mos;
		mos << VARS_SAVE_PATH;
		mos << "stTest.cpp";
		_file_cpp = mos.str();

		std::ofstream hf(_file_h.c_str(),std::ofstream::out | std::ofstream::trunc);

		vector<dumpNode *>::iterator dit = nodeVec.begin();
		for (; dit != nodeVec.end(); ++dit)
		{
			(*dit)->dumpHeadFile(hf);
		}
		hf.flush();
		hf.close();
		hf.clear();

		std::ofstream cppf(_file_cpp.c_str(),std::ofstream::out | std::ofstream::trunc);
		if (!cppf)
			cout << "error" << endl ;
		
		dit = nodeVec.begin();
		for (; dit != nodeVec.end(); ++dit)
		{
			(*dit)->dumpInitFile(cppf);
		}
		cppf.flush();
		cppf.close();
		cppf.clear();	

	}	
	return 0;
}


