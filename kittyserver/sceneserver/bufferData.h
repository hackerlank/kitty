#ifndef BUFFER_DATA_H
#define BUFFER_DATA_H
#include "build.pb.h"
#include "zType.h"

struct Buffer
{
    //来源类型
    HelloKittyMsgData::BufferSrcType srcType;
    //bufferID
    DWORD id;
    //开始时间
    DWORD beginTime;
    //持续时间
    DWORD lastTime;
    //buffer类型
    HelloKittyMsgData::BufferTypeID bufferType;
    Buffer()
    {
        srcType = HelloKittyMsgData::BST_Default;
        id = 0;
        beginTime = 0;
        lastTime = 0;
        bufferType = HelloKittyMsgData::Buffer_Type_Default;
    }
    void save(HelloKittyMsgData::BufferData &bufferData) const
    {
        bufferData.set_srctype(srcType);
        bufferData.set_bufferid(id);
        bufferData.set_begintime(beginTime);
        bufferData.set_lasttime(lastTime);
        bufferData.set_buffertype(bufferType);
    }
    void load(const HelloKittyMsgData::BufferData &bufferData)
    {
        srcType = bufferData.srctype();
        id = bufferData.bufferid();
        beginTime = bufferData.begintime();
        lastTime = bufferData.lasttime();
        bufferType = bufferData.buffertype();
    }
};

#endif
