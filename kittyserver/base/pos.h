#ifndef POS_H
#define POS_H

#include "zType.h"
#include "kittygarden.pb.h"


struct Point
{
    int x;
    int y;
    Point(const int _x = 0,const int _y = 0) : x(_x),y(_y)
    {
    }
    
    Point(const Point &point) : x(point.x),y(point.y)
    {
    }

    Point(const HelloKittyMsgData::Point &point) : x(point.x()),y(point.y())
    {
    }

    bool save(HelloKittyMsgData::Point *point)
    {
        point->set_x(x);
        point->set_y(y);
        return true;
    }

    Point& operator= (const Point &point)
    {
        x = point.x;
        y = point.y;
        return *this;
    }
    
    bool operator < (const Point &point) const
    {
        if(x < point.x)
        {
            return true;
        }
        if(x == point.x)
        {
            return y < point.y;
        }
        return false;
    }

    bool operator == (const Point &point) const
    {
        return x == point.x && y == point.y;
    }

    bool operator != (const Point &point) const
    {
        return !(*this == point);
    }

    int hashKey() const
    {
        return ((x >> 16) + y);
    }

};
struct InitBuildPoint
{
    DWORD buildType;
    DWORD buildLevel;
    Point point;
};
class AreaGrid
{
    public:
        Point m_point;
        HelloKittyMsgData::AreaGridStatus m_status;
        DWORD m_parseTime;
        DWORD m_lastSec;
        DWORD m_totalSec;
        AreaGrid(const Point &point = Point(),const HelloKittyMsgData::AreaGridStatus status = HelloKittyMsgData::AGS_Close,const DWORD parseTime = 0, const DWORD lastSec = 0,const DWORD totalSec = 0) : m_point(point),m_status(status),m_parseTime(parseTime),m_lastSec(lastSec),m_totalSec(totalSec)
        {
        }
        AreaGrid(const HelloKittyMsgData::AreaGrid &areaGrid) : m_point(areaGrid.point()),m_status(areaGrid.status()),m_parseTime(areaGrid.parsetime()),m_lastSec(areaGrid.lastsec()),m_totalSec(areaGrid.totalsec())
        {
        }
        AreaGrid(const AreaGrid &areaGrid) : m_point(areaGrid.m_point),m_status(areaGrid.m_status),m_parseTime(areaGrid.m_parseTime),m_lastSec(areaGrid.m_lastSec),m_totalSec(areaGrid.m_totalSec)
        {
        }

        bool save(HelloKittyMsgData::AreaGrid *areaGrid)
        {
            if(!areaGrid)
            {
                return false;
            }

            HelloKittyMsgData::Point *point = areaGrid->mutable_point();
            m_point.save(point);
            areaGrid->set_status(m_status);
            areaGrid->set_parsetime(m_parseTime);
            areaGrid->set_lastsec(m_lastSec);
            areaGrid->set_totalsec(m_totalSec);
            return true;
        }
};

class GateGrid
{
    public:
        Point m_point;
        GateGrid(const Point &point = Point()) : m_point(point)
        {
        }
        GateGrid(const HelloKittyMsgData::GateGrid &gateGrid) : m_point(gateGrid.point())
        {
        }
        GateGrid(const GateGrid& gateGrid) : m_point(gateGrid.m_point)
        {
        }

        bool save(HelloKittyMsgData::GateGrid *gateGrid)
        {
            if(!gateGrid)
            {
                return false;
            }

            HelloKittyMsgData::Point *point = gateGrid->mutable_point();
            m_point.save(point);
            return true;
        }
};

#endif

