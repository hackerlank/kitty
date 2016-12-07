#!/usr/bin/python
# -*- coding: utf-8 -*-

import xlrd
import xdrlib,sys,os,shutil,platform


#excel字段类型
STRING_TYPE = "string"
INT_TYPE = "int"
FLOAT_TYPE = "float"

#字段使用标识
CLIENT_SERVER_NOT_USERD = "N"
CLIENT_ONLY_UESED = "C"
SERVER_ONLY_USERD = "S"
CLIENR_SERVER_USERD = "A"

#非数据行
CHINA_NAME_COL = 0     #字段中文名
ENGLISH_NAME_COL = 3   #字段英文名
DATA_TYPE = 1          #字段数据类型
USE_FLAG = 2           #字段使用范围
KEY_COL   = 3          #键值
RANGE_COL = 5          #字段值范围
MAX_UN_DATA_NUM_COL = 6

#表类型
SHEET_TYPE_IGNORE = 1
SHEET_TYPE_ONLY_CHECK = 2
SHEET_TYPE_CHECK_AND_WRITE = 3

#excel文件夹
EXCEL_DIR_NAME = "Excel"
#客户端生成的xml目录
CLIENT_XML_DIR_NAME = "clientXml"
#服务器生成的xml目录
SERVER_XML_DIR_NAME = "serverproto"
#protobuf python目录
SERVER_PYTHON_DIR_NAME = "pythonproto"
#protobuf cpp目录
SERVER_CPP_DIR_NAME = "cppproto"
#managerCpp目录
MANAGER_CPP_DIR_NAME = "managercpp"
#生成的tbx目录
SERVER_TBX_DIR_NAME = "tbx"
#生成读取tbx的xml文件
READ_TBX_XML_DIR_NAME = "readtbxxml"
#客户端生成的txt目录
CLIENT_TXT_DIR_NAME = "clientTxt"

#数据类型集
VALUE_TYPE_MAP = {STRING_TYPE:"字符串类型",INT_TYPE:"int型类型",FLOAT_TYPE:"浮点型数据"}

#非法字符集
EALLY_CHAR_MAP = {"*":"*", " ":" ", "?":"?"}

#第一次写cpp文件
WRITE_FIRST_CPP_FLAG = True 

#打开表格
def open_excel(file):
    try:
        data = xlrd.open_workbook(file)
        return data
    except Exception,e:
        print "错误: 打开 %s 表格失败" %(file)

#检查每个表是否正确
def check_sheet(data,keyMap,classArr):
    if data is None:    
        return False    
    table = data.sheets()
    for sheet in table:
        result = []
        if not check_analysis_sheet_name(sheet.name,result):
            continue
        try:
            type = int(result[1])
        except Exception,e:
            print "错误: %s 表类型 %d 错误" %(sheet.name,type)
            continue 
        if type != SHEET_TYPE_IGNORE:
            try:
                if not check_english_name(result[2]):
                    print "错误: %s 表对应的英文名错误" %(sheet.name)
                    return False
            except Exception,e:
                print "错误: %s 表名字对应的英文名解析异常" %(sheet.name)
                return False
            if keyMap.has_key(result[2]):
                print "错误: %s 表名字 %s 重复 " %(sheet.name,result[2])
                return False
            sheetKeyArr = []
            if not check_sheet_all_cols(sheet,sheetKeyArr):
                return False
            else:
                keyMap[result[2]] = sheetKeyArr
            classArr.append(result[2])
    return True

        

#判断字符名字是否含有中文以及是否合法
def check_english_name(name):
    if not len(name):
        return False
    for char in name:
        try:
            if u'\u4e00' <= char <= u'\u9fff':
                return False
            elif EALLY_CHAR_MAP.has_key(char):
                return False
        except:
            return False
    return True

