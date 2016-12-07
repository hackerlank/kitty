# $Id: Makefile 53 2013-04-17 05:03:41Z zhongjunwei $

#CXX = /usr/bin/g++
CXXFLAGS = -std=gnu++0x $(FIR_COMPILE_OPT) -I../base -I../protocol -I../base/curl -I../base/hiredis -I../base/lzma -I../tbxcpp -I../protocpp -Wall -D_GNU_SOURCE -D_REENTRANT `mysql_config --cflags` `xml2-config --cflags`

#CXXLFLAGS = $(FIR_COMPILE_OPT) -Wall -L../base -lfir -ljson -lcurl -lencdec -lhiredis -lpthread -lrt -lz -L../xml -lxml -L../tbxcpp -ltbx -lprotobuf -lproto `mysql_config --libs_r` `xml2-config --libs` -llog4cxx

CXXLFLAGS = $(FIR_COMPILE_OPT) -Wall -L../tbxcpp -ltbx -lprotobuf -lproto `mysql_config --libs_r` `xml2-config --libs` -llog4cxx -L../base -lfir -lcurl -ljson -lencdec -lhiredis -llzma -lpthread -lrt -lz -L../xml -lxml 
LIB_FIR = ../tbxcpp/libtbx.a ../base/libfir.a
LIB_CRYPTO = ../base/libcrypto.a 
SELFTARGET = superserver
TARGET = Superserver

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

$(ALL_TARGETS): $(OBJS) $(LIB_FIR) $(LIB_CRYPTO)
	$(CXX) $(OBJS) -o $@ $(CXXLFLAGS)

clean:
	@rm -rf $(OBJPATH) $(ALL_TARGETS) core *~

