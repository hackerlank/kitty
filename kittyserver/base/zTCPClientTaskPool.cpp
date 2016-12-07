/**
 * \file
 * \version  $Id: zTCPClientTaskPool.cpp 64 2013-04-23 02:05:08Z  $
 * \author  ,@163.com
 * \date 2004年11月18日 14时19分29秒 CST
 * \brief 实现线程池类，用于处理多连接服务器
 *
 * 
 */


#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <assert.h>
#include <ext/pool_allocator.h>

#include "zSocket.h"
#include "zThread.h"
#include "zTCPClientTaskPool.h"
#include "Fir.h"
#include "zTime.h"
#include "zMisc.h"
#include "extractProtoMsg.h"
void callerr(int errct)
{
    if(errct == -1)
    {
        char buf[255];
        sprintf(buf,"%s:",strerror(errno));
        assert(-1 != errct);
    }
}
/**
 * \brief 检测TCP连接状况,如果未连接,尝试连接
 *
 */
class zCheckconnectThread : public zThread
{
    private:
        zTCPClientTaskPool *pool;
    public:
        zCheckconnectThread(
                zTCPClientTaskPool *pool,
                const std::string &name = std::string("zCheckconnectThread"))
            : zThread(name), pool(pool)
        {
        }
        virtual void run()
        {
            while(!isFinal())
            {
                zThread::sleep(4);
                zTime ct;
                pool->timeAction(ct);
            }
        }
};

/**
 * \brief 连接任务链表
 *
 */
//typedef std::list<zTCPClientTask *, __gnu_cxx::__pool_alloc<zTCPClientTask *> > zTCPClientTaskContainer;
typedef std::list<zTCPClientTask * > zTCPClientTaskContainer;
//typedef std::list<zTCPClientTask * > zTCPClientTaskContainer;

/**
 * \brief 连接任务链表叠代器
 *
 */
typedef zTCPClientTaskContainer::iterator zTCPClientTask_IT;

typedef std::vector<struct epoll_event> epollfdContainer;

class zTCPClientTaskQueue
{
    public:
        zTCPClientTaskQueue() :_size(0) {}
        virtual ~zTCPClientTaskQueue() {}
        inline void add(zTCPClientTask *task)
        {
            mlock.lock();
            _queue.push(task);
            ++_size;
            mlock.unlock();
        }
    protected:
        inline void check_queue()
        {
            mlock.lock();
            while(!_queue.empty())
            {
                _temp_queue.push(_queue.front());
                _queue.pop();
            }
            _size = 0;
            mlock.unlock();
            while(!_temp_queue.empty())
            {
                zTCPClientTask *task = _temp_queue.front();
                _temp_queue.pop();
                _add(task);
            }
        }
        virtual void _add(zTCPClientTask *task) = 0;
        unsigned int _size;
    private:
        zMutex mlock;
        //		std::queue<zTCPClientTask *, std::deque<zTCPClientTask *, __gnu_cxx::__pool_alloc<zTCPClientTask *> > > _queue;
        std::queue<zTCPClientTask *, std::deque<zTCPClientTask * > > _queue;
        //		std::queue<zTCPClientTask *, std::deque<zTCPClientTask * > > _queue;
        /**
         * \brief 	缓冲一下需要转交线程的连接
         * 			这样可以保证再锁的外层调用虚函数_add()
         * 			将不再惧怕_add()导致死锁等问题
         *
         */
        //		std::queue<zTCPClientTask *, std::deque<zTCPClientTask *, __gnu_cxx::__pool_alloc<zTCPClientTask *> > > _temp_queue;
        std::queue<zTCPClientTask *, std::deque<zTCPClientTask * > > _temp_queue;
        //		std::queue<zTCPClientTask *, std::deque<zTCPClientTask * > > _temp_queue;
};

/**
 * \brief 处理TCP连接的验证，如果验证不通过，需要回收这个连接
 *
 */
class zCheckwaitThread : public zThread, public zTCPClientTaskQueue
{

    private:

        zTCPClientTaskPool *pool;
        zTCPClientTaskContainer tasks;	/**< 任务列表 */
        zTCPClientTaskContainer::size_type task_count;          /**< tasks计数(保证线程安全*/
        int kdpfd;
        epollfdContainer epfds;

