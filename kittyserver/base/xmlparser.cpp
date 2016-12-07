#include "xmlparser.h"

namespace Fir {

	XMLParser::XMLParser() : _doc(NULL)
	{
	}

	XMLParser::~XMLParser()
	{
		final();
	}

	void XMLParser::final()
	{
		if (_doc) {
			xmlFreeDoc(_doc);
			_doc = NULL;
		}
	}

	void XMLParser::load_from_file(const char *filename)
	{
		if (filename) {
			final();
			_doc = xmlParseFile(filename);
		}
	}

	void XMLParser::load_from_memory(const char *buf)
	{
		if (buf) {
			final();
			_doc = xmlParseDoc((xmlChar *)buf);
		}
	}

	const XMLParser::Node *XMLParser::root() const
	{
		return _doc ? xmlDocGetRootElement(_doc) : NULL;
	}

	const XMLParser::Node *XMLParser::child(const Node *parent, const char *name) const
	{
		if (!parent) 
			return NULL;

		for (xmlNode *child = parent->children; child; child = child->next)
			if (name ? !xmlStrcmp(child->name, (const xmlChar *)name) : !xmlNodeIsText(child))
				return child;
		
		return NULL;
	}

	const XMLParser::Node *XMLParser::next(const Node *node, const char *name) const
	{
		if (!node) 
			return NULL;

		for (xmlNode *next = node->next; next; next = next->next)
			if (name ? !xmlStrcmp(next->name, (const xmlChar *)name) : !xmlNodeIsText(next))
				return next;
		
		return NULL;
	}

	unsigned int XMLParser::child_count(const Node *parent, const char *name) const
	{
		unsigned int count = 0;
		if (parent) {
			const Node *node = child(parent, name);
			while (node) {
				++count;
				node = next(node, name);
			}
		}

		return count;	
	}

    bool XMLParser::has_attribute(const Node *node, const char *name) const
    {
		char *temp = (char *)xmlGetProp((xmlNodePtr)node, (const xmlChar *)name);
		if (temp) {
			xmlFree(temp);
			return true;
		}
        return false;
    }
    
	VarType XMLParser::node_attribute(const Node *node, const char *name) const
	{
		VarType ret;
		if (node && name) {
			char *temp = (char *)xmlGetProp((xmlNodePtr)node, (const xmlChar *)name);
			if (temp) {
				ret = temp;
				xmlFree(temp);
			}
		}
		return ret;
	}

	VarType XMLParser::node_value(const Node *node) const
	{
		VarType ret;

		if (node) {
			for (xmlNode *text = node->children; text; text = text->next) {
				//if (!xmlStrcmp(text->name, (const xmlChar *) "text")) {
				if (xmlNodeIsText(text)) {
					ret = (const char *)text->content;
					break;
				}
			}
		}

		return ret;
	}


}
