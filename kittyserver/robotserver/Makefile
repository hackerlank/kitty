# $Id: Makefile 53 2013-04-17 05:03:41Z zhongjunwei $

#CXX = /usr/bin/g++
CXXFLAGS = -std=gnu++0x $(FIR_COMPILE_OPT) -I../base -I../protocol -I../base/hiredis -I../base/lzma -I../protocpp -Wall -D_GNU_SOURCE -D_REENTRANT `mysql_config --cflags` `xml2-config --cflags` `freetype-config --cflags`
CXXLFLAGS = $(FIR_COMPILE_OPT) -Wall -L../base -lfir -lencdec -llzma -lpthread -lprotobuf -lproto -lrt -lz -lhiredis -L../xml -lxml `mysql_config --libs_r` `xml2-config --libs` -llog4cxx -lgd `freetype-config --libs`

LIB_FIR = ../base/libfir.a
LIB_PROMSG = ../base/libcrypto.a
SELFTARGET = robotserver
TARGET = Robotserver

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

$(ALL_TARGETS): $(OBJS) $(LIB_FIR) $(LIB_PROMSG)
	$(CXX) $(OBJS) -o $@ $(CXXLFLAGS)

clean:
	@rm -rf $(OBJPATH) $(ALL_TARGETS) core *~


