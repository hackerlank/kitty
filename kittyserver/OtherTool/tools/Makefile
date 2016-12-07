# $Id: Makefile 43 2013-04-10 10:29:18Z pengqiangqiang $

#CXX = /usr/bin/g++
CXXFLAGS = $(FIR_COMPILE_OPT) -g -Wno-deprecated -I../base -I../common -I../base/hiredis -I../protosMessageCPP -Wall -D_GNU_SOURCE -D_REENTRANT -D_USE_GCC_4 `mysql_config --cflags` `xml2-config --cflags`
CXXLFLAGS = $(FIR_COMPILE_OPT) -Wall -L../base -lfir -lencdec -lhiredis -lpthread -lprotobuf -lproto -lrt -lz -llog4cxx `mysql_config --libs_r` `xml2-config --libs`
LIB_FIR = ../base/libfir.a
#LIB_PROTO = ../base/libproto.a


ALL_TARGETS = dumpXml 
all: $(ALL_TARGETS)

$(OBJS): %.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@

include .dumpXml.d
.dumpXml.d: .%.d: %.cpp
	@$(CXX) -M $(CXXFLAGS) $< > $@.$$$$; sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$
dumpXml: dumpXml.o $(LIB_ZEBRA)
	$(CXX) dumpXml.o -o $@ $(CXXLFLAGS)

clean:
	@rm -rf *.o $(ALL_TARGETS) core *~