        /**
         * \brief 添加一个连接任务
         * \param task 连接任务
         */
        void _add(zTCPClientTask *task)
        {
            task->addEpoll(kdpfd, EPOLLIN | EPOLLERR | EPOLLPRI, (void *)task);
            tasks.push_back(task);
            task_count = tasks.size();
            if (task_count > epfds.size())
            {
                epfds.resize(task_count + 16);
            }
        }

        void remove(zTCPClientTask *task)
        {
            task->delEpoll(kdpfd, EPOLLIN | EPOLLERR | EPOLLPRI);
            tasks.remove(task);
            task_count = tasks.size();
        }
        void remove(zTCPClientTask_IT &it)
        {
            (*it)->delEpoll(kdpfd, EPOLLIN | EPOLLERR | EPOLLPRI);
            tasks.erase(it);
            task_count = tasks.size();
        }

    public:

        /**
         * \brief 构造函数
         * \param pool 所属的连接池
         * \param name 线程名称
         */
        zCheckwaitThread(
                zTCPClientTaskPool *pool,
                const std::string &name = std::string("zCheckwaitThread"))
            : zThread(name), pool(pool)
        {
            task_count = 0;
            kdpfd = epoll_create(256);
            callerr(kdpfd);
            //assert(-1 != kdpfd);
            epfds.resize(256);
        }

        /**
         * \brief 析构函数
         *
         */
        ~zCheckwaitThread()
        {
            TEMP_FAILURE_RETRY(::close(kdpfd));
        }

        virtual void run();

};

/**
 * \brief 等待接受验证指令，并进行验证
 *
 */
void zCheckwaitThread::run()
{
    zTCPClientTask_IT it, next;

    while(!isFinal())
    {
        check_queue();
        if (!tasks.empty())
        {
            for (it = tasks.begin(), next = it, ++next; it != tasks.end(); it = next, ++next)	
            {
                zTCPClientTask *task = *it;
                if (task->isTerminate())
                {
                    remove(it);
                    //sync -> close
                    task->resetState();
                    task->delFromPool();
                }
            }

            int retcode = epoll_wait(kdpfd, &epfds[0], task_count, 0);
            if (retcode > 0)
            {
                for(int i = 0; i < retcode; ++i)
                {
                    zTCPClientTask *task = (zTCPClientTask *)epfds[i].data.ptr;
                    if (epfds[i].events & (EPOLLERR | EPOLLPRI))
                    {
                        //套接口出现错误
                        Fir::logger->debug("%s(%u): 套接口异常错误, %s", __PRETTY_FUNCTION__, __LINE__, epfds[i].events & EPOLLERR ? "EPOLLERR" : "EPOLLPRI");
                        remove(task);
                        task->resetState();
                    }
                    else if (epfds[i].events & EPOLLIN)
                    {
                        switch(task->checkRebound())
                        {
                            case 1:
                                //验证成功，获取下一个状态
                                remove(task);
                                //tasks.remove(task);
                                if (!pool->addMain(task))
                                {
                                    task->resetState();
                                }
                                break;
                            case 0:
                                //超时，下面会处理
                                break;
                            case -1:
                                //验证失败，回收任务
                                remove(task);
                                task->resetState();
                                break;
                        }
                    }
                }
            }

            // 对验证超时的链接进行回收处理，以防死链接存在
            zTime ct;
            for (it = tasks.begin(), next = it, next++; it != tasks.end(); it = next, next++)
            {
                zTCPClientTask *task = *it;
                if (task->checkStateTimeout(zTCPClientTask::sync, ct, 30))
                {
                    remove(it);
                    task->resetState();
                }
            }
        }

        zThread::msleep(50);
    }

    //把所有等待验证队列中的连接加入到回收队列中，回收这些连接
    for(it = tasks.begin(), next = it, ++next; it != tasks.end(); it = next, ++next)
    {
        zTCPClientTask *task = *it;
        remove(it);
        task->resetState();
    }
}

/**
 * \brief TCP连接的主处理线程，一般一个线程带几个TCP连接，这样可以显著提高效率
 *
 */
class zTCPClientTaskThread : public zThread, public zTCPClientTaskQueue
{

    private:

        zTCPClientTaskPool *pool;
        zTCPClientTaskContainer tasks;	/**< 任务列表 */
        zTCPClientTaskContainer::size_type task_count;          /**< tasks计数(保证线程安全*/
        int kdpfd;
        epollfdContainer epfds;

