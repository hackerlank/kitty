# $Id: Makefile 67 2013-04-23 09:44:20Z pengqiangqiang $

#CXX = /usr/bin/g++
CXXFLAGS = -std=gnu++0x $(FIR_COMPILE_OPT) -Wall -g3 -I../base -I../base/curl -I../protocol -I../common -I../tbxcpp -I../base/EncDec -I../base/hiredis -I../base/lzma -I../protocpp -Wno-deprecated -Wall -D_GNU_SOURCE -D_REENTRANT `mysql_config --cflags` `xml2-config --cflags`
CXXLFLAGS = $(FIR_COMPILE_OPT) -Wall -L../tbxcpp -ltbx `mysql_config --libs_r` `xml2-config --libs` -llog4cxx -L../base -lfir -lencdec -lpthread -lrt  -ljson -lz -lcurl -lhiredis -llzma -lprotobuf -lproto -L../xml -lxml 
LIB_FIR = ../tbxcpp/libtbx.a ../base/libfir.a
SELFTARGET = sceneserver
TARGET = Sceneserver
ifeq ($(ISDebug),TRUE)
	ALL_TARGETS = $(TARGET)
	OBJPATH = $(BUILD_DIR_ROOT)/$(SELFTARGET)/.obj/
else
	ALL_TARGETS = $(ReLeaseBin)/$(TARGET)
	OBJPATH = $(BUILD_DIR_ROOT)/$(SELFTARGET)/.releaseobj/
endif

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:%.cpp=$(OBJPATH)%.o)
DEPS = $(SRCS:%.cpp=$(OBJPATH).%.d)

all: $(ALL_TARGETS)
include $(DEPS)
$(DEPS): $(OBJPATH).%.d: %.cpp
	@$(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; sed 's,\($*\)\.o[ :]*,$(OBJPATH)\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$

$(OBJS):$(OBJPATH)%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(ALL_TARGETS): $(OBJS) $(LIB_FIR) 
	$(CXX) $(OBJS) -o $@ $(CXXLFLAGS)

clean:
	@rm -rf $(OBJPATH) $(ALL_TARGETS) core *~