#检查表名格式
def check_analysis_sheet_name(name,result):
    sheetName = name
    tag = '_'

    for checkTime in range(1,3):
        posIndex = sheetName.find(tag)
        if posIndex == -1:
            print "错误: %s 名字的有问题" %(name)
            return False
        result.append(sheetName[:posIndex])
        sheetName = sheetName[posIndex+1:]
    result.append(sheetName)
    return True

#获得谋列的数据值得范围(目前只针对整形)
def get_sheet_col_range(rangeStr,dataType,rangeArr):
    try:
        if not len(rangeStr):
            return False
    except:
        return False

    range = rangeStr
    tag = '-'
    posIndex = range.find(tag)
    if posIndex == -1 or (posIndex == len(range)-1) or (posIndex == 0):
        return False
    if dataType == INT_TYPE:
        try:
            rangeArr[0] = int(range[:posIndex])
            rangeArr[1] = int(range[posIndex+1:])
        except Exception,e:
            return False
    elif dataType == FLOAT_TYPE:
        try:
            rangeArr[0] = float(range[:posIndex])
            rangeArr[1] = float(range[posIndex+1:])
        except Exception,e:
            return False
    return True


#检查每一列的数据类型和范围是否正确
def check_sheet_col_type(sheet,col,sheetKeyArr):
    if col >= sheet.ncols:
        print "错误: %s 访问列数越界 %d" %(sheet.name,col)
        return False

    colArry = sheet.col_values(col)
    if len(colArry) <= MAX_UN_DATA_NUM_COL:
        print "错误: %s 第 %d 列 的行数错误" %(sheet.name,col)
        return False
    if not check_english_name(colArry[ENGLISH_NAME_COL]):
        print "错误: %s 第 %d 列 的英文名字 %s 错误" %(sheet.name,col,colArry[ENGLISH_NAME_COL])
        return False

    dataType = colArry[DATA_TYPE]
    if not VALUE_TYPE_MAP.has_key(dataType):
        print "错误: %s 第 %d 列 的数据类型 %s 非法" %(sheet.name,col,dataType)
        return False
    
    rangeArr = [0,0]
    checkFlag = False

    if dataType != STRING_TYPE:
        checkFlag = True
    if checkFlag and not get_sheet_col_range(colArry[RANGE_COL],dataType,rangeArr):
        print "错误: %s 第 %d 列 的数据范围区域非法" %(sheet.name,col)
        return False

    for index in range(MAX_UN_DATA_NUM_COL,len(colArry)):
        if str(colArry[index]) != "" and checkFlag:
            if dataType == INT_TYPE:
                try:
                    if (int(colArry[index]) < rangeArr[0]) or (int(colArry[index]) > rangeArr[1]):
                        print "错误: %s 第 %d 列 第 %d 行 的数据值 %s 超出范围 %d--%d 字段名为 %s" %(sheet.name,col,index,str(colArry[index]),rangeArr[0],rangeArr[1],colArry[ENGLISH_NAME_COL])
                        return False
                except Exception,e:
                    print "错误: %s 第 %d 列 第 %d 行 的数据值类型不对" %(sheet.name,col,index)
                    return False
            elif dataType == FLOAT_TYPE:
                try:
                    if (float(colArry[index]) < rangeArr[0]) or (float(colArry[index]) > rangeArr[1]):
                        print "错误: %s 第 %d 列 第 %d 行 的数据值 %d 超出范围" %(sheet.name,col,index,int(colArry[index]))
                        return False
                except Exception,e:
                    print "错误: %s 第 %d 列 第 %d 行 的数据值类型不对" %(sheet.name,col,index)
                    return False
            else:
                print "错误: %s 第 %d 列 的数据类型不对:" %(sheet.name,col)
                return False
    #[字段名,值类型,解析范围表示]
    cellArr = [colArry[ENGLISH_NAME_COL],dataType,colArry[USE_FLAG]]
    sheetKeyArr.append(cellArr)
    return True