        /**
         * \brief 添加一个连接任务
         * \param task 连接任务
         */
        void _add(zTCPClientTask *task)
        {
            task->addEpoll(kdpfd, EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLPRI, (void *)task);
            tasks.push_back(task);
            task_count = tasks.size();
            if (task_count > epfds.size())
            {
                epfds.resize(task_count + 16);
            }
        }

        void remove(zTCPClientTask_IT &it)
        {
            (*it)->delEpoll(kdpfd, EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLPRI);
            tasks.erase(it);
            task_count = tasks.size();
        }

    public:

        static const zTCPClientTaskContainer::size_type connPerThread = 256;	/**< 每个线程带的连接数量 */

        /**
         * \brief 构造函数
         * \param pool 所属的连接池
         * \param name 线程名称
         */
        zTCPClientTaskThread(
                zTCPClientTaskPool *pool,
                const std::string &name = std::string("zTCPClientTaskThread"))
            : zThread(name), pool(pool)
        {
            task_count = 0;
            kdpfd = epoll_create(connPerThread);
            callerr(kdpfd);
            //assert(-1 != kdpfd);
            epfds.resize(connPerThread);
        }

        /**
         * \brief 析构函数
         *
         */
        ~zTCPClientTaskThread()
        {
            TEMP_FAILURE_RETRY(::close(kdpfd));
        }

        virtual void run();

        /**
         * \brief 返回连接任务的个数
         * \return 这个线程处理的连接任务数
         */
        const zTCPClientTaskContainer::size_type size() const
        {
            return task_count + _size;
        }

};

/**
 * \brief 主处理线程，回调处理连接的输入输出指令
 *
 */
