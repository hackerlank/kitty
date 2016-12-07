# $Id: Makefile 43 2013-04-10 10:29:18Z pengqiangqiang $

#CXX = /usr/bin/g++
CXXFLAGS = $(FIR_COMPILE_OPT) -g -Wno-deprecated -I../base -I../common -I../base/hiredis -I../protosMessageCPP -Wall -D_GNU_SOURCE -D_REENTRANT `mysql_config --cflags` `xml2-config --cflags`
CXXLFLAGS = $(FIR_COMPILE_OPT) -Wall -L../base -lfir -lencdec -lhiredis -lpthread -lprotobuf -lproto -lrt -lz -llog4cxx `mysql_config --libs_r` `xml2-config --libs`
LIB_FIR = ../base/libfir.a
#LIB_PROTO = ../base/libproto.a


ALL_TARGETS = arraytest memdbtest prototest
all: $(ALL_TARGETS)

$(OBJS): %.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $< -o $@
	
include .arraytest.d
.arraytest.d: .%.d: %.cpp
	@$(CXX) -M $(CXXFLAGS) $< > $@.$$$$; sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$
arraytest: arraytest.o $(LIB_FIR)
	$(CXX) arraytest.o -o $@ $(CXXLFLAGS)

include .memdbtest.d
.memdbtest.d: .%.d: %.cpp
	@$(CXX) -M $(CXXFLAGS) $< > $@.$$$$; sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$
memdbtest: memdbtest.o $(LIB_FIR)
	$(CXX) memdbtest.o -o $@ $(CXXLFLAGS)

include .prototest.d
.prototest.d: .%.d: %.cpp
	@$(CXX) -M $(CXXFLAGS) $< > $@.$$$$; sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$
prototest: prototest.o $(LIB_FIR)
	$(CXX) prototest.o -o $@ $(CXXLFLAGS)

clean:
	@rm -rf *.o $(ALL_TARGETS) core *~

