BUILD_DIR_ROOT = $(PWD)

SVN_ROOT=svn://10.1.21.101/svnkitty/hellokitty

DESIGN_SVNURL=$(SVN_ROOT)/design/designconfig/excel
DESIGN_XML_SVNURL=$(SVN_ROOT)/design/designconfig/xml

MAKE_TBX_PATH=$(BUILD_DIR_ROOT)/excel
TBX_PATH=$(BUILD_DIR_ROOT)/tbx
TBX_CPP_PATH=$(BUILD_DIR_ROOT)/tbxcpp
XML_PATH=$(BUILD_DIR_ROOT)/config
MAKE_XML_PATH=$(XML_PATH)/xml
XML_CPP_PATH=$(BUILD_DIR_ROOT)/xml
FIRCOMM_PATH=$(BUILD_DIR_ROOT)/common
VERSION:=$(shell date +%y%m%d%H%M)
WARNING_OPT = -g -Wall -Werror -Wno-unused-local-typedefs -Wno-narrowing -Wno-deprecated -fno-strict-aliasing
MACRO_OPT = -D_DEBUG_WCX -D_ZYZ_DEBUG -D_DEBUG_LBS -D_USE_CMD_NAMESPACE  -D_USE_EPOLL_ -D_GNU_SOURCE -D_REENTRANT -D_USE_GCC_4
DIR_OPT = -I../xml -I../base -I../tbxcpp -I$(FIRCOMM_PATH) -I../base/rapidxml -I../base/hiredis -I../base/EncDec -I$(TBX_CPP_PATH)

FIR_DEBUG_OPT = $(WARNING_OPT) $(MACRO_OPT) $(DIR_OPT) -D_ALL_SUPER_GM -D_DEBUG 
FIR_RELEASE_OPT = -O2 $(WARNING_OPT) $(MACRO_OPT) $(DIR_OPT) $(BATTLE_OPTIONS) $(FIR_OP_OPTIONS) -DVS=$(VERSIONSTRING) $(ASAN_OPT)
SUB_DIRS = base/EncDec base/hiredis base/lzma protocpp base tbxcpp xml 
SUB_TARGETS = superserver recordserver gateserver  sceneserver flserver robotserver gmtoolserver resourceserver
export
ReLeaseBin = $(PWD)/Release-$(VERSION)
#ALL_SUB_DIRS = $(SUB_DIRS)
NJOBS = `if [ "" == "$(NCPUS)" ]; then echo ""; else echo "-j$(NCPUS)"; fi`

export CXX = /usr/bin/c++

.PHONY : all debug debug_nogm ctags doc distclean clean sql res svn build release_cp dist_tar dist map distall dist_tar_gm dist_gm map_gm distall_gm  ChangeLog

all: debug
define MKDIR
	@if test ! -d $(1) ; \
	then \
	mkdir -p $(1) ; \
	fi 
endef
define SubMKDIR
if test ! -d $(1) ; then   mkdir -p $(1);  fi
endef

CheckReLeaseBin:
	$(call MKDIR,$(ReLeaseBin))
	$(call MKDIR,$(ReLeaseBin)/superserver)
	$(call MKDIR,$(ReLeaseBin)/recordserver)
	$(call MKDIR,$(ReLeaseBin)/gateserver)
	$(call MKDIR,$(ReLeaseBin)/sceneserver)
	$(call MKDIR,$(ReLeaseBin)/flserver)
	$(call MKDIR,$(ReLeaseBin)/robotserver)
	$(call MKDIR,$(ReLeaseBin)/gmtoolserver)
	$(call MKDIR,$(ReLeaseBin)/gametools)
	$(call MKDIR,$(ReLeaseBin)/gametools/parseXmlTool)
	$(call MKDIR,$(ReLeaseBin)/resourceserver)