#检查所有列的数据类型和范围是否正确 
def check_sheet_all_cols(sheet,sheetKeyArr):
    rowNum = sheet.nrows  
    colNum = sheet.ncols
    if not (rowNum and colNum):
        print "错误: %s 的行或者列为空" %(sheet.name)
        return False
    for col in range(0,colNum):
        if not check_sheet_col_type(sheet,col,sheetKeyArr):
            return False
    return True


#生成xml的声明部分
def write_xml_declaration():
    xmlCode = "<?xml version='1.0' encoding='UTF-8'?>\n"
    return xmlCode


#生成xmlsheet节点中的每一个元素
def write_xml_row(sheet,xmlCode,keyArr,clientFlg=True):
    result =[False,xmlCode]
    if sheet.nrows <= MAX_UN_DATA_NUM_COL:
        print  "错误: %s 表中无任何数据" %(sheet.name)
        return result
    for colIndex in range(MAX_UN_DATA_NUM_COL,sheet.nrows):
        colArr = sheet.row_values(colIndex)
        xmlCode += "\t\t"
        xmlCode += "<rowNode>"
        xmlCode += "\n"
        for index in range(0,len(colArr)):
            rowNameTypeArr = keyArr[index]
            if len(rowNameTypeArr) != 3:
                print "错误: %s 字段名和类型非法:" %(sheet.name)
                return result
            keyName = rowNameTypeArr[0]
            keyValue = colArr[index]
            useFlg = rowNameTypeArr[2]
            if clientFlg:
                if useFlg == CLIENT_SERVER_NOT_USERD or useFlg == SERVER_ONLY_USERD:
                    continue
            else:
                if useFlg == CLIENT_SERVER_NOT_USERD or userFlg == CLIENT_ONLY_UESED:
                    continue
            if rowNameTypeArr[1] == STRING_TYPE:
                keyValue = str(keyValue)
            elif rowNameTypeArr[1] == INT_TYPE:
                #当整数不填的话，默认值为0
                if str(colArr[index]) == "":
                    keyValue = 0
                else:
                    keyValue = int(keyValue)
            elif rowNameTypeArr[1] == FLOAT_TYPE:
                #当浮点不填的话，默认值为0
                if str(colArr[index]) == "":
                    keyValue = 0.0
                else:
                    keyValue = float(keyValue)
            else:
                print "错误: %s 字段名 %s 的类型 %d 非法" %(sheet.name,keyName,rowNameTypeArr[1])
                return result
            xmlCode += "\t\t\t"
            xmlCode += "<%s>" %(keyName)
            xmlCode += str(keyValue)
            xmlCode += "</%s>" %(keyName)
            xmlCode += "\n"
        xmlCode += "\t\t"
        xmlCode += "</rowNode>"
        xmlCode += "\n"
    result[0] = True
    result[1] = xmlCode
    return result
     



#生成xml中的sheet部分
def write_xml_sheet_node(sheet,xmlCode,keyMap):
    resultCode = [False,xmlCode]
    result = []
    if not check_analysis_sheet_name(sheet.name,result):
        return False
    try:
        type = int(result[1])
    except Exception,e:
        print "错误: %s 表类型错误" %(sheet.name,type)
        return resultCode 
    if type == SHEET_TYPE_CHECK_AND_WRITE:
        try:
            if not check_english_name(result[2]):
                print "错误: %s 表对应的英文名错误" %(sheet.name)
                return resultCode 
        except Exception,e:
            print "错误: %s 表名字对应的英文名解析异常" %(sheet.name)
            return resultCode 
        if not keyMap.has_key(result[2]):
            print "错误: %s 表没有对应的字段" %(sheet.name)
            return resultCode 
        keySheetName = result[2]
        
        xmlCode += "\t"
        xmlCode += "<%s>" %(keySheetName)
        xmlCode += "\n"

        result_Code =  write_xml_row(sheet,xmlCode,keyMap[result[2]],True)
        if result_Code[0]:
            xmlCode = result_Code[1]
       
        xmlCode += "\t"
        xmlCode += "</%s>" %(keySheetName)
        xmlCode += "\n"

        resultCode[0] = True
        resultCode[1] = xmlCode
    return resultCode


