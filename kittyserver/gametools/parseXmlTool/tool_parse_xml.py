#!/usr/bin/python
#_*_coding:utf-8_*_

from xml.dom import minidom
import xlrd, os, sys, re, getopt,shutil

#生成的cpp文件目录
XML_CPP_DIR_NAME = "xmlcpp"
#生成的管理文件目录
XML_CPP_MANAGER_DIR_NAME = "managerxmlcpp"
#生成的公共文件目录
XML_CPP_COMMON_DIR_NAME = "xmlcommon"
#xml文件夹
XML_DIR_NAME = "xmldir"

    
#读取xml文件
def read_xml(file):
    fp = open(file, "r")
    content = fp.read()
    fp.close()
    try:
        charset = re.compile(".*\s*encoding=\"([^\"]+)\".*", re.M).match(content).group(1)
    except:
        charset = "UTF-8"
    if charset.upper() != "UTF-8":
        content = re.sub(charset, "UTF-8", content)
        content = content.decode(charset).encode("UTF-8")
    return minidom.parseString(content)

#获得容器类型
def parse_container_type(node,objectMap):
    var_type,is_vec,is_map = "",False,False
    for sub_node in node.childNodes:
        if sub_node.nodeType == sub_node.ELEMENT_NODE:
            if var_type == "":
                var_type = sub_node.nodeName
            if sub_node.nodeName != var_type:
                continue
            if sub_node.nodeName == "vector":
                container_type = get_container_type(sub_node, struct_list)
                var_type = "std::vector< {0}{1} >".format(container_type, struct_name_suffix)
                is_vec = True
            elif sub_node.nodeName == "map":
                keyname = sub_node.getAttribute("key")
                if keyname == "":
                    continue
                container_type = get_container_type(sub_node, struct_list)
                var_type = "std::map< {0}, {1}{2} >".format(cpp_var_type, container_type, struct_name_suffix)
                is_map = True
            else:
                object_map = None
                if sub_node.nodeName in objectMap.keys():
                    object_map = objectMap[sub_node.nodeName]
                    parse_xml_node(sub_node,object_map)
                else:
                    object_map = { "struct":{}, "var":{} }
                    parse_xml_node(sub_node,object_map)
                    if len(object_map["var"]) > 0:
                        objectMap[sub_node.nodeName] = object_map 
                        var_type = sub_node.nodeName
    if not is_vec and not is_map and var_type not in objectMap.keys():
        var_type = "Fir::VarType" 
    return var_type

def parse_node(node, objects):
    struct_list = objects["struct"]
    var_list = objects["var"]
    for name, value in node.attributes.items():
        var_list[name] = cpp_var_type
    for sub_node in node.childNodes:
        if sub_node.nodeType == sub_node.ELEMENT_NODE:
            var_name = var_type = sub_node.nodeName
            if sub_node.nodeName == "vector":
                var_name = sub_node.getAttribute("var")
                if var_name == "": continue     #容器必须显式的以var属性指定变量名，否则忽略此节点
                container_type = get_container_type(sub_node, struct_list)
                if container_type in struct_list.keys():
                    var_type = "std::vector< {0}{1} >".format(container_type, struct_name_suffix)
                else:
                    var_type = "std::vector< {0} >".format(container_type)
            elif sub_node.nodeName == "map":
                var_name = sub_node.getAttribute("var")
                if var_name == "": continue
                keyname = sub_node.getAttribute("key")
                if keyname == "": continue  #map必须显式声明keyname，否则忽略这个节点
                container_type = get_container_type(sub_node, struct_list)
                if container_type in struct_list.keys():
                    var_type = "std::map< {0}, {1}{2} >".format(cpp_var_type, container_type, struct_name_suffix)
                else:
                    var_type = "std::map< {0}, {1} >".format(cpp_var_type, container_type)
            else:
                sub_objs = None
                if sub_node.nodeName in struct_list.keys():
                    sub_objs = struct_list[sub_node.nodeName]
                    parse_node(sub_node, sub_objs)
                else:
                    sub_objs = { "struct":{}, "var":{} }
                    parse_node(sub_node, sub_objs)
                    if len(sub_objs["var"]) > 0:
                        struct_list[sub_node.nodeName] = sub_objs
                    else:
                        var_type = cpp_var_type
            if var_name not in var_list:
                var_list[var_name] = var_type



