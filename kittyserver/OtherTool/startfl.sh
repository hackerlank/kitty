#/bin/bash

cond=$(ps -u $(basename $HOME) | grep -c Server)

ulimit -c unlimited

start()
{
	rm -rf /home/fl/log/*

	while [ $cond -gt 0 ];
	do
		stop
	done

	echo "start ./flserver/flserver"
	./flserver/flserver -d
	sleep 1
	echo "start ./allzoneserver/allzoneserver"
	./allzoneserver/allzoneserver -d
	sleep 2

	cond=$(ps -u $(basename $HOME) | grep server)
	echo "$cond"
}

stop()
{
	echo "stop ./flserver/flserver"
	pkill flserver
	sleep 1
	echo "stop ./allzoneserver/allzoneserver"
	pkill allzoneserver
	sleep 1
	while [ $cond -gt 0 ];
	do
		cond=$(ps -u $(basename $HOME) | grep -c server)
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
