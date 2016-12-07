#!/bin/bash
svn update dataconfig.sql
mysqldump -t  yhsdata -h10.0.2.5 -P3306 -udebug -pdebug --tables t_staticnpc --skip-comments --skip-disable-keys --skip-set-charset --skip-tz-utc  --skip-lock-tables > dataconfig.sql
#mysqldump -t  yhsdata -h10.0.2.5 -P3306 -udebug -pdebug  --tables t_charbase --where="f_charid <= 100" --skip-comments --skip-disable-keys --skip-set-charset --skip-tz-utc  --skip-lock-tables >> dataconfig.sql
sed -i '/\/\*\!40/d' dataconfig.sql
sed -i  's/INSERT INTO/REPLACE INTO/g' dataconfig.sql
svn commit  dataconfig.sql -m"update dataconfig" 

