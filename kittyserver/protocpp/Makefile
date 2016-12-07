# $Id: Makefile 43 2013-04-10 10:29:18Z $

#CXX = /usr/bin/g++
CXXFLAGS = $(FIR_COMPILE_OPT) $(PIC_FLAG) -g -I. -Wall -D_GNU_SOURCE -D_REENTRANT `mysql_config --cflags` `xml2-config --cflags`
CXXLFLAGS = $(FIR_COMPILE_OPT) -Wall -lpthread -lprotobuf -lprotoc -lz `mysql_config --libs_r` `xml2-config --libs` -llog4cxx

OBJS = login.pb.o serialize.pb.o item.pb.o trade.pb.o build.pb.o common.pb.o warehouse.pb.o kittygarden.pb.o task.pb.o atlas.pb.o gm.pb.o achievement.pb.o enterkitty.pb.o carnival.pb.o friend.pb.o event.pb.o recordserialize.pb.o email.pb.o produce.pb.o usecardbuild.pb.o toy.pb.o dress.pb.o paper.pb.o divine.pb.o family.pb.o burstevent.pb.o littergame.pb.o auction.pb.o order.pb.o chat.pb.o giftpackage.pb.o rank.pb.o guide.pb.o active.pb.o gmtool.pb.o gmtest.pb.o room.pb.o system.pb.o trainorder.pb.o ordersystem.pb.o signin.pb.o composite.pb.o recharge.pb.o market.pb.o unitbuild.pb.o resource.pb.o playeractive.pb.o

LIB_PROTO = ../base/libproto.a
#DYN_LIB_PROTO = ../base/libproto.so
SRCS = $(OBJS:%.o=%.cpp)
DEPS = $(OBJS:%.o=.%.d)

all: $(LIB_PROTO)

#include $(DEPS)
#$(DEPS): .%.d: %.cpp
#	@$(CXX) -M $(CXXFLAGS) $< > $@.$$$$; sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; rm -f $@.$$$$
	
$(OBJS): %.o: %.cc
	$(CXX) -c $(CXXFLAGS) $< -o $@

$(LIB_PROTO): $(OBJS)
	@rm -rf $(LIB_PROTO)
	@ar cru $(LIB_PROTO) $(OBJS)
	@ranlib $(LIB_PROTO)

clean:
	@rm -rf $(OBJS) $(LIB_PROTO) core *~
