#/bin/bash

cond=$(ps -u $(basename $HOME) | grep -c server)

ulimit -c unlimited

start()
{
	while [ $cond -gt 0 ];
	do
		stop
	done

    removelog
    sleep 1
    echo "redis-server ~/redis.conf"
    redis-server ~/redis.conf
    sleep 1
	echo "start ./gmtoolserver/Gmtoolserver"
	./gmtoolserver/Gmtoolserver -d
	sleep 1
	echo "start ./superserver/Superserver"
	./superserver/Superserver -d
    sleep 1
	echo "start ./recordserver/Recordserver"
	./recordserver/Recordserver -d
    sleep 1
	echo "start ./sceneserver/SceneServer"
    ./sceneserver/Sceneserver -d 
	sleep 1
	./sceneserver/Sceneserver -d 
	echo "start ./gateserver/Gateserver"
	./gateserver/Gateserver -d 
	sleep 1
	./gateserver/Gateserver -d 
	sleep 2

	cond=$(ps -u $(basename $HOME) | grep server)
	echo "$cond"
}

function removelog()
{
    if [ -d log ]
    then
        rm -rf log/superserver*.*
        rm -rf log/billgatewayserver*.*
        rm -rf log/gatewayserver*.*
        rm -rf log/recordserver*.*
        rm -rf log/sceneserver*.*
        rm -rf log/billsuperserver*.*
        rm -rf log/recharge_superserver*.*
    else
        mkdir log
    fi
}

stop()
{
    sleep 1
	echo "stop ./gmtoolserver/Gmtoolserver"
	pkill Gmtoolserver 
	sleep 1
	echo "stop ./superserver/Superserver"
	pkill Superserver
	sleep 1
	echo "stop ./recordserver/Recordserver"
	pkill Recordserver 
	sleep 1
	echo "stop ./sceneserver/Sceneserver"
	pkill Sceneserver
	sleep 1
	echo "stop ./gateserver/Gateserver"
	pkill Gateserver
    sleep 1
    echo "stop redis-server"
    pkill redis-server
	while [ $cond -gt 0 ]
	do
		cond=$(ps -u $(basename $HOME) | grep -c server)
		echo "ServerNum:$cond"
		sleep 1
	done
}

function stopflserver()
{
    sleep 1
    echo "stop ./flserver/Flserver"
    pkill Flserver
    
    local cond=$(ps -u $(basename $HOME) | grep -c Flserver)
    
    while [ $cond -gt 0 ]
	do
		cond=$(ps -u $(basename $HOME) | grep -c Flserver)
		echo "flserverNum:$cond"
		sleep 1
	done
    rm -rf log
}


case $1 in
	start)
	start
	;;
	stop)
	stop
	;;
    debug)
    stopflserver
    stop
    sleep 1
    mkdir log
	echo "start ./flserver/Flserver"
	./flserver/Flserver -d
    start
    ;;
	*)
	stop
	start
	;;
esac