void zTCPClientTaskThread::run()
{
    zTCPClientTask_IT it, next;

    zRTime currentTime;
    zRTime _1_msec(currentTime), _50_msec(currentTime);

    int kdpfd_r;
    epollfdContainer epfds_r;
    kdpfd_r = epoll_create(256);
    callerr(kdpfd_r);
    //assert(-1 != kdpfd_r);
    epfds.resize(256);
    DWORD fds_count_r = 0;
    //long time = pool->usleep_time;
    bool check=false;
    while(!isFinal())
    {
        currentTime.now();
        if (check) 
        {
            check_queue();
            if (!tasks.empty())
            {
                for(it = tasks.begin(), next = it, ++next; it != tasks.end(); it = next, ++next)
                {
                    zTCPClientTask *task = *it;

                    if (task->isTerminate())
                    {
                        if (task->isFdsrAdd())
                        {
                            task->delEpoll(kdpfd_r, EPOLLIN | EPOLLERR | EPOLLPRI);
                            fds_count_r --;
                            task->fdsrAdd(false); 
                        }
                        remove(it);
                        // state_okay -> state_recycle
                        task->getNextState();
                        task->delFromPool();
                    }
                    else
                    {
                        if (task->checkFirstMainLoop())
                        {
                            //如果是第一次加入处理，需要预先处理缓冲中的数据
                            task->ListeningRecv(false);
                        }
                        if(!task->isFdsrAdd())
                        {
                            task->addEpoll(kdpfd_r, EPOLLIN | EPOLLERR | EPOLLPRI, (void *)task);
                            task->fdsrAdd(true); 
                            ++fds_count_r;
                            if (fds_count_r > epfds_r.size())
                            {
                                epfds_r.resize(fds_count_r + 16);
                            }
                        }
                    }
                }
            }
            check=false;
        }
        zThread::msleep(2);
        if(fds_count_r && _1_msec.elapse(currentTime) >= 2)
        {
            //struct timespec _tv_1;
            //struct timespec _tv_2;
            //clock_gettime(CLOCK_REALTIME, &_tv_1);
            //int retcode = epoll_wait(kdpfd_r, &epfds_r[0], fds_count_r, time/1000);
            int retcode = epoll_wait(kdpfd_r, &epfds_r[0], fds_count_r, 0);
            if (retcode > 0)
            {
                for(int i = 0; i < retcode; ++i)
                {
                    zTCPClientTask *task = (zTCPClientTask *)epfds_r[i].data.ptr;
                    if (epfds_r[i].events & (EPOLLERR | EPOLLPRI))
                    {
                        //套接口出现错误
                        Fir::logger->debug("%s(%u): 套接口异常错误, %s", __PRETTY_FUNCTION__, __LINE__, epfds[i].events & EPOLLERR ? "EPOLLERR" : "EPOLLPRI");
                        task->Terminate(zTCPClientTask::TM_sock_error);
                        check=true;
                    }
                    else
                    {
                        if (epfds_r[i].events & EPOLLIN)
                        {
                            //套接口准备好了读取操作
                            if (!task->ListeningRecv(true))
                            {
                                Fir::logger->debug("%s: 套接口读操作错误", __PRETTY_FUNCTION__);
                                task->Terminate(zTCPClientTask::TM_sock_error);
                                check=true;
                            }
                        }
                    }
                    epfds_r[i].events=0; 
                }
            }
            //clock_gettime(CLOCK_REALTIME, &_tv_2);
            //unsigned long end=_tv_2.tv_sec*1000000L + _tv_2.tv_nsec/1000L;
            //unsigned long begin= _tv_1.tv_sec*1000000L + _tv_1.tv_nsec/1000L;
            //time = time - (end - begin);
        }
        //else
        //{
        //	zThread::usleep(time);
        //	time = 0;
        //}
        if(check)
        {
            //if(time <=0)
            //{
            //	time = 0;
            //}
            continue;
        }

        //if (time <=0)
        if (_50_msec.elapse(currentTime) >= (pool->usleep_time/1000))
        {
            _50_msec = currentTime;
            if (!tasks.empty())
            {
                int retcode = epoll_wait(kdpfd, &epfds[0], task_count, 0);
                if (retcode > 0)
                {
                    for(int i = 0; i < retcode; ++i)
                    {
                        zTCPClientTask *task = (zTCPClientTask *)epfds[i].data.ptr;
                        if (epfds[i].events & (EPOLLERR | EPOLLPRI))
                        {
                            //套接口出现错误
                            Fir::logger->debug("%s(%u): 套接口异常错误, %s", __PRETTY_FUNCTION__, __LINE__, epfds[i].events & EPOLLERR ? "EPOLLERR" : "EPOLLPRI");
                            task->Terminate(zTCPClientTask::TM_sock_error);
                        }
                        else
                        {
                            if (epfds[i].events & EPOLLIN)
                            {
                                //套接口准备好了读取操作
                                if (!task->ListeningRecv(true))
                                {
                                    Fir::logger->debug("%s: 套接口读操作错误", __PRETTY_FUNCTION__);
                                    task->Terminate(zTCPClientTask::TM_sock_error);
                                }
                            }
                            if (epfds[i].events & EPOLLOUT)
                            {
                                //套接口准备好了写入操作
                                if (!task->ListeningSend())
                                {
                                    Fir::logger->debug("%s: 套接口写操作错误", __PRETTY_FUNCTION__);
                                    task->Terminate(zTCPClientTask::TM_sock_error);
                                }
                            }
                        }
                        epfds[i].events=0; 
                    }
                }
                //time = pool->usleep_time;
            }
            check=true;
        }
    }

    //把所有任务队列中的连接加入到回收队列中，回收这些连接
    for(it = tasks.begin(), next = it, ++next; it != tasks.end(); it = next, ++next)
    {
        zTCPClientTask *task = *it;
        remove(it);
        // state_okay -> state_recycle
        task->getNextState();
    }
}



/**
 * \brief 析构函数
 *
 */
zTCPClientTaskPool::~zTCPClientTaskPool()
{
    if (checkconnectThread)
    {
        checkconnectThread->final();
        checkconnectThread->join();
        SAFE_DELETE(checkconnectThread);
    }
    if (checkwaitThread)
    {
        checkwaitThread->final();
        checkwaitThread->join();
        SAFE_DELETE(checkwaitThread);
    }

    taskThreads.joinAll();

    zTCPClientTask_IT it, next;
    for(it = tasks.begin(), next = it, ++next; it != tasks.end(); it = next, ++next)
    {
        zTCPClientTask *task = *it;
        tasks.erase(it);
        SAFE_DELETE(task);
    }
}

zTCPClientTaskThread *zTCPClientTaskPool::newThread()
{
    std::ostringstream name;
    name << "zTCPClientTaskThread[" << taskThreads.size() << "]";
    zTCPClientTaskThread *taskThread = FIR_NEW zTCPClientTaskThread(this, name.str());
    if (NULL == taskThread)
        return NULL;
    if (!taskThread->start())
        return NULL;
    taskThreads.add(taskThread);
    return taskThread;
}

