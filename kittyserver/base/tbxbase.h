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
	数据表格的管理器模板
	$Id: tbxbase.h 108 2013-05-07 07:03:00Z chengxie $

*/


#ifndef __TBXBASE_H__
#define __TBXBASE_H__
#include <unordered_map>
#include <functional>
#include "Fir.h"
#include "zSingleton.h"
#include "xmlparser.h"
#include "tbx.h"
#include "dataManager.h"

using namespace std::placeholders;

namespace tbx
{

	typedef std::function<void (const std::string &, byte *&, int &)> ReadTbxBufFunc;

	template <typename table_type, typename conf_type,typename tbx_type>
		class table 
        {
			public:
				typedef std::function<bool (const conf_type&)> for_each_func;

			public:
				table() {}
				virtual ~table() {}

				inline const conf_type &get(unsigned int id) const 
                {
					static conf_type tmp;
					if (!id)
					{
						Fir::logger->error("not found id %u in table \"%s\".\n", id, _proto.GetTypeName().c_str());
						return tmp;
					}
					auto it = _tbx_map.find(id);
					if (it == _tbx_map.end()) 
					{
						Fir::logger->error("not found id %u in table \"%s\".\n", id, _proto.GetTypeName().c_str());
						return tmp;
					}
					return *it->second;
				}
				inline const conf_type * get_base(unsigned int id) const 
                {
					auto it = _tbx_map.find(id);
					if (it != _tbx_map.end())
						return it->second;
					return NULL;
				}

				inline unsigned int count() const 
                {
					return _tbx_map.size(); 
				}

				void load(const std::string &filename, const tbx::ReadTbxBufFunc &func) 
                {
					byte *buf = NULL;
					int size = 0;
					func(filename, buf, size);
					if (buf) 
                    {
						_proto.ParseFromArray(buf, size);
						SAFE_DELETE_VEC(buf);
					}
                    Fir::logger->debug("load the tbx %s",filename.c_str());
					for (int i = 0; i < _proto.datas_size(); ++i)
                    {
						tbx_type *obj = _proto.mutable_datas(i);
                        conf_type *confInst = new conf_type(obj);
                        confInst->init();
						_tbx_map[confInst->getKey()] = confInst;
                     #if 0
                        if(strcmp(filename.c_str(),"buildFunction.tbx") == 0)
                        {
                            pb::buildFunction::t_buildFunction *testObj = (pb::buildFunction::t_buildFunction*)obj;
                            Fir::logger->debug("print test tbxid:%u,id:%s,name:%s,info:%u",testObj->tbxid(),testObj->id().c_str(),testObj->name().c_str(),testObj->info());
                        }
                    #endif
					}
				}

				void for_each(const tbx::table<table_type,conf_type,tbx_type>::for_each_func &func) const 
                {
					for (int i = 0; i < _proto.datas_size(); ++i)
						if (!func(_proto.datas(i)))
							break;
				}
				typedef typename std::unordered_map<unsigned int, const conf_type*>::const_iterator _tbx_iter;
				_tbx_iter begin() const
				{
					return _tbx_map.begin();
				}
				_tbx_iter end() const
				{
					return _tbx_map.end();
				}
                const std::unordered_map<unsigned int, const conf_type*> getTbxMap() const
                {
                    return _tbx_map;
                }

			private:
				table_type _proto;
				std::unordered_map<unsigned int, const conf_type*>	_tbx_map;
		};


	class mgr : public Fir::Singleton<mgr>
    {

		public:
			mgr() {}
			virtual ~mgr() {}

			void init(Fir::XMLParser &xml, const tbx::ReadTbxBufFunc &func);

		private:

			template <typename table_type,typename conf_typ,typename tbx_type>
				void load_table(const std::string &name, tbx::table<table_type,conf_typ,tbx_type> &tb, const ReadTbxBufFunc &func) 
                {
					auto it = _tbx_files.find(name);
					if (it != _tbx_files.end())
						tb.load(it->second, func);
				}

			void loadconfig(const Fir::XMLParser &xml) {
				const Fir::XMLParser::Node *root = xml.root();
				if (root) 
                {
					const Fir::XMLParser::Node *node = xml.child(root, "file");
					while (node) 
                    {
						std::string name = xml.node_attribute(node, "name");
						std::string path = xml.node_attribute(node, "path");
						_tbx_files[name] = path;
						node = xml.next(node, "file");
					}
				}
			}

		private:

			std::map<const std::string, std::string> _tbx_files;
	};

}


#endif

