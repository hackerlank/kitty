LIB_LZMA = ../liblzma.a
CXX = g++
LIB =
RM = rm -f
CFLAGS = -c -O2 -Wall -D_7ZIP_ST

OBJS = \
  LzmaUtil.o \
  Alloc.o \
  LzFind.o \
  LzmaDec.o \
  LzmaEnc.o \
  7zFile.o \
  7zStream.o \
  LzmaLib.o \
  quicklz.o \

all: $(LIB_LZMA)

LzmaUtil.o: LzmaUtil.c
	$(CXX) $(CFLAGS) LzmaUtil.c

Alloc.o:Alloc.c
	$(CXX) $(CFLAGS) Alloc.c

LzFind.o:LzFind.c
	$(CXX) $(CFLAGS) LzFind.c

LzmaDec.o:LzmaDec.c
	$(CXX) $(CFLAGS) LzmaDec.c

LzmaEnc.o:LzmaEnc.c
	$(CXX) $(CFLAGS) LzmaEnc.c

7zFile.o:7zFile.c
	$(CXX) $(CFLAGS) 7zFile.c

7zStream.o:7zStream.c
	$(CXX) $(CFLAGS) 7zStream.c

LzmaLib.o:LzmaLib.c
	$(CXX) $(CFLAGS) LzmaLib.c

quicklz.o:quicklz.c
	$(CXX) $(CFLAGS) quicklz.c

clean:
	-$(RM) $(LIB_LZMA) $(OBJS)

$(LIB_LZMA): $(OBJS)
	@rm -rf $(LIB_LZMA)
	@ar cru $(LIB_LZMA) $(OBJS)
	@ranlib $(LIB_LZMA)

