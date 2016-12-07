/*
                 __________________        __________          ___________________
                /                 /\      /         /\        /                  /\
               /   ______________/ /     /___   ___/ /       /  _____________   / /
              /   / \____________\/      \__/  / __\/       /  /\__________ /  / /
             /   / /                       /  / /          /  / /          /  / /
            /   /_/____________           /  / /          /  /_/__________/  / /
           /                  /\         /  / /          /                  / /
          /  ________________/ /        /  / /          /      ____________/ /
         /  / _______________\/        /  / /          /  /\   \___________\/
        /  / /                        /  / /          /  / /\   \
       /  / /                        /  / /          /  / /  \   \
      /  / /                        /  / /          /  / /    \   \
     /  / /                     ___/  /_/_         /  / /      \   \
    /  / /                     /         /\       /  / /        \   \
   /__/ /                     /_________/ /      /__/ /          \___\
   \__\/                      \_________\/       \__\/            \___\


	Fir Game Engine

	Files:		xmlparser.h, xmlparser.cpp
	Author:		CHENG XIE
	Describe:	封装了rapidxml 解析器.

	$Id: xmlparser.h 154 2013-05-15 02:56:29Z chengxie $

*/

#ifndef __XMLPARSER_H__
#define __XMLPARSER_H__
#include <string>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>
#include "Fir.h"
#include "vartype.h"

namespace Fir {

	class XMLParser {
		public:
			typedef	xmlDoc XmlDoc;
			typedef	xmlNode	Node;

			XMLParser();
			virtual ~XMLParser();

			void final();

			/*!
			  \brief 从文件加载
			  */
			void load_from_file(const char *filename);

			/*!
			  \brief 从内存加载
			  */
			void load_from_memory(const char *buf);

			/*!
			  \brief 获得根节点
			  \return Node* 返回节点
			  */
			const Node *root() const;

			/*!
			  \brief 获得子节点
			  \param parent 父节点
			  \param name 子节点名称
			  \return Node* 返回节点
			  */
			const Node *child(const Node *parent, const char *name = NULL) const;

			/*!
			  \brief 获得下一个节点
			  \param node 本节点
			  \param name 下个节点名称
			  \return Node *
			  */
			const Node *next(const Node *node, const char *name = NULL) const;

			/*!
			  \brief 获得子节点的数目
			  \param parent 父节点
			  \param name 子节点名称
			  \return unsigned int
			  */
			unsigned int child_count(const Node *parent, const char *name = NULL) const;

            /*!
             \brief 获得节点属性值
             \param node 节点
             \param name 属性字段名
             \return 属性值
             */
            bool has_attribute(const Node *node, const char *name) const;
        
			/*!
			  \brief 获得节点属性值
			  \param node 节点
			  \param name 属性字段名
			  \return 属性值
			  */
			VarType node_attribute(const Node *node, const char *name) const;

			/*!
			  \brief 获得节点内容值
			  \param node 节点
			  \return 内容值
			  */
			VarType node_value(const Node *node) const;

		private:

			XmlDoc *_doc;
	};
}
#endif