res:
	#生成excel资源
	chmod +x gametools/parseExcelTool/tool_parse_excel.py
	@rm -rf tbx/*.*
	@rm -rf tbxcpp/*.*
	
	@(cd gametools/parseExcelTool;./tool_parse_excel.py)
	@if ls gametools/parseExcelTool/readtbxxml/*.xml > /dev/null 2>&1; then cp gametools/parseExcelTool/readtbxxml/* tbx/; fi
	@if ls gametools/parseExcelTool/tbx/*.tbx > /dev/null 2>&1; then cp gametools/parseExcelTool/tbx/* tbx/; fi
	@if ls gametools/parseExcelTool/cppproto/*.h > /dev/null 2>&1; then cp gametools/parseExcelTool/cppproto/* tbxcpp/; fi
	@if ls gametools/parseExcelTool/managercpp/*.h > /dev/null 2>&1; then cp gametools/parseExcelTool/managercpp/* tbxcpp/; fi
		
	@rm -rf gametools/parseExcelTool/readtbxxml
	@rm -rf gametools/parseExcelTool/tbx
	@rm -rf gametools/parseExcelTool/cppproto
	@rm -rf gametools/parseExcelTool/managercpp
	@rm -rf gametools/parseExcelTool/clientXml
	@rm -rf gametools/parseExcelTool/pythonproto
	@rm -rf gametools/parseExcelTool/serverproto
	
	#生成xml资源
	chmod +x gametools/parseXmlTool/tool_parse_xml.py
	@rm -rf xml/*.*
	
	@(cd gametools/parseXmlTool;./tool_parse_xml.py)
	@if ls gametools/parseXmlTool/xmlcpp/*.h > /dev/null 2>&1; then cp gametools/parseXmlTool/xmlcpp/* xml/; fi
	@if ls gametools/parseXmlTool/xmlcommon/*.h > /dev/null 2>&1; then cp gametools/parseXmlTool/xmlcommon/* xml/; fi
	@if ls gametools/parseXmlTool/managerxmlcpp/*.h > /dev/null 2>&1; then cp gametools/parseXmlTool/managerxmlcpp/* xml/; fi
	
	@rm -rf gametools/parseXmlTool/xmlcpp
	@rm -rf gametools/parseXmlTool/xmlcommon
	@rm -rf gametools/parseXmlTool/managerxmlcpp
	
	#生成proto消息资源
	chmod +x gametools/parseProtoTool/tool_parse_xml_proto.py
	@rm -rf protomsg/*.*
	
	@(cd gametools/parseProtoTool;./tool_parse_xml_proto.py)
	@if ls gametools/parseProtoTool/protodir/*.proto > /dev/null 2>&1; then cp gametools/parseProtoTool/protodir/* protomsg/; fi
	@rm -rf gametools/parseProtoTool/protodir


debug:  
	@for dir in $(SUB_DIRS); \
		do \
		FIR_COMPILE_OPT='$(FIR_DEBUG_OPT) -DBUILD_STRING=$(BUILD_STRING)' PIC_FLAG='-fPIC' $(MAKE)  $(NJOBS) -C $$dir || exit 1; \
	done
	@for dir in $(SUB_TARGETS); \
		do \
		$(call SubMKDIR, $(BUILD_DIR_ROOT)/$$dir/.obj);\
		FIR_COMPILE_OPT='$(FIR_DEBUG_OPT) -DBUILD_STRING=$(BUILD_STRING)' PIC_FLAG='-fPIC' $(MAKE) ISDebug=TRUE $(NJOBS) -C $$dir || exit 1; \
	done
release: CheckReLeaseBin cpRleaseres
	@for dir in $(SUB_DIRS); \
		do \
		FIR_COMPILE_OPT='$(FIR_RELEASE_OPT) -DBUILD_STRING=$(BUILD_STRING)' PIC_FLAG='-fPIC' $(MAKE)  $(NJOBS) -C $$dir || exit 1; \
	done
	@for dir in $(SUB_TARGETS); \
		do \
		$(call SubMKDIR, $(BUILD_DIR_ROOT)/$$dir/.releaseobj);\
		FIR_COMPILE_OPT='$(FIR_RELEASE_OPT) -DBUILD_STRING=$(BUILD_STRING)' PIC_FLAG='-fPIC' $(MAKE)  $(NJOBS) -C $$dir || exit 1; \
	done
	@(mv $(ReLeaseBin)/Superserver $(ReLeaseBin)/superserver/)
	@(mv $(ReLeaseBin)/Recordserver $(ReLeaseBin)/recordserver/)
	@(mv $(ReLeaseBin)/Gateserver $(ReLeaseBin)/gateserver/)
	@(mv $(ReLeaseBin)/Sceneserver $(ReLeaseBin)/sceneserver/)
	@(mv $(ReLeaseBin)/Flserver $(ReLeaseBin)/flserver/)
	@(mv $(ReLeaseBin)/Robotserver $(ReLeaseBin)/robotserver/)
	@(mv $(ReLeaseBin)/Gmtoolserver $(ReLeaseBin)/gmtoolserver/)
	@(mv $(ReLeaseBin)/Resourceserver $(ReLeaseBin)/resourceserver/)
	#SUPPER
	@objcopy --only-keep-debug $(ReLeaseBin)/superserver/Superserver $(ReLeaseBin)/superserver.symbol
	@objcopy --strip-debug $(ReLeaseBin)/superserver/Superserver
	@objcopy --add-gnu-debuglink $(ReLeaseBin)/superserver.symbol $(ReLeaseBin)/superserver/Superserver
	#recordserver 
	@objcopy --only-keep-debug $(ReLeaseBin)/recordserver/Recordserver $(ReLeaseBin)/recordserver.symbol
	@objcopy --strip-debug $(ReLeaseBin)/recordserver/Recordserver
	@objcopy --add-gnu-debuglink $(ReLeaseBin)/recordserver.symbol $(ReLeaseBin)/recordserver/Recordserver
	#sceneserver
	@objcopy --only-keep-debug $(ReLeaseBin)/sceneserver/Sceneserver $(ReLeaseBin)/scenesserver.symbol
	@objcopy --strip-debug $(ReLeaseBin)/sceneserver/Sceneserver
	@objcopy --add-gnu-debuglink $(ReLeaseBin)/scenesserver.symbol $(ReLeaseBin)/sceneserver/Sceneserver
	#gateserver
	@objcopy --only-keep-debug $(ReLeaseBin)/gateserver/Gateserver $(ReLeaseBin)/gateserver.symbol
	@objcopy --strip-debug $(ReLeaseBin)/gateserver/Gateserver	
	@objcopy --add-gnu-debuglink $(ReLeaseBin)/gateserver.symbol $(ReLeaseBin)/gateserver/Gateserver
	#flserver
	@objcopy --only-keep-debug $(ReLeaseBin)/flserver/Flserver $(ReLeaseBin)/flserver.symbol
	@objcopy --strip-debug $(ReLeaseBin)/flserver/Flserver	
	@objcopy --add-gnu-debuglink $(ReLeaseBin)/flserver.symbol $(ReLeaseBin)/flserver/Flserver
	#Robotserver
	@objcopy --only-keep-debug $(ReLeaseBin)/robotserver/Robotserver $(ReLeaseBin)/robotserver.symbol
	@objcopy --strip-debug $(ReLeaseBin)/robotserver/Robotserver	
	@objcopy --add-gnu-debuglink $(ReLeaseBin)/robotserver.symbol $(ReLeaseBin)/robotserver/Robotserver
	#gmtoolserver
	@objcopy --only-keep-debug $(ReLeaseBin)/gmtoolserver/Gmtoolserver $(ReLeaseBin)/gmtoolserver.symbol
	@objcopy --strip-debug $(ReLeaseBin)/gmtoolserver/Gmtoolserver
	@objcopy --add-gnu-debuglink $(ReLeaseBin)/gmtoolserver.symbol $(ReLeaseBin)/gmtoolserver/Gmtoolserver
	#Resourceserver
	@objcopy --only-keep-debug $(ReLeaseBin)/resourceserver/Resourceserver $(ReLeaseBin)/resourceserver.symbol
	@objcopy --strip-debug $(ReLeaseBin)/resourceserver/Resourceserver
	@objcopy --add-gnu-debuglink $(ReLeaseBin)/resourceserver.symbol $(ReLeaseBin)/resourceserver/Resourceserver
	@(tar cvzf $(ReLeaseBin).tar $(ReLeaseBin)/)
	@(rm -rf $(ReLeaseBin)/)
clear:
	@(cd superserver/;rm -rf .obj;rm -rf .releaseobj)
	@(cd recordserver/;rm -rf .obj;rm -rf .releaseobj)
	@(cd sceneserver/;rm -rf .obj;rm -rf .releaseobj)
	@(cd gateserver/;rm -rf .obj;rm -rf .releaseobj)
	@(cd flserver/;rm -rf .obj;rm -rf .releaseobj)
	@(cd robotserver/;rm -rf .obj;rm -rf .releaseobj)
	@(cd gmtoolserver/;rm -rf .obj;rm -rf .releaseobj)
	@(cd protocpp/;rm -rf .obj;rm -rf .releaseobj)
	@(cd resourceserver/;rm -rf .obj;rm -rf .releaseobj)

clearswp:
	@(cd superserver/;rm -rf *.*~ .*.*.swp)
	@(cd recordserver/;rm -rf *.*~ .*.*.swp)
	@(cd sceneserver/;rm -rf *.*~ .*.*.swp)
	@(cd gateserver/;rm -rf *.*~ .*.*.swp)
	@(cd flserver/;rm -rf *.*~ .*.*.swp)
	@(cd protocol/;rm -rf *.*~ .*.*.swp)
	@(cd robotserver/;rm -rf *.*~ .*.*.swp)
	@(cd resourceserver/;rm -rf *.*~ .*.*.swp)
	@(rm -rf *.*~ .*.*.swp)
cpRleaseres:
	@(rsync -av --delete --exclude='.svn' $(PWD)/gametools/parseXmlTool/xmldir $(ReLeaseBin)/gametools/parseXmlTool/)
	@(rsync -av --delete --exclude='.svn' $(PWD)/gametools/lua $(ReLeaseBin)/gametools/)
	@(rm -rf $(ReLeaseBin)/gametools/parseXmlTool/xmldir/redis_config.xml)
	@(rsync -av --delete --exclude='.svn' $(PWD)/tbx $(ReLeaseBin)/)
	@(rsync -av --delete --exclude='.svn' $(PWD)/mapxml $(ReLeaseBin)/)
	@(rsync -av --delete --exclude='.svn' $(PWD)/gametools/lua $(ReLeaseBin)/gametools/)
	@(rsync -av --delete --exclude='.svn' $(PWD)/config_sample.xml $(ReLeaseBin)/)
	@(rsync -av --delete --exclude='.svn' $(PWD)/superserver/loginServerList_sample.xml $(ReLeaseBin)/superserver/)
	@(rsync -av --delete --exclude='.svn' $(PWD)/runserver_sample.sh $(ReLeaseBin)/)
	@(rsync -av --delete --exclude='.svn' $(PWD)/sql $(ReLeaseBin)/)
	@(cp -u $(PWD)/forbidWords $(ReLeaseBin)/forbidWords)
	@(rm -rf $(ReLeaseBin)/sql/sqlconfig.xml)
ctags:
	@ctags -R -h ".c.cpp.h" -o tags

systags:
	@ctags -I _THROW --file-scope=yes --langmap=c:+.h --languages=c,c++ --links=yes --c-kinds=+p -R -f ~/.vim/systags /usr/include /usr/local/include

proto:
	@rm -rf ./protocpp/*.{h,cpp,cc}
	@./protoc -I=./protomsg --cpp_out=./protocpp ./protomsg/*.proto

distclean: 
	@find . -iname .\*.d -exec rm \{\} \;

clean:
	@for dir in $(SUB_DIRS); \
	do \
	$(MAKE)  -C $$dir clean; \
	done
	@for dir in $(SUB_TARGETS); \
	do \
	$(MAKE) ISDebug=TRUE -C $$dir clean; \
	done

cleanrelease:
	@for dir in $(SUB_DIRS); \
	do \
	$(MAKE)  -C $$dir clean; \
	done
	@for dir in $(SUB_TARGETS); \
	do \
	$(MAKE) -C $$dir clean; \
	done

release_cp:
	@(rsync -av --delete   $(ReLeaseBin)/mapxml  $(ReLeaseBin)/Release/)
	@(rsync -av --delete   $(ReLeaseBin)/tbx  $(ReLeaseBin)/Release/)
	$(call MKDIR,$(ReLeaseBin)/Release/gametools)
	$(call MKDIR,$(ReLeaseBin)/gametools/Release/parseXmlTool)
	@(rsync -av --delete   $(ReLeaseBin)/gametools/parseXmlTool/xmldir  $(ReLeaseBin)/Release/gametools/parseXmlTool/)
	@(rsync -av --delete   $(ReLeaseBin)/gametools/lua  $(ReLeaseBin)/Release/gametools/)
	@(rsync -av --delete   $(ReLeaseBin)/sql $(ReLeaseBin)/Release/)
	@cp -avf $(ReLeaseBin)/forbidWords $(ReLeaseBin)/Release/
	@cp -avf $(ReLeaseBin)/config_sample.xml $(ReLeaseBin)/Release/
	@cp -avf $(ReLeaseBin)/runserver_sample.sh $(ReLeaseBin)/Release/
	@cp -avf $(ReLeaseBin)/flserver $(ReLeaseBin)/Release/
	@cp -avf $(ReLeaseBin)/superserver $(ReLeaseBin)/Release/
	@cp -avf $(ReLeaseBin)/recordserver $(ReLeaseBin)/Release/
	@cp -avf $(ReLeaseBin)/sceneserver $(ReLeaseBin)/Release/
	@cp -avf $(ReLeaseBin)/gateserver $(ReLeaseBin)/Release/
	@cp -avf $(ReLeaseBin)/robotserver $(ReLeaseBin)/Release/
	#SUPPER
	@objcopy --only-keep-debug $(ReLeaseBin)/Release/superserver/Superserver $(ReLeaseBin)/Release/superserver.symbol
	@objcopy --strip-debug $(ReLeaseBin)/Release/superserver/Superserver
	@objcopy --add-gnu-debuglink $(ReLeaseBin)/Release/superserver.symbol $(ReLeaseBin)/Release/superserver/Superserver
	#recordserver 
	@objcopy --only-keep-debug $(ReLeaseBin)/Release/recordserver/Recordserver $(ReLeaseBin)/Release/recordserver.symbol
	@objcopy --strip-debug $(ReLeaseBin)/Release/recordserver/Recordserver
	@objcopy --add-gnu-debuglink $(ReLeaseBin)/Release/recordserver.symbol $(ReLeaseBin)/Release/recordserver/Recordserver
	#sceneserver
	@objcopy --only-keep-debug $(ReLeaseBin)/Release/sceneserver/Sceneserver $(ReLeaseBin)/Release/scenesserver.symbol
	@objcopy --strip-debug $(ReLeaseBin)/Release/sceneserver/Sceneserver
	@objcopy --add-gnu-debuglink $(ReLeaseBin)/Release/scenesserver.symbol $(ReLeaseBin)/Release/sceneserver/Sceneserver
	#gateserver
	@objcopy --only-keep-debug $(ReLeaseBin)/Release/gateserver/Gateserver $(ReLeaseBin)/Release/gateserver.symbol
	@objcopy --strip-debug $(ReLeaseBin)/Release/gateserver/Gateserver	
	@objcopy --add-gnu-debuglink $(ReLeaseBin)/Release/gateserver.symbol $(ReLeaseBin)/Release/gateserver/Gateserver
	#flserver
	@objcopy --only-keep-debug $(ReLeaseBin)/Release/flserver/Flserver $(ReLeaseBin)/Release/flserver.symbol
	@objcopy --strip-debug $(ReLeaseBin)/Release/flserver/Flserver	
	@objcopy --add-gnu-debuglink $(ReLeaseBin)/Release/flserver.symbol $(ReLeaseBin)/Release/flserver/Flserver
	#Robotserver
	@objcopy --only-keep-debug $(ReLeaseBin)/Release/robotserver/Robotserver $(ReLeaseBin)/Release/robotserver.symbol
	@objcopy --strip-debug $(ReLeaseBin)/Release/robotserver/Robotserver	
	@objcopy --add-gnu-debuglink $(ReLeaseBin)/Release/robotserver.symbol $(ReLeaseBin)/Release/robotserver/Robotserver
	#打包
	@cd $(ReLeaseBin)/Release ; tar cvzf $(ReLeaseBin)/server-$(VERSION).tgz mapxml tbx gametools sql forbidWords config_sample.xml runserver_sample.sh robotserver superserver recordserver sceneserver gateserver flserver
	@cd $(ReLeaseBin)/Release ; tar cvzf $(ReLeaseBin)/server-$(VERSION).symbol.tgz superserver.symbol recordserver.symbol scenesserver.symbol gateserver.symbol flserver.symbol robotserver.symbol
	@rm -rf $(ReLeaseBin)/Release
	@echo 生成包 server-$(VERSION).tgz server-$(VERSION).symbol.tgz
touch:
	touch .now
	find . -cnewer .now
	find . -cnewer .now -exec touch {} \;
	rm -f .now

