#!/bin/sh
cp config/redis_config.xml.example config/redis_config.xml
ip=`sh ./localip.sh`
sed -i "s/172.17.116.14/$ip/" config/redis_config.xml
