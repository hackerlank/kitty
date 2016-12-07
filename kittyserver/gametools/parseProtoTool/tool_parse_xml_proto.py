#!/usr/bin/python
#_*_coding:utf-8_*_

from xml.dom import minidom
import xlrd, os, sys, re, getopt,shutil

#生成的protobuf的文件夹
PROTO_DIR_NAME = "protodir"
#xml消息的所在文件夹
PROTO_MSG_XML_DIR_NAME = "msgxml"

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

#解析节点的属性
def parse_xml_node_attr(file,node,protoCode):
    attrMap = {}
    order = 1
    result = [False]
    for name,value in node.attributes.items():
        if attrMap.has_key(name):
            print "警告: %s 文件 %s 节点 %s 属性重名" %(file,node.nodeName,name)
            continue
        attrMap[name] = value
    if not attrMap.has_key("name"):
        print "警告: %s 文件 %s 节点没有name属性" %(file,node.nodeName)
        return result
    if node.nodeName == "proto":
        protoCode += "\nmessage %s\n" %(attrMap["name"])
        protoCode += "{\n"
    else:
        protoCode += "\n%s %s \n" %(node.nodeName,attrMap["name"])
        protoCode += "{\n"
    
    for sub_node in node.childNodes:
        if sub_node.nodeName == "#text":
            continue
        if sub_node.nodeName != "member":
            print "警告: %s 文件 %s 节点的子节点名 %s 不是member" %(file,node.nodeName,sub_node.nodeName)
            continue
        sub_attrMap = {}
        for sub_name,sub_value in sub_node.attributes.items():
            if sub_attrMap.has_key(sub_name):
                print "警告: %s 文件 %s 节点 %s 属性重名" %(file,sub_node.nodeName,sub_name)
                continue
            sub_attrMap[sub_name] = sub_value
        if sub_attrMap.has_key("default") and len(sub_attrMap["default"]) == 0:
            sub_attrMap["default"] = "\"\""
        if node.nodeName != "enum":
            if not sub_attrMap.has_key("name") or not sub_attrMap.has_key("type"):
                print "警告: %s 文件 %s 节点 %s 子节点 没有name或者type属性名" %(file,node.nodeName,sub_node.nodeName)
                continue
            if node.nodeName == "proto":
                if not sub_attrMap.has_key("prefix"):
                    print "警告: %s 文件 %s 节点 %s 子节点 message 没有prefix属性名" %(file,node.nodeName,sub_node.nodeName)
                    continue
            #转换类型
            valueType = sub_attrMap["type"]
            if valueType == "uint":
                sub_attrMap["type"] = "uint32"
            elif valueType == "ulong":
                sub_attrMap["type"] = "uint64"
            else:
                pass

            if node.nodeName == "proto":
                prefix = sub_attrMap["prefix"]
                if not prefix in ["required","optional","repeated"]:
                    print "警告: %s 文件 %s 节点 %s 子节点 message 没有prefix属性值 %s 错误" %(file,node.nodeName,sub_node.nodeName,prefix)
                    continue
                if not sub_attrMap.has_key("default"):
                    protoCode += "\t %s %s %s = %d;\n" %(sub_attrMap["prefix"],sub_attrMap["type"],sub_attrMap["name"],order)
                else:
                    protoCode += "\t %s %s %s = %d [default = %s];\n" %(sub_attrMap["prefix"],sub_attrMap["type"],sub_attrMap["name"],order,sub_attrMap["default"])
            order += 1
        else:
            if not sub_attrMap.has_key("name") or not sub_attrMap.has_key("value"):
                print "警告: %s 文件 %s 节点 %s 子节点 enum 没有name或者value属性名" %(file,node.nodeName,sub_node.nodeName)
                continue
            protoCode += "\t %s = %d;\n" %(sub_attrMap["name"],int(sub_attrMap["value"]))
    protoCode += "}\n"
    result[0] = True
    result.append(protoCode)
    return result


#解析import属性节点
def parse_xml_import_node(file,node,protoCode):
    for sub_node in node.childNodes:
        if sub_node.nodeName == "#text":
            continue
        if sub_node.nodeName != "member":
            print "警告: %s 文件 %s 节点的子节点名 %s 不是member" %(file,node.nodeName,sub_node.nodeName)
            continue
        for sub_name,sub_value in sub_node.attributes.items():
            if sub_name == "name":
                protoCode += "import \"%s.proto\";\n" %(sub_value)
    return protoCode 



#解析xml节点
def parse_xml_node(file,root):
    result = [False]
    protoCode = ""
    if root.nodeName != "protocol":
        print "错误: %s 文件根节点名字不为protocol" %(file)
        return result
    for node in root.childNodes:
        if node.nodeType != node.ELEMENT_NODE:
            continue
        if node.nodeName == "import":
            protoCode = parse_xml_import_node(file,node,protoCode)
            continue 
        ret = parse_xml_node_attr(file,node,protoCode)
        if ret[0]:
            protoCode = ret[1]
            if not result[0]:
                result[0] = True
    result.append(protoCode)
    return result 


#生成protobuf文件
def parse_xml_to_proto(fileName):
    print "提示: %s 开始解析" %(fileName)
    posIndex = fileName.find('.xml')
    if posIndex == -1 or posIndex == len(fileName)-1:
        return

    name = fileName[:posIndex]
    newFile = PROTO_MSG_XML_DIR_NAME + "/" + fileName 
    xml = read_xml(newFile)
    root = xml.documentElement
    protoName = PROTO_DIR_NAME + "/" + name + ".proto"
    proto_fp = file(protoName,"wb")

    #包裹
    packageCode = "package HelloKittyMsgData;\n"
    proto_fp.write(packageCode)
    
    ret = parse_xml_node(fileName,root)
    if ret[0]:
        proto_fp.write(ret[1])
        print "提示: %s 生成" %(protoName)
    else:
        print "提示:%s 生成失败:" %(protoName)
    proto_fp.close()

def main():
    reload(sys)
    sys.setdefaultencoding("utf-8")
    if os.path.isdir(PROTO_DIR_NAME):
        shutil.rmtree(PROTO_DIR_NAME)
    os.mkdir(PROTO_DIR_NAME)

    nowFile = PROTO_MSG_XML_DIR_NAME
    fileArr = os.listdir(nowFile)
    for file in fileArr:
        posIndex = file.find('.xml')
        if posIndex == -1 or posIndex == len(file)-1 or posIndex != len(file)-4:
            continue
        parse_xml_to_proto(file)


if __name__ == "__main__":
    main()