/**
 * \brief 初始化线程池，预先创建各种线程
 *
 * \return 初始化是否成功
 */
bool zTCPClientTaskPool::init()
{
    checkconnectThread = FIR_NEW zCheckconnectThread(this); 
    if (NULL == checkconnectThread)
        return false;
    if (!checkconnectThread->start())
        return false;
    checkwaitThread = FIR_NEW zCheckwaitThread(this);
    if (NULL == checkwaitThread)
        return false;
    if (!checkwaitThread->start())
        return false;

    if (NULL == newThread())
        return false;

    return true;
}

/**
 * \brief 把一个指定任务添加到池中
 * \param task 待添加的任务
 */
bool zTCPClientTaskPool::put(zTCPClientTask *task)
{
    if (task)
    {
        mlock.lock();
        for (zTCPClientTask_IT it = tasks.begin(); it != tasks.end(); ++it)
        {
            zTCPClientTask *temp = *it;
            if(task->getuniqueID() > 0)
            {
                if(task->getuniqueID() == temp->getuniqueID())
                {
                    mlock.unlock();
                    return false;

                }

            }
            else if (temp->ip == task->ip && temp->port == task->port)
            {
                mlock.unlock();
                return false;
            }
        }

        tasks.push_front(task);
        mlock.unlock();
        return true;
    }
    else
        return false;
}

/**
 * \brief 把一个指定任务从池中删除
 * \param task 待删除的任务
 */
void zTCPClientTaskPool::removeFromPool(zTCPClientTask *task)
{
    if(task)
    {
        mlock.lock();
        for (zTCPClientTask_IT it = tasks.begin(); it != tasks.end(); ++it)
        {
            zTCPClientTask *temp = *it;
            bool bfind =false;
            if(task->getuniqueID() > 0)
            {
                if(task->getuniqueID() == temp->getuniqueID())
                {
                    bfind = true;

                }

            }
            else if(temp->ip == task->ip && temp->port == task->port)
            {
                bfind = true;
            }
            if(bfind)
            {
                tasks.erase(it);
                SAFE_DELETE(task);
                mlock.unlock();
                return;

            }
        }
        mlock.unlock();
    }
}

/**
 * \brief 定时执行的任务
 * 主要是如果客户端断线尝试重连
 */
void zTCPClientTaskPool::timeAction(const zTime &ct)
{
    mlock.lock();
    zTCPClientTaskContainer tmp = tasks;
    mlock.unlock();

    zTCPClientTask_IT it, next;
    for (it = tmp.begin(), next = it, ++next; it != tmp.end(); it = next, ++next)
    {
        zTCPClientTask *task = *it;
        switch(task->getState())
        {
            case zTCPClientTask::close:
                if (!task->needDel() && task->checkStateTimeout(zTCPClientTask::close, ct, 4)
                        && task->connect())
                {
                    addCheckwait(task);
                }
                else if (task->needDel())
                {
#ifdef _PQQ_DEBUG
                    Fir::logger->debug("[taskdel] task设置为删除状态后，删除");
#endif
                    removeFromPool(task);
                }

                break;
            case zTCPClientTask::sync:
                break;
            case zTCPClientTask::okay:
                //已经在连接状态，检查测试信号指令
                {
                    if(task->getuniqueID() > 0)
                        break;
                    zRTime now;
                    if (task->checkInterval(now))
                    {
                        if (task->checkTick())
                        {
                            //测试信号在指定时间范围内没有返回
                            Fir::logger->error("套接口检查测试信号失败");
                            task->Terminate(zTCPClientTask::TM_sock_error);
                        }   
                        else
                        {
                            //发送测试信号
                            std::string ret;
                            CMD::t_ClientNullCmd tNullCmd;
                            if(encodeMessage(&tNullCmd,sizeof(tNullCmd),ret) && task->sendCmd(ret.c_str(),ret.size()))
                            {
                                Fir::logger->error("[网络]:%s:%d 发送心跳检测成功", task->ip.c_str(), task->port);
                                task->setTick();
                            }
                        }
                    }
                }
                break;
            case zTCPClientTask::recycle:
                if (task->checkStateTimeout(zTCPClientTask::recycle, ct, 4))
                    task->getNextState();
                break;
        }
    }
}