#生成xml的剩余部分
def write_xml_root(data,rootName,keyMap):
    xmlCode = "\n"
    xmlCode += "<%s>\n" %(rootName)
    
    table = data.sheets();
    for sheet in table:
        result = write_xml_sheet_node(sheet,xmlCode,keyMap)
        if result[0]:
            xmlCode = result[1]

    xmlCode += "</%s>" %(rootName)
    return xmlCode


#生成protosheet节点中的每一个元素
def write_proto_col(sheet,protoCode,keyArr):
    result =[False,protoCode]
    protoCode += "\t\trequired uint32 tbxid = 1;"
    dataIndex = 2
    for rowIndex in range(0,sheet.ncols):
        rowArr = sheet.col_values(rowIndex)
        if len(rowArr) <= MAX_UN_DATA_NUM_COL:
            print "错误: %s 第 %d 列 格式错误" %(sheet.name,rowIndex)
            return result
        if rowArr[USE_FLAG] == SERVER_ONLY_USERD or rowArr[USE_FLAG] == CLIENR_SERVER_USERD:
            protoCode += "\n\t\t"
            protoCode += "required"
            dataType = rowArr[DATA_TYPE]
            if dataType == STRING_TYPE:
                protoCode += " string"
            elif dataType == INT_TYPE:
                protoCode += " uint32"
            elif dataType == FLOAT_TYPE:
                protoCode += " double"
            else:
                print "错误: %s 第 %d 列 数据类型错误" %(sheet.name,rowIndex)
                return result
            protoCode += " %s = %d;" %(rowArr[ENGLISH_NAME_COL],dataIndex)
            dataIndex += 1;
    result[0] = True
    result[1] = protoCode
    return result



           
 
#生成proto中的sheet部分
def write_proto_sheet_node(sheet,protoCode,keyMap):
    resultCode = [False,protoCode]
    result = []
    if not check_analysis_sheet_name(sheet.name,result):
        return False
    try:
        type = int(result[1])
    except Exception,e:
        print "错误: %s 表类型 %d 错误" %(sheet.name,type)
        return resultCode 
    if type == SHEET_TYPE_CHECK_AND_WRITE:
        try:
            if not check_english_name(result[2]):
                print "错误: %s 表对应的英文名错误" %(sheet.name)
                return resultCode 
        except Exception,e:
            print "错误: %s 表名字对应的英文名解析异常" %(sheet.name)
            return resultCode 
        if not keyMap.has_key(result[2]):
            print "错误: %s 表没有对应的字段" %(sheet.name)
            return resultCode 
        keySheetName = result[2]
        
        protoCode += "message %s" %(keySheetName)
        protoCode += "\n{\n"
        protoCode += "\t"
        protoCode += "message t_%s" %(keySheetName)

        protoCode += "\n\t{\n"
        result = write_proto_col(sheet,protoCode,keyMap)
        if(result[0]):
            protoCode = result[1]
        protoCode += "\n\t}\n"
        
        protoCode += "\trepeated t_%s datas = 1;\n" %(keySheetName)
        protoCode += "}"

        resultCode[0] = True
        resultCode[1] = protoCode 
    return resultCode



#生成proto的剩余部分
def write_proto(data,keyMap):
    table = data.sheets();
    protoCode = "package pb;\n\n"
    for sheet in table:
        result = write_proto_sheet_node(sheet,protoCode,keyMap)
        if result[0]:
            protoCode = result[1]
            protoCode += "\n"
    return protoCode


#import 模块
def import_mod(modname, path):
    if sys.modules.has_key(modname): del sys.modules[modname]
    sys.path.insert(0, path)
    mod = __import__(modname)
    del sys.path[0]
    return mod

