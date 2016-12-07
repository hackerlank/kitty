/* LzmaUtil.c -- Test application for LZMA compression
2010-09-20 : Igor Pavlov : Public domain */

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Alloc.h"
#include "7zFile.h"
#include "7zVersion.h"
#include "LzmaDec.h"
#include "LzmaEnc.h"
#include "LzmaUtil.h"

const char *kCantReadMessage = "Can not read input file";
const char *kCantWriteMessage = "Can not write output file";
const char *kCantAllocateMessage = "Can not allocate memory";
const char *kDataErrorMessage = "Data error";

static void *SzAlloc(void *p, size_t size) { p = p; return MyAlloc(size); }
static void SzFree(void *p, void *address) { p = p; MyFree(address); }
static ISzAlloc g_Alloc = { SzAlloc, SzFree };

void PrintHelp(char *buffer)
{
  strcat(buffer, "\nLZMA Utility " MY_VERSION_COPYRIGHT_DATE "\n"
      "\nUsage:  lzma <e|d> inputFile outputFile\n"
             "  e: encode file\n"
             "  d: decode file\n");
}

int PrintError(char *buffer, const char *message)
{
  strcat(buffer, "\nError: ");
  strcat(buffer, message);
  strcat(buffer, "\n");
  return 1;
}

int PrintErrorNumber(char *buffer, SRes val)
{
  sprintf(buffer + strlen(buffer), "\nError code: %x\n", (unsigned)val);
  return 1;
}

int PrintUserError(char *buffer)
{
  return PrintError(buffer, "Incorrect command");
}

#define IN_BUF_SIZE (1 << 16)
#define OUT_BUF_SIZE (1 << 16)

static SRes Decode2(CLzmaDec *state, ISeqOutStream *outStream, ISeqInStream *inStream,
    UInt64 unpackSize)
{
  int thereIsSize = (unpackSize != (UInt64)(Int64)-1);
  Byte inBuf[IN_BUF_SIZE];
  Byte outBuf[OUT_BUF_SIZE];
  size_t inPos = 0, inSize = 0, outPos = 0;
  LzmaDec_Init(state);
  for (;;)
  {
    if (inPos == inSize)
    {
      inSize = IN_BUF_SIZE;
      RINOK(inStream->Read(inStream, inBuf, &inSize));
      inPos = 0;
    }
    {
      SRes res;
      SizeT inProcessed = inSize - inPos;
      SizeT outProcessed = OUT_BUF_SIZE - outPos;
      ELzmaFinishMode finishMode = LZMA_FINISH_ANY;
      ELzmaStatus status;
      if (thereIsSize && outProcessed > unpackSize)
      {
        outProcessed = (SizeT)unpackSize;
        finishMode = LZMA_FINISH_END;
      }
      
      res = LzmaDec_DecodeToBuf(state, outBuf + outPos, &outProcessed,
        inBuf + inPos, &inProcessed, finishMode, &status);
      inPos += inProcessed;
      outPos += outProcessed;
      unpackSize -= outProcessed;
      
      if (outStream)
        if (outStream->Write(outStream, outBuf, outPos) != outPos)
          return SZ_ERROR_WRITE;
        
      outPos = 0;
      
      if (res != SZ_OK || thereIsSize && unpackSize == 0)
        return res;
      
      if (inProcessed == 0 && outProcessed == 0)
      {
        if (thereIsSize || status != LZMA_STATUS_FINISHED_WITH_MARK)
          return SZ_ERROR_DATA;
        return res;
      }
    }
  }
}

