#!/usr/bin/python
#_*_coding:utf-8_*_

from xml.dom import minidom
import os,time,sys
##############################
#########新手慎用#############
##############################
databases = []
tempsql = os.getcwd() + os.sep + "temp.sql"

def readConfig(file):
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

def parseConfig():
    info = ["mysql"]
    dataFlInfo = ["mysql"]
    xml = readConfig("sqlconfig.xml")
    root = xml.documentElement
    for node in root.childNodes:
        if node.nodeType == node.ELEMENT_NODE:
            for name,val in node.attributes.items(): 
                if name == "user":
                    temp = "-u" + val
                    info.append(temp);
                    dataFlInfo.append(temp)
                elif name == "passwd":
                    temp = "-p" + val
                    info.append(temp)
                    dataFlInfo.append(temp)
                elif name == "ip":
                    temp = "-h" + val
                    info.append(temp)
                    dataFlInfo.append(temp)
                elif name == "data":
                    info.append(val)
                elif name == "flserver":
                    dataFlInfo.append(val)
                else:
                    continue
    databases.append(info)
    databases.append(dataFlInfo)

def mktempsql():
	if not os.path.exists(tempsql):
		os.mknod(tempsql)

def excSql(index):
    if index >= len(databases):
        return
    database = databases[index]
    sql = " ".join(database) + " < " + tempsql
    os.system(sql)

def copydata(text,updateFlg,index):
    mktempsql()
    curdir = os.getcwd()
    text = curdir + os.sep + text
    if not os.path.exists(text):
        os.mknod(text)
    try:
        file_object = open(text)
        file_tmp = open(tempsql,"w")
        templine = []
        for line in file_object:
            templine.append(line)
        arry = []
        if updateFlg:
            templine.reverse()
        for line in templine:
            if(not line.startswith("--")) and (not len(line) <= 0) and (not line.startswith("#")):
                arry.append(line)
            else:
                if updateFlg:
                    break;
                else:
                    continue
        if updateFlg:
            arry.reverse()
        for line in arry:
            if (not line.startswith("--")) and (not len(line) <= 0) and (not line.startswith("#")):
                try:
                    file_tmp.write(line)
                except:
                    print "写入数据失败"
            else:
                break 
    except:
        pass
    finally:
        file_object.close()
        file_tmp.close()
        excSql(index)
        os.remove(tempsql)
    
def update():
	try:
		copydata("update.sql",True,0)
	except:
		print "执行数据库失败"

def install():
	try:
		copydata("install.sql",False,0)
		copydata("flserver.sql",False,1)
	except:
		print "安装数据库失败"

def clear():
	try:
		install()
	except:
		print "清空数据库失败"

def main():
	reload(sys)
	sys.setdefaultencoding("utf-8")
	parseConfig()
	if len(sys.argv) < 2:
		usage()
		sys.exit()
	if sys.argv[1] == "update":
		update()
	elif sys.argv[1] == "install":
		install()
	elif sys.argv[1] == "clear":
		clear()
	else:
		print "非法的操作"

def usage():
	print "usage:\n\tupdate\t修改数据库表 当前目录需要update.sql文件\n\tinstall\t安装数据库 当前目录需要install.sql文件"

if __name__ == "__main__":
	main()