#创建配置管理文件头文件
def create_cpp_head():
    cppCode = "#ifndef __TBX_H__\n"
    cppCode += "#define __TBX_H__\n"
    
    nowFile = SERVER_CPP_DIR_NAME  
    fileArr = os.listdir(nowFile)
    for file in fileArr:
        posIndex = file.find('.h')
        if posIndex == -1 or posIndex == len(file)-1:
            continue
        cppCode += "#include \"%s\"\n" %(file)
    
    cppCode += "#include \"tbxbase.h\"\n"
    cppCode += "\nnamespace tbx \n{\n"
    return cppCode

#创建配置管理文件实现文件
def create_cpp_headcpp():
    cppCode = "#include \"tbx.h\"\n\n"
    cppCode += "namespace tbx \n{\n"
    return cppCode


#创建配置管理文件头文件的尾部
def create_cpp_head_tail():
    cppCode = "}\n\n"
    cppCode += "#endif"
    return cppCode

#创建配置管理文件实现文件的尾部
def create_cpp_cpp_tail():
    cppCode = "}\n"
    return cppCode


#创建配置管理文件的实现文件
def create_cpp_cpp_function(classArr):
    ret = [False]
    cppCode = "" 
    for key in classArr:
        cppCode += "\ttbx::table<pb::%s,pb::Conf_t_%s,pb::%s::t_%s> _%s;\n" %(key,key,key,key,key)
    
    cppCode += "\n"
    cppCode += "\tvoid mgr::init(Fir::XMLParser &xml, const tbx::ReadTbxBufFunc &func)\n"
    cppCode += "\t{\n"
    cppCode += "\t\tloadconfig(xml);\n"
    for key in classArr:
        cppCode += "\t\tload_table(\"pb.%s\",_%s,func);\n" %(key,key) 
    cppCode += "\t}\n"
    
    cppCode += "\n"
    for key in classArr:
        cppCode += "\tconst tbx::table< pb::%s,pb::Conf_t_%s,pb::%s::t_%s >& %s()\n" %(key,key,key,key,key)
        cppCode += "\t{\n"
        cppCode += "\t\t return _%s;\n\t}\n" %(key)
   

    cppCode += "\n" 
    for key in classArr: 
        cppCode += "\tconst pb::Conf_t_%s& %s(unsigned int id)\n" %(key,key)
        cppCode += "\t{\n"
        cppCode += "\t\t return _%s.get(id);\n\t}\n" %(key)
    
    ret[0] = True
    ret.append(cppCode)
    return ret

def create_cpp_head_function(classArr):
    ret = [False]
    cppCode = "" 
    for key in classArr:
        cppCode += "\tconst tbx::table< pb::%s,pb::Conf_t_%s,pb::%s::t_%s >& %s();\n" %(key,key,key,key,key)
    
    cppCode += "\n"
    for key in classArr: 
        cppCode += "\tconst pb::Conf_t_%s& %s(unsigned int id);\n" %(key,key)
    
    ret[0] = True
    ret.append(cppCode)
    return ret

#把生成的cc文件转换成cpp文件
def mv_cc_to_cpp():
    nowFile = SERVER_CPP_DIR_NAME
    fileArr = os.listdir(nowFile)
    for file in fileArr:
        posIndex = file.find('.cc')
        if posIndex == -1 or posIndex == len(file)-1:
            continue
        extendName = file[0:posIndex]
        mvstr = "mv %s/%s %s/%s.cpp" %(SERVER_CPP_DIR_NAME,file,SERVER_CPP_DIR_NAME,extendName)
        os.system(mvstr)


#生成cpp和python代码
def create_code():
    protoName = "./" + SERVER_XML_DIR_NAME + "/*.proto"
    os.system("protoc -I={0} --python_out={1} --cpp_out={2} {3}".format(SERVER_XML_DIR_NAME,SERVER_PYTHON_DIR_NAME,SERVER_CPP_DIR_NAME,protoName))


