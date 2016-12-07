#include <stdio.h>
#include <math.h>
#include <map>
#include <string>
#include <vector>
namespace distance
{
#define PI                      3.1415926
#define EARTH_RADIUS            6378.137        //地球近似半径
#define BASE32 "0123456789bcdefghjkmnpqrstuvwxyz"
    /***********************获取九个的矩形编码****************************************/
    std::map<std::string,std::string> BORDERS;
    std::map<std::string,std::string> NEIGHBORS;
    void setMap()
    {
        NEIGHBORS.insert(std::make_pair("right:even", "bc01fg45238967deuvhjyznpkmstqrwx"));
        NEIGHBORS.insert(std::make_pair("left:even", "238967debc01fg45kmstqrwxuvhjyznp"));
        NEIGHBORS.insert(std::make_pair("top:even", "p0r21436x8zb9dcf5h7kjnmqesgutwvy"));
        NEIGHBORS.insert(std::make_pair("bottom:even", "14365h7k9dcfesgujnmqp0r2twvyx8zb"));

        NEIGHBORS.insert(std::make_pair("right:odd", "p0r21436x8zb9dcf5h7kjnmqesgutwvy"));
        NEIGHBORS.insert(std::make_pair("left:odd", "14365h7k9dcfesgujnmqp0r2twvyx8zb"));
        NEIGHBORS.insert(std::make_pair("top:odd", "bc01fg45238967deuvhjyznpkmstqrwx"));
        NEIGHBORS.insert(std::make_pair("bottom:odd", "238967debc01fg45kmstqrwxuvhjyznp"));

        BORDERS.insert(std::make_pair("right:even", "bcfguvyz"));
        BORDERS.insert(std::make_pair("left:even", "0145hjnp"));
        BORDERS.insert(std::make_pair("top:even", "prxz"));
        BORDERS.insert(std::make_pair("bottom:even", "028b"));

        BORDERS.insert(std::make_pair("right:odd", "prxz"));
        BORDERS.insert(std::make_pair("left:odd", "028b"));
        BORDERS.insert(std::make_pair("top:odd", "bcfguvyz"));
        BORDERS.insert(std::make_pair("bottom:odd", "0145hjnp"));
    }
    /**分别计算每个点的矩形编码
     * @param srcHash
     * @param dir
     * @return
     */
    std::string calculateAdjacent(const std::string &srcHash,const std::string& dir)
    {
        char lastChr = srcHash[srcHash.size()-1];
        int a = srcHash.length()%2;
        std::string type = (a>0)?"odd":"even";
        std::string base(srcHash.begin(),srcHash.begin() + srcHash.size() - 1);
        std::string key = dir + ":";
        key = key + type;
        std::string chstr = BORDERS[key];
        bool bFind = false;
        size_t it = 0;
        for(; it != chstr.size();it++)
        {
            if(chstr[it] == lastChr)
            {
                bFind = true;
                break;
            }

        }
        if (bFind){
            base = calculateAdjacent(base, dir);
        }
        it = 0;
        bFind  = false;
        std::string neistr = NEIGHBORS[key];
        for(; it != neistr.size();it++)
        {
            if(neistr[it] == lastChr)
            {
                base.push_back(BASE32[it]);
                break;
            }

        }
        return base;
    } 

    /**获取九个点的矩形编码
     * @param geohash
     * @return
     */
    void getGeoHashExpand(const std::string geohash,std::vector<std::string> &rvec)
    {
        rvec.push_back(geohash);
        std::string geohashTop = calculateAdjacent(geohash, "top");
        std::string geohashBottom = calculateAdjacent(geohash, "bottom");
        std::string geohashRight = calculateAdjacent(geohash, "right");   
        std::string geohashLeft = calculateAdjacent(geohash, "left"); 
        std::string geohashTopLeft = calculateAdjacent(geohashLeft, "top");  
        std::string geohashTopRight = calculateAdjacent(geohashRight, "top");
        std::string geohashBottomRight = calculateAdjacent(geohashRight, "bottom"); 
        std::string geohashBottomLeft = calculateAdjacent(geohashLeft, "bottom");
        rvec.push_back(geohashTop);
        rvec.push_back(geohashBottom);
        rvec.push_back(geohashRight);
        rvec.push_back(geohashLeft);
        rvec.push_back(geohashTopLeft);
        rvec.push_back(geohashTopRight);
        rvec.push_back(geohashBottomRight);
        rvec.push_back(geohashBottomLeft);

    }
    void encode_geohash(double latitude, double longitude, int precision, std::string &geohash)
    {
        int is_even=1;
        double lat[2], lon[2], mid;
        char bits[] = {16,8,4,2,1};
        int bit=0, ch=0;
        lat[0] = -90.0;
        lat[1] = 90.0; 
        lon[0] = -180.0;
        lon[1] = 180.0;
        while (int(geohash.size()) < precision)
        {
            if (is_even)
            {
                mid = (lon[0] + lon[1]) / 2;
                if (longitude > mid)
                {
                    ch |= bits[bit];
                    lon[0] = mid;
                }
                else
                    lon[1] = mid;
            }
            else
            {
                mid = (lat[0] + lat[1]) / 2;
                if (latitude > mid)
                {
                    ch |= bits[bit];
                    lat[0] = mid;
                }
                else
                    lat[1] = mid;
            }
            is_even = !is_even;
            if (bit < 4)
                bit++;
            else
            {
                geohash.push_back(BASE32[ch]);
                bit = 0;
                ch = 0;
            }
        }
    }


    // 求弧度
    double radian(double d)
    {
        return d * PI / 180.0;   //角度1˚ = π / 180
    }

    //计算距离
    double get_distance(double lat1, double lng1, double lat2, double lng2)
    {
        double radLat1 = radian(lat1);
        double radLat2 = radian(lat2);
        double a = radLat1 - radLat2;
        double b = radian(lng1) - radian(lng2);

        double dst = 2 * asin((sqrt(pow(sin(a / 2), 2) + cos(radLat1) * cos(radLat2) * pow(sin(b / 2), 2) )));

        dst = dst * EARTH_RADIUS;
        dst= round(dst * 10000) / 10000;
        return dst;
    }
}//namespace distance

