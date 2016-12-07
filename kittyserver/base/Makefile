# $Id: Makefile 64 2013-04-23 02:05:08Z zhongjunwei $
#include des/Makefile

#CXX = /usr/bin/g++
CXXFLAGS = -std=gnu++0x $(FIR_COMPILE_OPT) $(PIC_FLAG) -I. -I./curl -I./hiredis -I./lzma -I./EncDec -I../protocpp -I../protocol -I /usr/local/openssl/include -I /usr/local/libxml2/include/libxml2 -Wall -D_GNU_SOURCE -D_REENTRANT `mysql_config --cflags` `xml2-config --cflags` `freetype-config --cflags`
CXXLFLAGS = $(FIR_COMPILE_OPT) -Wall -lcurl -lpthread -lrt -lencdec -lz -lhiredis -llzma -lproto -lprotobuf -lxml `xml2-config --libs` `mysql_config --libs_r` -llog4cxx -lgd `freetype-config --libs`

LIB_FIR = libfir.a 
DYN_LIB_FIR = libfir.so

SRCS = $(wildcard *.cpp)
OBJS = $(SRCS:%.cpp=%.o)
DEPS = $(OBJS:%.o=.%.d)

all: $(LIB_FIR)

#include $(DEPS)
$(DEPS): .%.d: %.cpp
	@$(CXX) -MM $(CXXFLAGS) $< > $@.$$$$; sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$

$(OBJS): %.o: %.cpp $(DEPS)
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(LIB_FIR): $(OBJS) 
	@rm -rf $(LIB_FIR)
	@ar cru $(LIB_FIR) $(OBJS) 
	@ranlib $(LIB_FIR)
	echo $(PIC_FLAG)
	@if [ -n "$(PIC_FLAG)" ]; then \
		$(CXX) -shared -o $(DYN_LIB_FIR) $(OBJS); \
		rm lib*.so; \
	fi

clean:
	@rm -rf $(OBJS) $(LIB_FIR) core *~ .*.d