#检查生成tbx二进制文件
def check_tbx_file(sheet):
    result = []
    ret = [False]
    if not check_analysis_sheet_name(sheet.name,result):
        return ret
    ret.append(result[2])
    try:
        type = int(result[1])
    except Exception,e:
        print "错误: %s 表类型 %d 错误" %(sheet.name,type)
        return ret
    if type == SHEET_TYPE_IGNORE or type == SHEET_TYPE_ONLY_CHECK:
        return ret
    try:
        if not check_english_name(result[2]):
            print "错误: %s 表对应的英文名错误" %(sheet.name)
            return ret 
    except Exception,e:
        print "错误: %s 表名字对应的英文名解析异常" %(sheet.name)
        return ret
    ret[0] = True
    return ret
  
#生成tbx节点中的每一个元素
def write_tbx_row_obj(sheet,obj,keyArr,row):
    colArr = sheet.row_values(row)
    key = colArr[0]
    try:
        exec "obj.{0} = int(key)".format("tbxid")
    except:
        print "tbxid error key:%s" %(key)
    for index in range(0,len(colArr)):
        rowNameTypeArr = keyArr[index]
        if len(rowNameTypeArr) != 3:
            print "错误: %s 字段名和类型非法:" %(sheet.name)
            continue
        keyName = rowNameTypeArr[0]
        keyValue = colArr[index]
        try:
            useFlg = rowNameTypeArr[2]
        except:
            print "useFlg error :%s" %(rowNameTypeArr[2])
        
        if useFlg == CLIENT_SERVER_NOT_USERD or useFlg == CLIENT_ONLY_UESED:
            continue
        if rowNameTypeArr[1] == STRING_TYPE:
            temp = str(keyValue).decode("UTF-8")
            try:
                exec "obj.{0} = temp".format(keyName)
            except:
                print "string key:%s,temp:%s" %(keyName,temp)
        elif rowNameTypeArr[1] == INT_TYPE:
            if keyValue == "":
                keyValue = "0"
            try:
                exec "obj.{0} = int(keyValue)".format(keyName)
            except:
                print "int key:%s,value:%s" %(keyName,keyValue)
        elif rowNameTypeArr[1] == FLOAT_TYPE:
            if keyValue == "":
                keyValue = "0"
            try:
                exec "obj.{0} = float(keyValue)".format(keyName)
            except:
                print "double key:%s,value:%s" %(keyName,keyValue)
        else:
            print "错误: %s 字段名 %s 的类型 %d 非法" %(sheet.name,keyName,rowNameTypeArr[1])

#生成sheet对应的tbx
def create_sheet_tbx_file(sheet,mod,keyMap):
    ret = check_tbx_file(sheet)
    if not ret[0]:
        return
    if not keyMap.has_key(ret[1]):
        print "错误: %s 生成 %s tbx失败" %(sheet.name,ret[1])
        return
    keyArr = keyMap[ret[1]]
    pb = eval("mod." + ret[1] + "()")
    for row in range(MAX_UN_DATA_NUM_COL,sheet.nrows):
        try:
            obj = pb.datas.add()
            write_tbx_row_obj(sheet,obj,keyArr,row)
        except:
            print "错误: %s 第 %d 行出现错误" %(sheet.name,row)

    tbxFile = SERVER_TBX_DIR_NAME + "/" + ret[1] + ".tbx" 
    op = file(tbxFile,"wb")
    op.write(pb.SerializeToString())
    op.close()
    print "提示: 创建 %s 文件成功" %(tbxFile)
 
#生成book中的所有sheet的tbx
def create_book_tbx_file(fileName,name,keyMap):
    pyFileName = name + "_pb2.py"
    nowFile = SERVER_PYTHON_DIR_NAME
    fileArr = os.listdir(nowFile)
    if not pyFileName in fileArr:
        print "错误: %s 文件没有找到" %(pyFileName)
        return
    
    data = open_excel(fileName)
    mod = import_mod(name +"_pb2",SERVER_PYTHON_DIR_NAME)
    table = data.sheets()
    for sheet in table:
        create_sheet_tbx_file(sheet,mod,keyMap)

