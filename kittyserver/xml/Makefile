#$Id: Makefile 250 2013-05-29 02:02:52Z chengxie $

CXXFLAGS = -std=gnu++0x $(FIR_COMPILE_OPT) -g -Wno-deprecated -I../base -I../common  -I../base/rapidxml -I../base/hiredis -I../protosMessageCPP -Wall -D_GNU_SOURCE -D_REENTRANT -D_USE_GCC_4 `mysql_config --cflags` `xml2-config --cflags`
CXXLFLAGS = $(FIR_COMPILE_OPT) -Wall -L../base -lfir -lencdec -lhiredis -lpthread -lprotobuf -lproto -lrt -lz -llog4cxx `mysql_config --libs_r` `xml2-config --libs`
TARGET = libxml.a
SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:%.cpp=%.o)
DEPS = $(OBJS:%.o=.%.d)









#OBJS := $(patsubst %.cpp, $(BUILD_PATH)/%.o, $(wildcard *.cpp))
#DEPS = $(OBJS:$(BUILD_PATH)/%.o=$(BUILD_PATH)/.%.d)

all: $(TARGET)

#include $(DEPS)
$(DEPS): .%.d: %.cpp
	@$(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$

$(OBJS): %.o: %.cpp $(DEPS)
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(TARGET): $(OBJS) 
	@rm -rf $(TARGET)
	@ar cru $(TARGET) $(OBJS)
	@ranlib $(TARGET)

clean:
	@rm -rf $(OBJS) $(TARGET) core *~ .*.d
