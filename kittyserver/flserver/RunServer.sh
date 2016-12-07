#/bin/bash

cond=$(ps -u $(basename $HOME) | grep -c FLServer)

start()
{
	rm /home/fl/log/*

	while [ $cond -gt 0 ];
	do
		stop
	done
	echo "start FLServer/FLServer"
	./FLServer -d
	
	cond=$(ps -u $(basename $HOME) | grep FLServer)
	echo "$cond"
}

stop()
{
	echo "stop ./FLServer/FLServer"
	pkill FLServer
	while [ $cond -gt 0 ];
	do
		cond=$(ps -u $(basename $HOME) | grep -c FLServer)
		echo "ServerNum:$cond"
		sleep 1
	done
}

case $1 in
	start)
	start
	;;
	stop)
	stop
	;;
	*)
	stop
	start
	;;
esac