#根据tbx文件生成tbx对应的xml文件
def write_tbx_xml_node():
    xmlCode = "<tbx>\n"
    nowFile = SERVER_TBX_DIR_NAME 
    fileArr = os.listdir(nowFile)
    for file in fileArr:
        posIndex = file.find('.tbx')
        if posIndex == -1 or posIndex == len(file)-1:
            continue
        extendName = file[:posIndex]
        xmlCode += "\t<file name=\"pb.%s\" path=\"%s\"/>\n" %(extendName,file)
    xmlCode += "</tbx>\n"
    return xmlCode

#生成txt的code
def write_txt_code(sheet,keyArr):
    ret = [False]
    txtCode = ""
    for colIndex in range(0,sheet.nrows):
        #第二行不要
        if colIndex == USE_FLAG:
            continue
        colArr = sheet.row_values(colIndex)
        tempCode = ""
        for index in range(0,len(colArr)):
            #前5行直接转化
            if colIndex < MAX_UN_DATA_NUM_COL:
                if tempCode == "":
                    tempCode = str(colArr[index])
                else:
                    tempCode += "\t" + str(colArr[index])
            else:
                try:
                    rowNameTypeArr = keyArr[index]
                except Exception:
                    print "错误: %s 表的第 %d 行 第 %d 列 生成txt有问题" %(sheet.name,colIndex,index)
                    return ret
                if len(rowNameTypeArr) != 3:
                    print "错误 %s 字段名和类型非法:" %(sheet.name)
                    return ret
                keyName = rowNameTypeArr[0]
                keyValue = colArr[index]
                useFlg = rowNameTypeArr[2]
                if useFlg == CLIENT_SERVER_NOT_USERD or useFlg == SERVER_ONLY_USERD:
                    continue
                if rowNameTypeArr[1] == STRING_TYPE:
                    tempMap = sheet.cell(colIndex,index)
                    if tempMap.ctype == 2:
                        keyValue = int(tempMap.value)
                    else:
                        keyValue = str(keyValue)
                elif rowNameTypeArr[1] == INT_TYPE:
                    #当整数不填的话，默认值为0
                    if str(colArr[index]) == "":
                        keyValue = 0
                    else:
                        keyValue = int(keyValue)
                elif rowNameTypeArr[1] == FLOAT_TYPE:
                    #当浮点不填的话，默认值为0
                    if str(colArr[index]) == "":
                        keyValue = 0.0
                    else:
                        keyValue = float(keyValue)
                else:
                    print "错误: %s 字段名 %s 的类型 %d 非法" %(sheet.name,keyName,rowNameTypeArr[1])
                    return ret 
                if tempCode == "":
                    tempCode = str(keyValue)
                else:
                    tempCode += "\t" + str(keyValue)
        txtCode += tempCode;
        txtCode += "\r\n"
    ret[0] = True
    ret.append(txtCode)
    return ret 
     
#生成txt文件
def write_txt_sheet(sheet,keyArr):
    result = []
    if not check_analysis_sheet_name(sheet.name,result):
        return
    
    txtName = CLIENT_TXT_DIR_NAME + "/" + result[2] + ".txt"
    if not keyArr.has_key(result[2]):
        print "错误: %s 表没有对应的字段" %(sheet.name)
        return 
    
    ret = write_txt_code(sheet,keyArr[result[2]])
    if not ret[0]:
        print "错误: %s 文件生成失败:%s" %(sheet.name,txtName)
        return
        
    txt_fp = file(txtName,"wb")
    txt_fp.write(ret[1])
    txt_fp.close()
    print "提示: %s 文件生 %s 成功" %(sheet.name,txtName)

