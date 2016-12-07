#include <string>
#include <map>
#include <vector>

class CommandManager
{
    public:
        static bool dispatchCommand(std::string &command);
        static void init();
        typedef bool (*GmFun)(const std::vector<std::string> &commandVec); 
    public:
        static bool load(const std::vector<std::string> &commandVec);
        static bool visit(const std::vector<std::string> &commandVec);
        static bool showloadInfo(const std::vector<std::string> &commandVec);
        static bool opbuild(const std::vector<std::string> &commandVec);
        static bool chat(const std::vector<std::string> &commandVec);
        static bool gm(const std::vector<std::string> &commandVec);
        static bool leavemsg(const std::vector<std::string> &commandVec);
        static bool loadspecail(const std::vector<std::string> &commandVec);
        static bool gmspecail(const std::vector<std::string> &commandVec);
        static bool loadstep(const std::vector<std::string> &commandVec);
        static bool visitstep(const std::vector<std::string> &commandVec);
        static bool testgeohash(const std::vector<std::string> &commandVec);
        static bool testdistance(const std::vector<std::string> &commandVec);
        static bool testgeohashexpand(const std::vector<std::string> &commandVec);

    private:
        static std::map<std::string,GmFun> s_gmFunMap;
};
