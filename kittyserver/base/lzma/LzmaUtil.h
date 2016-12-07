#ifndef LZMAUTIL_H
#define LZMAUTIL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Alloc.h"
#include "7zFile.h"
#include "7zVersion.h"
#include "LzmaDec.h"
#include "LzmaEnc.h"
#include <vector>
#include <assert.h>

typedef struct 
{
  ISeqInStream SeqInStream;
  const std::vector<unsigned char> *Buf;
  unsigned BufPos;
} VectorInStream;

SRes VectorInStream_Read(void *p, void *buf, size_t *size);

typedef struct
{
  ISeqOutStream SeqOutStream;
  std::vector<unsigned char> *Buf;
} VectorOutStream;

size_t VectorOutStream_Write(void *p, const void *buf, size_t size);

void CompressInc(std::vector<unsigned char> &outBuf,const std::vector<unsigned char> &inBuf);

void UncompressInc(std::vector<unsigned char> &outBuf,const std::vector<unsigned char> &inBuf);

SRes Encode(ISeqOutStream *outStream, ISeqInStream *inStream, UInt64 fileSize, char *rs);
SRes Decode(ISeqOutStream *outStream, ISeqInStream *inStream);

#endif