SRes Decode(ISeqOutStream *outStream, ISeqInStream *inStream)
{
  UInt64 unpackSize;
  int i;
  SRes res = 0;

  CLzmaDec state;

  /* header: 5 bytes of LZMA properties and 8 bytes of uncompressed size */
  unsigned char header[LZMA_PROPS_SIZE + 8];

  /* Read and parse header */

  RINOK(SeqInStream_Read(inStream, header, sizeof(header)));

  unpackSize = 0;
  for (i = 0; i < 8; i++)
    unpackSize += (UInt64)header[LZMA_PROPS_SIZE + i] << (i * 8);

  LzmaDec_Construct(&state);
  RINOK(LzmaDec_Allocate(&state, header, LZMA_PROPS_SIZE, &g_Alloc));
  res = Decode2(&state, outStream, inStream, unpackSize);
  LzmaDec_Free(&state, &g_Alloc);
  return res;
}

SRes Encode(ISeqOutStream *outStream, ISeqInStream *inStream, UInt64 fileSize, char *rs)
{
  CLzmaEncHandle enc;
  SRes res;
  CLzmaEncProps props;

  rs = rs;

  enc = LzmaEnc_Create(&g_Alloc);
  if (enc == 0)
    return SZ_ERROR_MEM;

  LzmaEncProps_Init(&props);
  res = LzmaEnc_SetProps(enc, &props);

  if (res == SZ_OK)
  {
    Byte header[LZMA_PROPS_SIZE + 8];
    size_t headerSize = LZMA_PROPS_SIZE;
    int i;

    res = LzmaEnc_WriteProperties(enc, header, &headerSize);
    for (i = 0; i < 8; i++)
      header[headerSize++] = (Byte)(fileSize >> (8 * i));
    if (outStream->Write(outStream, header, headerSize) != headerSize)
      res = SZ_ERROR_WRITE;
    else
    {
      if (res == SZ_OK)
        res = LzmaEnc_Encode(enc, outStream, inStream, NULL, &g_Alloc, &g_Alloc);
    }
  }
  LzmaEnc_Destroy(enc, &g_Alloc, &g_Alloc);
  return res;
}

int main2(int numArgs, const char *args[], char *rs)
{
  CFileSeqInStream inStream;
  CFileOutStream outStream;
  char c;
  int res;
  int encodeMode;
  Bool useOutFile = False;

  FileSeqInStream_CreateVTable(&inStream);
  File_Construct(&inStream.file);

  FileOutStream_CreateVTable(&outStream);
  File_Construct(&outStream.file);

  if (numArgs == 1)
  {
    PrintHelp(rs);
    return 0;
  }

  if (numArgs < 3 || numArgs > 4 || strlen(args[1]) != 1)
    return PrintUserError(rs);

  c = args[1][0];
  encodeMode = (c == 'e' || c == 'E');
  if (!encodeMode && c != 'd' && c != 'D')
    return PrintUserError(rs);

  {
    size_t t4 = sizeof(UInt32);
    size_t t8 = sizeof(UInt64);
    if (t4 != 4 || t8 != 8)
      return PrintError(rs, "Incorrect UInt32 or UInt64");
  }

  if (InFile_Open(&inStream.file, args[2]) != 0)
    return PrintError(rs, "Can not open input file");

  if (numArgs > 3)
  {
    useOutFile = True;
    if (OutFile_Open(&outStream.file, args[3]) != 0)
      return PrintError(rs, "Can not open output file");
  }
  else if (encodeMode)
    PrintUserError(rs);

  if (encodeMode)
  {
    UInt64 fileSize;
    File_GetLength(&inStream.file, &fileSize);
    res = Encode(&outStream.s, &inStream.s, fileSize, rs);
  }
  else
  {
    res = Decode(&outStream.s, useOutFile ? &inStream.s : NULL);
  }

  if (useOutFile)
    File_Close(&outStream.file);
  File_Close(&inStream.file);

  if (res != SZ_OK)
  {
    if (res == SZ_ERROR_MEM)
      return PrintError(rs, kCantAllocateMessage);
    else if (res == SZ_ERROR_DATA)
      return PrintError(rs, kDataErrorMessage);
    else if (res == SZ_ERROR_WRITE)
      return PrintError(rs, kCantWriteMessage);
    else if (res == SZ_ERROR_READ)
      return PrintError(rs, kCantReadMessage);
    return PrintErrorNumber(rs, res);
  }
  return 0;
}

