#include  "wordFilter.h"
#include  "zMisc.h"
#define MAX_CHATINFO    256
/**
 * \brief 读取禁言过滤表格
 *
 *
 */
void wordFilter::init()
{
    //禁止词汇

    std::string f = Fir::global["forbidWordsFile"];
    if (""==f)
        f = "forbidWords";
    std::ifstream src(f.c_str());
    if (!src)
    {
        Fir::logger->warn("打开词汇过滤列表失败！file=%s", Fir::global["forbidWordsFile"].c_str());
        return;
    }

    char buf[256];
    bzero(buf,sizeof(buf));
    std::vector<std::string> vs;
    std::vector<std::string>::iterator word,replacer;
    while (src.getline(buf, sizeof(buf)))
    {
        vs.clear();
        Fir::stringtok(vs, buf, " \t\r");
        if (vs.size()==3)
        {
            replacer = vs.begin();
            replacer++;
            word = replacer++;

            //转换成小写
            bzero(buf,sizeof(buf));
            word->copy(buf, word->length(), 0);
            for (unsigned int i=0; i<word->length() && i<(count_of(buf)-1); i++)
                buf[i] = tolower(buf[i]);
            word->assign(buf);

            forbidWords[*word] = *replacer;
            Fir::logger->debug("%lu\t%s\t%s", forbidWords.size(), (*word).c_str(), (*replacer).c_str());
        }
    }

    Fir::logger->debug("加载词汇过滤列表 %lu", forbidWords.size());
}

/**
 * \brief 禁言过虑
 *
 *
 * \param text: 过虑内容
 * \param len: 内容长度
 * \return 
 */
bool wordFilter::doFilter(char *text, unsigned int len)
{
    //进行词汇过滤
    zRegex regex;
    bool ret = true;

    char copy[MAX_CHATINFO+1];
    bzero(copy, sizeof(copy));
    strncpy(copy, text, MAX_CHATINFO);
    for (unsigned int i=0; i<strlen(copy) && i<(count_of(copy)-1); i++)
        copy[i] = tolower(copy[i]);

    std::string content(text);//原始字符
    std::string content_copy(copy);//小于字符
    std::string::size_type pos=0;

    for (std::map<std::string, std::string>::iterator it=forbidWords.begin(); it!=forbidWords.end(); ++it)//遍历屏蔽字库
    {
        pos = content_copy.find(it->first.c_str(), 0);//如果找到屏蔽字位置
        while (pos!=std::string::npos)
        {
            content.replace(pos, it->first.length(), it->second.c_str());
            content_copy.replace(pos, it->first.length(), it->second.c_str());
            pos = content_copy.find(it->first.c_str(), pos+it->first.length());
            ret = false;
        }
    }

    strncpy(text, content.c_str(), len);//目前恒等于16,还算安全
    return ret;
}

bool wordFilter::doFilter(std::string & strstring) 
{
    char copy[MAX_CHATINFO+1];
    bzero(copy, sizeof(copy));
    strncpy(copy, strstring.c_str(), MAX_CHATINFO);
    bool ret=  doFilter(copy,strstring.size());
    strstring =std::string(copy);
    return ret;

}

bool wordFilter::hasForbitWord(const char *text)
{
    //进行词汇过滤
    char copy[MAX_CHATINFO+1];
    bzero(copy, sizeof(copy));
    strncpy(copy, text, MAX_CHATINFO);
    for (unsigned int i=0; i<strlen(copy) && i<(count_of(copy)-1); i++)
        copy[i] = tolower(copy[i]);
    std::string strcopy(copy);

    std::string::size_type pos=0;
    for (std::map<std::string, std::string>::iterator it=forbidWords.begin(); it!=forbidWords.end(); ++it)//遍历屏蔽字库
    {
        pos = strcopy.find(it->first.c_str(), 0);//如果找到屏蔽字位置
        if(pos != std::string::npos)
        {
            return  true;
        }
    }
    return false;

}