/**
 * \brief 检查是否需要重新加载配置文件
 *
 */
void zTCPClientTaskPool::resetAll()
{
    mlock.lock();
    for(zTCPClientTask_IT it = tasks.begin(); it != tasks.end(); ++it)
    {
        zTCPClientTask *task = *it;
        switch(task->getState())
        {
            case zTCPClientTask::close:
                break;
            case zTCPClientTask::sync:
                break;
            case zTCPClientTask::okay:
                {
                    task->Terminate(zTCPClientTask::TM_sock_error);
                }
                break;
            case zTCPClientTask::recycle:
                break;
            default:
                break;
        }
    }
    mlock.unlock();

}

/**
 * \brief 关闭所有连接
 *
 */
void zTCPClientTaskPool::terminateAll()
{
    mlock.lock();
    for(zTCPClientTask_IT it = tasks.begin(); it != tasks.end(); ++it)
    {
        zTCPClientTask *task = *it;
        if (task)
            task->Terminate(zTCPClientTask::TM_sock_error);
    }
    mlock.unlock();
}

/**
 * \brief 关闭某个连接
 *
 */
void zTCPClientTaskPool::terminateOne(std::string ip, DWORD port)
{
    mlock.lock();
    for(zTCPClientTask_IT it = tasks.begin(); it != tasks.end(); ++it)
    {
        zTCPClientTask *task = *it;
        if (task && task->ip == ip && task->port == port)
        {
            task->Terminate(zTCPClientTask::TM_sock_error);
        }
    }
    mlock.unlock();
}

/**
 * \brief 检查是否需要重新加载配置文件
 *
 */
bool zTCPClientTaskPool::isReload()
{
    bool ret = true;

    mlock.lock();
    for(zTCPClientTask_IT it = tasks.begin(); it != tasks.end(); ++it)
    {
        zTCPClientTask *task = *it;
        switch(task->getState())
        {
            case zTCPClientTask::close:
                break;
            case zTCPClientTask::sync:
                break;
            case zTCPClientTask::okay:
                //已经在连接状态，发送网络测试信号
                {
                    ret = false;
                }
                break;
            case zTCPClientTask::recycle:
                break;
            default:
                break;
        }
    }
    mlock.unlock();

    return ret;
}

bool zTCPClientTaskPool::isTasksEmpty()
{
    mlock.lock(); 
    bool ret = tasks.empty();	
    mlock.unlock();
    return ret;
}
/**
 * \brief 把任务添加到等待连接认证返回的队列中
 * \param task 待添加的任务
 */
void zTCPClientTaskPool::addCheckwait(zTCPClientTask *task)
{
    checkwaitThread->add(task);
    task->getNextState();
}

/**
 * \brief 把任务添加到主处理循环中
 * \param task 待添加的任务
 * \return 添加是否成功
 */
bool zTCPClientTaskPool::addMain(zTCPClientTask *task)
{
    zTCPClientTaskThread *taskThread = NULL;
    for(unsigned int i = 0; i < taskThreads.size(); ++i)
    {
        zTCPClientTaskThread *tmp = (zTCPClientTaskThread *)taskThreads.getByIndex(i);
        //Fir::logger->debug("%u", tmp->size());
        if (tmp && tmp->size() < connPerThread)
        {
            taskThread = tmp;
            break;
        }
    }
    if (NULL == taskThread)
        taskThread = newThread();
    if (taskThread)
    {
        taskThread->add(task);
        task->getNextState();
        return true;
    }
    else
    {
        Fir::logger->fatal("%s: 不能得到一个空闲线程", __FUNCTION__);
        return false;
    }
}

/** 
 * \brief 设置一个连接为重连状态
 * \prarm ip 这个连接对应的IP
 * \prarm port 这个连接对应的端口
 * \prarm reconn 是否重连 true重连，false 不再重连，将会删掉
 */
void zTCPClientTaskPool::setTaskReconnect(const std::string& ip, unsigned short port, bool reconn)
{
    mlock.lock();
    for (zTCPClientTask_IT it = tasks.begin(); it != tasks.end(); ++it)
    {
        zTCPClientTask* temp = *it;
        if (temp->ip == ip && temp->port == port)
        {
            temp->setDeleteFlag(!reconn);
        }
    }
    mlock.unlock();
}