#if 0
int MY_CDECL main(int numArgs, const char *args[])
{
  char rs[800] = { 0 };
  int res = main2(numArgs, args, rs);
  fputs(rs, stdout);
  return res;
}
#endif

SRes VectorInStream_Read(void *p, void *buf, size_t *size)
{
    VectorInStream *ctx = (VectorInStream*)p;
    *size = ctx->Buf->size() - ctx->BufPos < *size ? ctx->Buf->size() - ctx->BufPos : *size;
    if (*size)
        memcpy(buf, &(*ctx->Buf)[ctx->BufPos], *size);
    ctx->BufPos += *size;
    return SZ_OK;
}


size_t VectorOutStream_Write(void *p, const void *buf, size_t size)
{
  VectorOutStream *ctx = (VectorOutStream*)p;
  if (size)
  {
    unsigned oldSize = ctx->Buf->size();
    ctx->Buf->resize(oldSize + size);
    memcpy(&(*ctx->Buf)[oldSize], buf, size);
  }
  return size;
}

void CompressInc(std::vector<unsigned char> &outBuf, const std::vector<unsigned char> &inBuf)
{
  CLzmaEncHandle enc = LzmaEnc_Create(&g_Alloc);
  assert(enc);

  CLzmaEncProps props;
  LzmaEncProps_Init(&props);
  props.writeEndMark = 1; // 0 or 1
  
  SRes res = LzmaEnc_SetProps(enc, &props);
  assert(res == SZ_OK);

  SizeT propsSize = LZMA_PROPS_SIZE;
  outBuf.resize(propsSize);

  res = LzmaEnc_WriteProperties(enc, &outBuf[0], &propsSize);
  assert(res == SZ_OK && propsSize == LZMA_PROPS_SIZE);

  VectorInStream inStream = { &VectorInStream_Read, &inBuf, 0 };
  VectorOutStream outStream = { &VectorOutStream_Write, &outBuf };

  res = LzmaEnc_Encode(enc,
    (ISeqOutStream*)&outStream, (ISeqInStream*)&inStream,
    0, &g_Alloc, &g_Alloc);
  assert(res == SZ_OK);

  LzmaEnc_Destroy(enc, &g_Alloc, &g_Alloc);
}

void UncompressInc(std::vector<unsigned char> &outBuf,const std::vector<unsigned char> &inBuf)
{
  CLzmaDec dec;  
  LzmaDec_Construct(&dec);
  
  SRes res = LzmaDec_Allocate(&dec, &inBuf[0], LZMA_PROPS_SIZE, &g_Alloc);
  assert(res == SZ_OK);

  LzmaDec_Init(&dec);

  outBuf.resize(1024*64*10);
  unsigned outPos = 0, inPos = LZMA_PROPS_SIZE;
  ELzmaStatus status;
  const unsigned BUF_SIZE = 10240;
  while (outPos < outBuf.size())
  {
    //unsigned destLen = min(BUF_SIZE, outBuf.size() - outPos);
    SizeT destLen = outBuf.size() - outPos < BUF_SIZE ? outBuf.size() - outPos : BUF_SIZE;
    //unsigned srcLen  = min(BUF_SIZE, inBuf.size() - inPos);
    SizeT srcLen = inBuf.size() - inPos < BUF_SIZE ? inBuf.size() - inPos : BUF_SIZE;
    unsigned srcLenOld = srcLen, destLenOld = destLen;
    res = LzmaDec_DecodeToBuf(&dec,
      &outBuf[outPos], &destLen,
      &inBuf[inPos], &srcLen,
      (outPos + destLen == outBuf.size())
      ? LZMA_FINISH_END : LZMA_FINISH_ANY, &status);
    assert(res == SZ_OK);
    inPos += srcLen;
    outPos += destLen;
    if (status == LZMA_STATUS_FINISHED_WITH_MARK)
      break;
  }

  LzmaDec_Free(&dec, &g_Alloc);
  outBuf.resize(outPos);
}