#解析xml节点
def parse_xml_node(node,objectMap):
    object_map = objectMap["struct"]
    value_map = objectMap["var"]
    for name,value in node.attributes.items():
        value_map[name] = "Fir::VarType"
    for sub_node in node.childNodes:
        if sub_node.nodeType == sub_node.ELEMENT_NODE:
            var_name = var_type = sub_node.nodeName
            if sub_node.nodeName == "map":
                var_name = sub_node.getAttribute("var")
                keyname = sub_node.getAttribute("key")
                if var_name == "" or keyname == "":
                    continue
                container_type = parse_container_type(sub_node,object_map)
                if container_type in object_map.keys():
                    var_type = "std::map< {0}, {1}{2} >".format("Fir::VarType",container_type,"_t")
                else:
                    var_type = "std::map< {0}, {1} >".format("Fir::VarType",container_type)
            elif sub_node.nodeName == "vector":
                var_name = sub_node.getAttribute("var")
                if var_name == "":
                    continue
                container_type = parse_container_type(sub_node,object_map)
                if container_type in object_map.keys():
                    var_type = "std::vector< {0}{1} >".format(container_type,"_t")
                else:
                    var_type = "std::vector< {0} >".format(container_type)
                pass
            else:
                sub_objectMap = None
                if sub_node.nodeName in object_map.keys():
                    sub_objectMap = object_map[sub_node.nodeName]
                    parse_xml_node(sub_node,sub_objectMap)
                else:
                    sub_objectMap = { "struct":{}, "var":{} }
                    parse_xml_node(sub_node,sub_objectMap)
                    if len(sub_objectMap["var"]) > 0:
                        object_map[sub_node.nodeName] = sub_objectMap 
                    else:
                        var_type = "Fir::VarType"
            if var_name not in value_map:
                value_map[var_name] = var_type

#创建vector部分
def create_cpp_vector_code(var_name,var_type,objectMap,tab_char):
    code = ""
    object_map = objectMap["struct"]
    vec_type = var_type[var_type.find("< ") + 2 : var_type.find(" >")]
    no_suffix = vec_type[0 : -len("_t")]
    if no_suffix in object_map.keys():
        code += tab_char + "load_vector< %s >(\"%s\", \"%s\", _%s, xml, node);\n" %(vec_type,var_name,no_suffix,var_name)
    elif vec_type == "Fir::VarType":
        code += tab_char + "load_vartype_vector(\"%s\", _%s, xml, node);\n" %(var_name,var_name)
    else:
        code += tab_char + "_%s = error;\n" %(var_name)
        print "暂不支持容器嵌套, 请使用节点嵌套容器! 错误的vector:", var_name
    return code

#创建c++中map部分
def create_cpp_map_code(var_name,var_type,objectMap,tab_char):
    code = ""
    object_map = objectMap["struct"]
    map_type = var_type[var_type.find(", ") + 2 : var_type.find(" >")]
    no_suffix = map_type[0 : -len("_t")]
    code += tab_char + "%s *map_%s_node = xml.child(node, \"map\");\n" %("const Fir::XMLParser::Node",var_name)
    code += tab_char + "while (map_%s_node)\n"  %(var_name)
    code += tab_char + "{\n"
    code += tab_char + "\tif (xml.node_attribute(map_%s_node, \"var\") == \"%s\")\n" %(var_name,var_name)
    code += tab_char + "\t{\n"
    code += tab_char + "\t\t%s keyname = xml.node_attribute(map_%s_node, \"key\");\n" %("Fir::VarType", var_name)

    if no_suffix in object_map.keys():
        map_type = no_suffix
        code += tab_char + "\t\t%s *sub_node = xml.child(map_%s_node, \"%s\");\n" %("const Fir::XMLParser::Node",var_name,map_type)
        code += tab_char + "\t\twhile (sub_node)\n"
        code += tab_char + "\t\t{\n"
        code += tab_char + "\t\t\t%s_t temp_%s;\n" %(map_type,map_type)
        code += tab_char + "\t\t\ttemp_%s.load(xml,sub_node);\n" %(map_type)
        code += tab_char + "\t\t\t_%s[xml.node_attribute(sub_node, keyname)] = temp_%s;\n" %(var_name,map_type)
        code += tab_char + "\t\t\tsub_node = xml.next(sub_node, \"%s\");\n" %(map_type)
        code += tab_char + "\t\t}\n"
    elif map_type == cpp_var_type:  # <==== 此种情况不会出现！map的类型，必定有一个字段是key，所以不会成为VarType类型
        code += tab_char + "\t\t%s *sub_node = xml.child(map_%s_node, NULL);\n" %("const Fir::XMLParser::Node",var_name)
        code += tab_char + "\t\twhile (sub_node) {\n"
        code += tab_char + "\t\t\t_%s[xml.node_attribute(sub_node, keyname)] = xml.node_value(sub_node);\n" %(var_name)
        code += tab_char + "\t\t\tsub_node = xml.next(sub_node, NULL);\n"
        code += tab_char + "\t\t}\n"
    else:
        code += tab_char + "\t\t_%s = error;\n" %(var_name)
        print "暂不支持容器嵌套, 请使用节点嵌套容器! 错误的map:", var_name 
    code += tab_char + "\t\tbreak;\n"
    code += tab_char + "\t}\n"
    code += tab_char + "\tmap_%s_node = xml.next(map_%s_node, \"map\");\n" %(var_name,var_name)
    code += tab_char + "}\n"
    return code



