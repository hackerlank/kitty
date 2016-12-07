#!/bin/bash
if [ ! $# == 1 ];then
    echo "need role account ,plat"
    exit
fi
charid="$1"
file=Role_$1.sql
mysqldump -t  yhsdata -h10.0.2.5 -P3306 -udebug -pdebug --tables t_charbase --skip-comments --skip-disable-keys --skip-set-charset --skip-tz-utc  --skip-lock-tables --where 'f_charid='${charid}''  > $file
sed -i '/\/\*\!40/d' $file

