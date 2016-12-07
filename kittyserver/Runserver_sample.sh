#/bin/bash
OLDIFS=$IFS
IFS=' ' 
arr=($(sed -n '3p' ./sql/sqlconfig.xml))
IFS=$OLDIFS
USER=$(echo ${arr[1]} | cut -d \" -f 2)
PASSWD=$(echo ${arr[2]} | cut -d \" -f 2)
Host=$(echo ${arr[3]} | cut -d \" -f 2)
Port=$(echo ${arr[4]} | cut -d \" -f 2)
DATABASE=$(echo ${arr[6]} | cut -d \" -f 2)
TABLE="t_serverlist"
COMMAND="select NAME from ${TABLE}"
declare count=`mysql -h${Host} -P${Port} -u${USER} -p${PASSWD} -D ${DATABASE} -e "${COMMAND}" --skip-column-name`
cond=$(ps -u $(basename $HOME) |grep -v 'redis-server\|Robotserver'|grep -c server)
mysql -h${Host} -P${Port} -u${USER} -p${PASSWD} -D ${DATABASE} < ./sql/dataconfig.sql
ulimit -c unlimited

start()
{
    #先暂停服务器
    while [ $cond -gt 0 ];
	do
		stop
	done

    #建立日志文件夹
    if [ -d log ]
    then
        rm -rf logbak
        mv log logbak
    fi
    mkdir log

    #启动redis
    #echo "redis-server ~/redis.conf"
    #redis-server ~/redis.conf

    #启动登录服务器
    echo "start ./flserver/Flserver"
    ./flserver/Flserver -d

    #启动GM工具服务器
    echo "start ./gmtoolserver/Gmtoolserver"
    ./gmtoolserver/Gmtoolserver -d

    #启动资源工具服务器
    echo "start ./resourceserver/Resourceserver"
    ./resourceserver/Resourceserver -d

    #启动superserver
    for list in $count
    do
        if [ ${list} = "SuperServer" ]
        then
            echo "start ./superserver/Superserver"
            ./superserver/Superserver -d
            sleep 1
            break
        fi
    done

    #启动recordserver
    for list in $count
    do
        if [ ${list} = "RecordServer" ]
        then
            echo "start ./recordserver/Recordserver"
            ./recordserver/Recordserver -d
            sleep 1
        fi
    done

    #启动sceneserver
    for list in $count
    do
        if [ ${list} = "ScenesServer" ]
        then
            echo "start ./sceneserver/SceneServer"
            ./sceneserver/Sceneserver -d
            sleep 1
        fi
    done

    #启动gatewayserver
    for list in $count
    do
        if [ ${list} = "GatewayServer" ]
        then
            echo "start ./gateserver/Gateserver"
            ./gateserver/Gateserver -d
            sleep 1
        fi
    done

    cond=$(ps -u $(basename $HOME) | grep server)
    echo "$cond"
}

stop()
{
    #暂停网关服务器
    sleep 1
	echo "stop ./gateserver/Gateserver"
	pkill Gateserver

    #暂停场景服务器
    sleep 1
	echo "stop ./sceneserver/Sceneserver"
	pkill Sceneserver

    #暂停db服务器
    sleep 1
	echo "stop ./recordserver/Recordserver"
	pkill Recordserver 

    #暂停管理服务器
	sleep 1
	echo "stop ./superserver/Superserver"
	pkill Superserver
	sleep 1

    #先暂停登录服务器
    sleep 1
    echo "stop ./flserver/Flserver"
	pkill Flserver 

    #暂停GM服务器
    sleep 1
	echo "stop ./gmtoolserver/Gmtoolserver"
	pkill Gmtoolserver 

    #暂停资源服务器
    sleep 1
    echo "stop ./resourceserver/Resourceserver"
    pkill Resourceserver

    #暂停redis服务器
    #echo "stop redis-server"
    #pkill redis-server

	while [ $cond -gt 0 ]
	do
		cond=$(ps -u $(basename $HOME) | grep -v 'redis-server\|Robotserver' | grep -c server)
		echo "ServerNum:$cond"
		sleep 1
	done
}

reload()
{
    #暂停网关服务器
    sleep 1
	echo "reload ./gateserver/Gateserver"
	pkill -SIGHUP Gateserver

    #暂停场景服务器
    sleep 1
	echo "reload ./sceneserver/Sceneserver"
	pkill -SIGHUP Sceneserver

    #暂停db服务器
    sleep 1
	echo "reload ./recordserver/Recordserver"
	pkill -SIGHUP Recordserver 

    #暂停管理服务器
	sleep 1
	echo "reload ./superserver/Superserver"
	pkill -SIGHUP Superserver
	sleep 1

    #先暂停登录服务器
    sleep 1
    echo "reload ./flserver/Flserver"
	pkill -SIGHUP Flserver 

    #暂停GM服务器
    sleep 1
	echo "reload ./gmtoolserver/Gmtoolserver"
	pkill -SIGHUP Gmtoolserver 

    #暂停资源服务器
    sleep 1
    echo "reload ./resourceserver/Resourceserver"
    pkill -SIGHUP Resourceserver

}


case $1 in
	start)
	start
	;;
	stop)
	stop
	;;
    debug)
    stop
    start
    ;;
    reload)
    reload
    ;;
	*)
	stop
	start
	;;
esac
