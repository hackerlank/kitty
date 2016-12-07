# $Id: Makefile 4 2013-03-14 09:04:58Z zhongjunwei $

#CXX = /usr/bin/g++
CXXFLAGS = $(FIR_COMPILE_OPT) $(PIC_FLAG) -g -I. -Wall -D_GNU_SOURCE -D_REENTRANT -I /usr/local/openssl/include `mysql_config --cflags` `xml2-config --cflags`
CXXLFLAGS = $(FIR_COMPILE_OPT) -Wall -lpthread -lz `mysql_config --libs_r` `xml2-config --libs` -llog4cxx

OBJS = md5.o blowfish.o md5ex.o mycast.o mydes.o myidea.o myrc5.o EncDec.o #md5c.o

#LIB_FIR = ../libzebra.a
LIB_ENCDEC = ../libencdec.a
DYN_LIB_ENCDEC = ../libencdec.so
SRCS = $(OBJS:%.o=%.cpp)
DEPS = $(OBJS:%.o=.%.d)

all: $(LIB_ENCDEC)

#include $(DEPS)
#$(DEPS): .%.d: %.cpp
#	@$(CXX) -M $(CXXFLAGS) $< > $@.$$$$; sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$
	
$(OBJS): %.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@
	gcc -c $(CXXFLAGS) md5c.c -o md5c.o

$(LIB_ENCDEC): $(OBJS)
	@rm -rf $(LIB_ENCDEC)
	@ar cru $(LIB_ENCDEC) $(OBJS) md5c.o
	@ranlib $(LIB_ENCDEC)
	@if [ -n "$(PIC_FLAG)" ]; then \
		$(CXX) -shared -o $(DYN_LIB_ENCDEC) $(OBJS) md5c.o; \
	fi

clean:
	@rm -rf $(OBJS) $(LIB_ENCDEC) core *~