#创建c++类中的成员数据
def create_struct_var_code(objectMap, tab_char):
    code = ""
    object_map = objectMap["struct"]
    value_map  = objectMap["var"]
    for var_name in sorted(value_map):
        var_type = value_map[var_name]
        if var_type in object_map.keys():
            code += tab_char + "%s *%s_node = xml.child(node, \"%s\");\n" %("const Fir::XMLParser::Node",var_name,var_name)
            code += tab_char + "while (%s_node)\n" %(var_name)
            code += tab_char + "{\n"
            code += tab_char + "\t_%s.load(xml, %s_node);\n" %(var_name,var_name)
            code += tab_char + "\t%s_node = xml.next(%s_node, \"%s\");\n" %(var_name,var_name,var_name)
            code += tab_char + "}\n"
        else:
            pass
            if var_type[0:11] == "std::vector":
                code += create_cpp_vector_code(var_name,var_type,objectMap,tab_char)
            elif var_type[0:8] == "std::map":
                code += create_cpp_map_code(var_name,var_type,objectMap,tab_char)
            else:
                code += tab_char + "if (!xml.has_attribute(node, \"%s\"))\n" %(var_name)
                code += tab_char + "{\n"
                code += tab_char + "\t%s *sub = xml.child(node, \"%s\");\n" %("const Fir::XMLParser::Node",var_name)
                
                code += tab_char + "\tif (sub)\n"
                code += tab_char + "\t{\n"
                code += tab_char + "\t\t_%s = xml.node_value(sub);\n" %(var_name)
                code += tab_char + "\t}\n"

                code += tab_char + "}\n"

                code += tab_char + "else\n"
                code += tab_char + "{\n"
                code += tab_char + "\t_%s = xml.node_attribute(node, \"%s\");\n" %(var_name,var_name)
                code += tab_char + "}\n"
    return code



#创建c++代码
def create_struct_code(name,objectMap,tab_char):
    code = ""
    object_map = objectMap["struct"]
    value_map = objectMap["var"]
    code += tab_char + "struct %s_t\n%s{\n" %(name,tab_char)
    if len(object_map) > 0:
        code += tab_char + "\tpublic:\n"
        for struct_name in sorted(object_map):
            code += create_struct_code(struct_name,object_map[struct_name],tab_char+"\t\t")
    if len(value_map) > 0:
        code += tab_char + "\tpublic:\n"
        code += tab_char + "\t\tvoid load(%s &xml, %s *node)\n" %("const Fir::XMLParser","const Fir::XMLParser::Node")
        code += tab_char + "\t\t{\n"
        
        code += tab_char + "\t\t\tif (!node)\n"
        code += tab_char + "\t\t\t{\n"
        code += tab_char + "\t\t\t\treturn;\n"
        code += tab_char + "\t\t\t}\n"
        
        code += create_struct_var_code(objectMap,tab_char+"\t\t\t")
        code += tab_char + "\t\t}\n"
        code += "\n"
        for var_name in sorted(value_map):
            var_type = value_map[var_name]
            if var_type in object_map.keys():
                var_type += "_t"
            code += tab_char + "\t\tconst %s &%s() const { return _%s; }\n" %(var_type,var_name,var_name)
        code += "\n"
        code += tab_char + "\tprivate:\n"
        for var_name in sorted(value_map):
            var_type = value_map[var_name]
            if var_type in object_map.keys():
                var_type += "_t" 
            code += tab_char + "\t\t%s _%s;\n" %(var_type,var_name)
    code += tab_char + "};\n\n"
    return code

