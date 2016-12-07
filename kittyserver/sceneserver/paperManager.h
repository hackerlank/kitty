#ifndef PAPER_MANAGER_H
#define PAPER_MANAGER_H
#include <set>
#include "zType.h"
#include "serialize.pb.h"

class SceneUser;
class PaperManager
{
    public:
        PaperManager(SceneUser *owner);
        ~PaperManager();
        //通过道具来判断是否开启图纸
        bool addPaperByItem(const DWORD itemID);
        //刷新所有获得的图纸
        bool flushPaper();
        //加载
        bool load(const HelloKittyMsgData::Serialize& binary);
        //保存
        bool save(HelloKittyMsgData::Serialize& binary);
        //获得已获得的图纸数量
        DWORD getPaperNum();
        //清空所欲获得的图纸
        bool clearPaper();
        //制作图纸
        bool produce(const DWORD paper);
        //重置
        void reset();
    private:
        //增加图纸
        bool addPaper(const DWORD paper);
        //删除某个获得的图纸
        bool delPaper(const DWORD paper);
        //更新单个图纸
        bool updatePaper(const DWORD paper);
        //检查图纸是否已获得
        bool checkPaper(const DWORD paper);
        //制作图纸成功
        bool makePaperSuccess(const DWORD paper);
    private:
        SceneUser *m_owner;
        std::map<DWORD,bool> m_paperMap;
};


#endif