#解析excel表
def parseExcel(fileName,name,classArr):
    print "提示: %s 开始被解析" %(fileName)
    if not check_english_name(name):
        print "错误: %s 文件名对应英文非法" %(fileName)
        return False
    
    #检测excel格式数据是否正确
    keyMap = {}
    data = open_excel(fileName)
    if not check_sheet(data,keyMap,classArr):
        return False
    if is_win_system():
        return False
    elif is_linux_system():
        #生成proto文件
        protoName = SERVER_XML_DIR_NAME + "/" + name + ".proto"
        proto_fp = file(protoName,"wb")
        proto_fp.write(write_proto(data,keyMap)) 
        proto_fp.close()
   
        #生成对应的cpp和python文件
        create_code()
    
        #将生成的cc文件转换成cpp文件
        mv_cc_to_cpp()
    
        #生成tbx二进制文件
        create_book_tbx_file(fileName,name,keyMap)
   
        #生成读取tbx的xml文件
        tbxxml = READ_TBX_XML_DIR_NAME + "/" + "tbx" + ".xml" 
        tbx_fp = file(tbxxml,"wb")
        tbx_fp.write(write_xml_declaration())
        tbx_fp.write(write_tbx_xml_node())
        tbx_fp.close()

    print "提示: %s 被解析完毕" %(fileName)
    return True


#生成对应的配置管理头文件
def create_cpp_head_file(classArr):
    cppName = MANAGER_CPP_DIR_NAME + "/" + "tbx" + ".h"
    cpp_fp = file(cppName,"wb")
    cpp_fp.write(create_cpp_head())
    ret = create_cpp_head_function(classArr)
    if ret[0]:
        cpp_fp.write(ret[1])
    cpp_fp.write(create_cpp_head_tail())
    cpp_fp.close()
    print "提示: %s 生成" %(cppName)

#生成对应的配置管理实现文件
def create_cpp_cpp_file(classArr):
    cppName = MANAGER_CPP_DIR_NAME + "/" + "tbx" + ".cpp"
    cpp_fp = file(cppName,"wb")
    cpp_fp.write(create_cpp_headcpp())
    ret = create_cpp_cpp_function(classArr)
    if ret[0]:
        cpp_fp.write(ret[1])
    cpp_fp.write(create_cpp_cpp_tail())
    print "提示: %s 生成" %(cppName)

#创建文件夹
def create_dir(name):
    if os.path.isdir(name):
        shutil.rmtree(name)
    os.mkdir(name)

#删除文件夹
def delete_empty_dir(name):
    empty = True
    nowFile = name 
    fileArr = os.listdir(nowFile)
    for file in fileArr:
        posIndex = file.find('.')
        if posIndex == -1 or posIndex == len(file)-1:
            continue
        empty = False

    if empty:
        shutil.rmtree(name)

#判断是否为win操作系统
def is_win_system():
    return 'Windows' in platform.system()

#判断是否为linux操作系统
def is_linux_system():
    return 'Linux' in platform.system()

#主函数
def main():
    reload(sys)  
    sys.setdefaultencoding('utf8')
   
    if is_win_system():
        return "警告:此工具只支持服务器"
    elif is_linux_system():
        create_dir(SERVER_XML_DIR_NAME)
        create_dir(SERVER_PYTHON_DIR_NAME)
        create_dir(SERVER_CPP_DIR_NAME)
        create_dir(MANAGER_CPP_DIR_NAME)
        create_dir(SERVER_TBX_DIR_NAME)
        create_dir(READ_TBX_XML_DIR_NAME)
    else:
        print "错误:操作系统有问题"
        return

    classArr = []
    nowFile = EXCEL_DIR_NAME
    fileArr = os.listdir(nowFile)
    for file in fileArr:
        posIndex = file.find('.')
        if posIndex == -1 or posIndex == len(file)-1:
            continue
        extendName = file[posIndex+1:]
        fileName = file[:posIndex]
        if extendName == "xlsx":
            dirFile = EXCEL_DIR_NAME + "/" + file
            parseExcel(dirFile,fileName,classArr)

    create_cpp_head_file(classArr)
    create_cpp_cpp_file(classArr)
    

if __name__=="__main__":
    main()