#创建xml对应的头文件
def create_cpp_define_file(file,objectMap):
    head = file
    code = "#ifndef _XML_%s_H_\n" %(head.upper())
    code += "#define _XML_%s_H_\n" %(head.upper())

    code += "#include \"xmlcommon.h\"\n"
    code += "\n"

    code += "namespace %s\n{\n " %("config")
    code += create_struct_code(file,objectMap,"\t")
    code += "}\n\n"
    code += "#endif\n\n"
    return code

#创建管理实现文件
def create_manager_cpp_file(xmlFileArr):
    struct_declare = ""
    func_define = ""
    struct_load = ""
    for xmlname in xmlFileArr:
        struct_declare += "\t%s%s _%s;\n" %(xmlname,"_t",xmlname)
        func_define += "\tconst %s%s &%s()\n" %(xmlname,"_t",xmlname)
        func_define += "\t{\n"
        func_define += "\t\treturn _%s;\n" %(xmlname)
        func_define += "\t}\n"
        struct_load += "\t\t_%s.load(xml, func(xml, \"%s.xml\"));\n" %(xmlname,xmlname)
    
    code = "#include \"xmlconfig.h\"\n"
    code += "\n"
    code += "namespace %s\n" %("config")
    code += "{\n"
    code += struct_declare
    code += "\n"
    code += "\tvoid init(const %s::ReadXmlFunc &func)\n" %("config")
    code += "\t{\n"
    code += "\t\t%s xml;\n" %("Fir::XMLParser")
    code += struct_load
    code += "\t}\n"
    code += "\n"
    code += func_define
    code += "}\n\n"
    return code

#创建管理头文件
def create_manager_define_file(xmlFileArr):
    func_declare = ""
    for xmlname in xmlFileArr:
        func_declare += "\tconst %s%s &%s();\n" %(xmlname,"_t",xmlname)
    code = "#ifndef __XML_CONFIG_H__\n"
    code += "#define __XML_CONFIG_H__\n"
    code += "#include <functional>\n"
    for xmlname in xmlFileArr:
        code += "#include \"%s.h\"\n" %(xmlname)
    code += "\n"
    code += "namespace %s\n" %("config")
    code += "{\n"
    code += "\ttypedef std::function<%s *(%s &, const char *)>\tReadXmlFunc;\n\n" %("const Fir::XMLParser::Node","Fir::XMLParser")
    code += "\tvoid init(const %s::ReadXmlFunc &func);\n\n" %("config")
    code += func_declare
    code += "}\n\n"
    code += "#endif\n\n"
    return code

#创建实现vector的文件
def create_cpp_vector_func(tab_char):
    code = tab_char + "template <typename T>\n"
    code += tab_char + "void load_vector(std::string vec_name, std::string sub_nodename, std::vector<T> &var, %s &xml, %s *node)\n" %("const Fir::XMLParser","const Fir::XMLParser::Node")
    code += tab_char + "{\n"
    code += tab_char + "\t%s *vec_node = xml.child(node, \"vector\");\n" %("const Fir::XMLParser::Node")
    code += tab_char + "\twhile (vec_node)\n"
    code += tab_char + "\t{\n"
    code += tab_char + "\t\tif (xml.node_attribute(vec_node, \"var\") == vec_name)\n"
    code += tab_char + "\t\t{\n"
    code += tab_char + "\t\t\tvar.resize(xml.child_count(vec_node, sub_nodename.c_str()));\n" 
    code += tab_char + "\t\t\t%s *sub_node = xml.child(vec_node, sub_nodename.c_str());\n" %("const Fir::XMLParser::Node")
    code += tab_char + "\t\t\tsize_t i = 0;\n"
    code += tab_char + "\t\t\twhile (sub_node && i < var.size())\n"
    code += tab_char + "\t\t\t{\n"
    code += tab_char + "\t\t\t\tvar[i].load(xml, sub_node);\n"
    code += tab_char + "\t\t\t\t++i;\n"
    code += tab_char + "\t\t\t\tsub_node = xml.next(sub_node, sub_nodename.c_str());\n"
    code += tab_char + "\t\t\t}\n"
    code += tab_char + "\t\t\tbreak;\n"
    code += tab_char + "\t\t}\n"
    code += tab_char + "\t\tvec_node = xml.next(vec_node, \"vector\");\n"
    code += tab_char + "\t}\n"
    code += tab_char + "}\n\n" 

    code += tab_char + "static void load_vartype_vector(std::string vec_name, std::vector<%s> &var, %s &xml, %s *node)\n" %("Fir::VarType","const Fir::XMLParser","const Fir::XMLParser::Node") 
    code += tab_char + "{\n"
    code += tab_char + "\t%s *vec_node = xml.child(node, \"vector\");\n" %("const Fir::XMLParser::Node")
    code += tab_char + "\twhile (vec_node)\n"
    code += tab_char + "\t{\n"
    code += tab_char + "\t\tif (xml.node_attribute(vec_node, \"var\") == vec_name)\n"
    code += tab_char + "\t\t{\n"
    code += tab_char + "\t\t\t%s *sub_node = xml.child(vec_node, NULL);\n" %("const Fir::XMLParser::Node")
    code += tab_char + "\t\t\twhile (sub_node)\n"
    code += tab_char + "\t\t\t{\n"
    code += tab_char + "\t\t\t\tvar.push_back(xml.node_value(sub_node));\n"
    code += tab_char + "\t\t\t\tsub_node = xml.next(sub_node, NULL);\n"
    code += tab_char + "\t\t\t}\n"
    code += tab_char + "\t\t\tbreak;\n"
    code += tab_char + "\t\t}\n"
    code += tab_char + "\t\tvec_node = xml.next(vec_node, \"vector\");\n"
    code += tab_char + "\t}\n"
    code += tab_char + "}\n\n" 
    return code


#创建公共函数文件
def create_common_define_file():
    code = "#ifndef __XML_COMMON_H__\n"
    code += "#define __XML_COMMON_H__\n"
    code += "#include <map>\n"
    code += "#include <vector>\n"
    code += "#include \"vartype.h\"\n"
    code += "#include \"xmlparser.h\"\n"
    code += "\n"
    code += "namespace %s\n" %("config")
    code += "\t{\n"
    code += create_cpp_vector_func("\t")
    code += "}\n\n"
    code += "#endif\n\n"
    return code


#创建代码结构容器
def parse_file(fileName):
    print "提示 %s 开始解析" %(fileName)
    posIndex = fileName.find('.xml')
    if posIndex == -1 or posIndex == len(fileName)-1:
        return
    name = fileName[:posIndex]
    newFile = XML_DIR_NAME + "/" + fileName 
    xml = read_xml(newFile)
    root = xml.documentElement
    objectMap = {"struct":{},"var":{}}
    parse_xml_node(root,objectMap)
    
    cppName = XML_CPP_DIR_NAME + "/" + name + ".h"
    cpp_fp = file(cppName,"wb")
    code = create_cpp_define_file(name,objectMap)  
    cpp_fp.write(code)
    cpp_fp.close()

    print "提示 %s 生成" %(cppName)

def main():
    reload(sys)
    sys.setdefaultencoding("utf-8")

    if os.path.isdir(XML_CPP_DIR_NAME):
        shutil.rmtree(XML_CPP_DIR_NAME)
    os.mkdir(XML_CPP_DIR_NAME)

    if os.path.isdir(XML_CPP_MANAGER_DIR_NAME):
        shutil.rmtree(XML_CPP_MANAGER_DIR_NAME)
    os.mkdir(XML_CPP_MANAGER_DIR_NAME)
    
    if os.path.isdir(XML_CPP_COMMON_DIR_NAME):
        shutil.rmtree(XML_CPP_COMMON_DIR_NAME)
    os.mkdir(XML_CPP_COMMON_DIR_NAME)



    nowFile = XML_DIR_NAME
    fileArr = os.listdir(nowFile)
    xmlfile = []
    for fileName in fileArr:
        posIndex = fileName.find('.xml')
        if posIndex == -1 or posIndex == len(fileName)-1 or posIndex != len(fileName)-4:
            continue
        parse_file(fileName)
        xmlfile.append(fileName[:posIndex])
    
    cppName = XML_CPP_COMMON_DIR_NAME + "/" + "xmlcommon.h"
    cpp_fp = file(cppName,"wb")
    code = create_common_define_file()  
    cpp_fp.write(code)
    cpp_fp.close()
    

    cppName = XML_CPP_MANAGER_DIR_NAME + "/" + "xmlconfig.h"
    cpp_fp = file(cppName,"wb")
    code = create_manager_define_file(xmlfile)  
    cpp_fp.write(code)
    cpp_fp.close()
    
    cppName = XML_CPP_MANAGER_DIR_NAME + "/" + "xmlconfig.cpp"
    cpp_fp = file(cppName,"wb")
    code = create_manager_cpp_file(xmlfile)  
    cpp_fp.write(code)
    cpp_fp.close()

if __name__ == "__main__":
    main()
    
        
