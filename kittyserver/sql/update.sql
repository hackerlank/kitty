#yhs   20150-08-05 关系表
CREATE TABLE IF NOT EXISTS relation(
    `playera` BIGINT(20) UNSIGNED,
    `playerb` BIGINT(20) UNSIGNED,
    `relation` BIGINT(20) UNSIGNED,
    PRIMARY KEY(playera,playerb)
)ENGINE=MyISAM DEFAULT CHARSET=utf8;
#flyer 20150-08-07
CREATE TABLE IF NOT EXISTS t_recordbinary(
    `f_allbinary` blob default NULL
);

#flyer   20150-08-17 邮件表
CREATE TABLE IF NOT EXISTS email(
    `sender` BIGINT(20) UNSIGNED,
    `receiver` BIGINT(20) UNSIGNED,
    `time` BIGINT(20) UNSIGNED,
    `conten` blob default NULL, 
    PRIMARY KEY(sender,receiver,time)
);

#flyer 2015-08-28
#ALTER TABLE t_serverlist add DBTABLE varchar(100) NOT NULL;

#yhs 2015-09-06 家族表
CREATE TABLE IF NOT EXISTS `t_family` (
    `f_familyid` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'id',
    `f_charid` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT 'leader',
    `f_name` varchar(50)  NOT NULL DEFAULT '' COMMENT 'family name',
    `f_icon` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'family icon',
    `f_limittype` tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT '0,public 1, no public 2,need requir',
    `f_limitow` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'level limit',
    `f_limithigh` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'level limit',
    `f_level` int(10) unsigned NOT NULL DEFAULT '1' COMMENT 'family level',
    `f_ranking` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'today clock zero get',
    `f_lastranking` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'yesterday clock zero get',
    `f_notice`   varchar(500) NOT NULL DEFAULT '' COMMENT 'family notice',
    `f_createtime`  int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'ctreatetime',
    PRIMARY KEY (`f_familyid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

#yhs 2015-09-08 家族表
CREATE TABLE IF NOT EXISTS `t_familymember` (
    `f_familyid` bigint(20) unsigned NOT NULL DEFAULT '0' ,
    `f_charid` bigint(20) unsigned NOT NULL DEFAULT '0',
    `f_type` tinyint(3)  unsigned NOT NULL DEFAULT '0' COMMENT '0 is member,1 is apply',
    `f_opTimer` int(10)  unsigned NOT NULL DEFAULT '0'  COMMENT 'set time',
    PRIMARY KEY (`f_familyid`,`f_charid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `t_lastflush` (
    `f_id` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT '1 is family rank cal ' ,
    `f_opTimer` int(10)  unsigned NOT NULL DEFAULT '0'  COMMENT 'cal time',
    PRIMARY KEY (`f_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
#yhs  2015-09-16
ALTER TABLE t_family  change `f_limittype`  `f_limittype` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '0,public 1, no public 2,need requir';

#flyer 2015-09-14
ALTER TABLE t_charbase drop f_gold;
ALTER TABLE t_charbase drop f_gem;
ALTER TABLE t_charbase drop f_store_limit;
ALTER TABLE t_charbase drop f_sale_grid_count;

#yhs 2015-09-15 家族表
CREATE TABLE IF NOT EXISTS `t_familymember` (
    `f_familyid` bigint(20) unsigned NOT NULL DEFAULT '0' ,
    `f_charid` bigint(20) unsigned NOT NULL DEFAULT '0',
    `f_type` int(3)  unsigned NOT NULL DEFAULT '0' COMMENT '0 is member,1 is apply',
    `f_opTimer` int(10)  unsigned NOT NULL DEFAULT '0'  COMMENT 'set time',
    PRIMARY KEY (`f_familyid`,`f_charid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

#flyer   20150-10-15 游艺竞拍表
CREATE TABLE IF NOT EXISTS auction(
    `id`  BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
    `charid` BIGINT(20) UNSIGNED,
    `reward` BIGINT(20) UNSIGNED,
    `auctioncnt` BIGINT(20) UNSIGNED,
    PRIMARY KEY(id)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

#flyer   20150-10-23 礼品表
CREATE TABLE IF NOT EXISTS gift(
    `sender` BIGINT(20) UNSIGNED,
    `receiver` BIGINT(20) UNSIGNED,
    `time` BIGINT(20) UNSIGNED,
    `conten` blob default NULL, 
    PRIMARY KEY(sender,receiver,time)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

#flyer   20150-10-26 兑现表
CREATE TABLE IF NOT EXISTS cash(
    `id` BIGINT(20) UNSIGNED,
    `receiver` BIGINT(20) UNSIGNED,
    `time` BIGINT(20) UNSIGNED,
    `status` BIGINT(20) UNSIGNED,
    `acceptername` varchar(20) NOT NULL,
    `address` varchar(100) NOT NULL,
    `phone` varchar(100) NOT NULL,
    `deliverycompany` varchar(100) NOT NULL,
    `deliverynum` varchar(100) NOT NULL,
    `data` blob default NULL,
    PRIMARY KEY(id)
)ENGINE=MyISAM DEFAULT CHARSET=utf8;

#flyer 2015-11-02
ALTER TABLE t_charbase add f_areaType  BIGINT(20) UNSIGNED default 1;

#yhs 2015-11-02  家族订单
ALTER table t_family add `f_score`  int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'score,today clock zero get';
ALTER table t_family add `f_contributionlast`  int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'yesterday contribution';
ALTER table t_familymember add `f_contributionlast` int(10)  unsigned NOT NULL DEFAULT '0'  COMMENT 'yesterday contribution';
ALTER table t_familymember add `f_contributionranklast` int(10)  unsigned NOT NULL DEFAULT '0'  COMMENT 'yesterday contribution rank';
ALTER table t_familymember add `f_isgetaward` int(10)  unsigned NOT NULL DEFAULT '0'  COMMENT 'yesterday award get 1,get';
ALTER table t_familymember  add `f_contribution` int(10)  unsigned NOT NULL DEFAULT '0'  COMMENT 'todayday contribution';
ALTER table t_family add `f_orderlist`  varchar(50)  NOT NULL DEFAULT '' COMMENT 'orderlist';

#flyer 2015-11-03
ALTER TABLE t_charbase add f_exp  BIGINT(20) UNSIGNED default 0;
#yhs  2015-11-05
ALTER table t_charbase add `f_lang` tinyint(3) unsigned NOT NULL DEFAULT '0' after `f_exp`;
#yhs 2015-11-05
CREATE TABLE  servernotice(
    `id` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0',
    `f_lang` INT(10) UNSIGNED NOT NULL DEFAULT '0'  COMMENT 'language',
    `f_time` INT(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'timer',
    `f_notice`   varchar(500) NOT NULL DEFAULT '' COMMENT 'notice',
    `f_adtype` INT(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'adtype',
    PRIMARY KEY(id)
)ENGINE=MyISAM DEFAULT CHARSET=utf8;

#flyer 2015-11-16
ALTER table t_charbase add `f_sushigole` tinyint(3) unsigned NOT NULL DEFAULT '0' after `f_lang`;
#yhs  2015-11-18
ALTER table t_charbase add `f_setname` tinyint(3) unsigned NOT NULL DEFAULT '0' after `f_sushigole`;
ALTER table t_charbase add `f_guideid` int(11) unsigned NOT NULL DEFAULT '0' after `f_setname`;
CREATE TABLE `t_staticnpc` (
    `f_npcid` bigint(20) NOT NULL DEFAULT '0',
    `f_npcbinary` blob,
    PRIMARY KEY (`f_npcid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;
#flyer 2015-11-19
ALTER TABLE t_charbase drop column f_sushigole;
#yhs 2015-11-25
ALTER table t_staticnpc add `f_level` int(11) unsigned NOT NULL DEFAULT '0' after `f_npcid`;
#yhs 1015-11-26
ALTER TABLE t_charbase add UNIQUE  KEY `accountkey`(`f_acctype`,`f_account`);
#yhs 15 12-07
source ./dataconfig.sql

#flyer   20150-11-25 gm服务器器表
CREATE TABLE IF NOT EXISTS t_gmserver(
    `game` BIGINT(20) UNSIGNED,
    `zone` BIGINT(20) UNSIGNED,
    `ip` varchar(100) NOT NULL,
    `port` BIGINT(20) UNSIGNED,
    `name` varchar(20) NOT NULL,
    PRIMARY KEY(game,zone)
)ENGINE=MyISAM DEFAULT CHARSET=utf8;

#flyer   20150-11-25 gm账号表
CREATE TABLE IF NOT EXISTS t_gmadminer(
    `account` varchar(100) NOT NULL,
    `passwd` varchar(100) NOT NULL,
    `des` varchar(20) NOT NULL,
    `permission` BIGINT(20) UNSIGNED,
    PRIMARY KEY(account)
)ENGINE=MyISAM DEFAULT CHARSET=utf8;

#flyer   20150-12-24 封号表
CREATE TABLE IF NOT EXISTS t_forbid(
    `charid` BIGINT(20) UNSIGNED,
    `endtime` BIGINT(20) UNSIGNED,
    `reason`  varchar(100) NOT NULL,
    PRIMARY KEY(charid)
)ENGINE=MyISAM DEFAULT CHARSET=utf8;

#flyer   20150-12-25 禁言表
CREATE TABLE IF NOT EXISTS t_forbidsys(
    `charid` BIGINT(20) UNSIGNED,
    `endtime` BIGINT(20) UNSIGNED,
    `reason`  varchar(100) NOT NULL,
    PRIMARY KEY(charid)
)ENGINE=MyISAM DEFAULT CHARSET=utf8;

#flyer   20160-03-14 礼品库存表
CREATE TABLE IF NOT EXISTS t_giftstore(
    `id` BIGINT(20) UNSIGNED,
    `num` BIGINT(20) UNSIGNED,
    PRIMARY KEY(id)
)ENGINE=MyISAM DEFAULT CHARSET=utf8;

#yhs 2016-05-03 合建表
CREATE TABLE `t_unitybuild` (
    `f_id` bigint(20) NOT NULL DEFAULT '0' COMMENT 'id',
    `f_unitybuildinfo` blob,
    PRIMARY KEY (`f_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

#yhs 2016-06-08 个人活动表
CREATE TABLE `t_playeractive` (
    `f_id` bigint(20) NOT NULL AUTO_INCREMENT,
    `f_begintime` varchar(20) NOT NULL,
    `f_endtime`  varchar(20) NOT NULL,
    `f_condition` INT(10) UNSIGNED NOT NULL DEFAULT '0',
    `f_conditionparam` INT(10) UNSIGNED NOT NULL DEFAULT '0',
    `f_preactive` INT(10) UNSIGNED NOT NULL DEFAULT '0',
    `f_award` varchar(100) NOT NULL  COMMENT '20_20,1_1000,3006_1',
    `f_title` varchar(20) NOT NULL,
    `f_desc`  varchar(100) NOT NULL,
    `f_open`  INT(10)  UNSIGNED NOT NULL DEFAULT '1',
    `f_modifytimer` timestamp NOT NULL DEFAULT current_timestamp,
    PRIMARY KEY (`f_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

#flyer   201606-08-14 资源服务器器表
CREATE TABLE IF NOT EXISTS t_resourceserver(
    `game` BIGINT(20) UNSIGNED,
    `zone` BIGINT(20) UNSIGNED,
    `ip` varchar(100) NOT NULL,
    `port` BIGINT(20) UNSIGNED,
    `name` varchar(20) NOT NULL,
    PRIMARY KEY(game,zone)
)ENGINE=MyISAM DEFAULT CHARSET=utf8;

#flyer 2016-06-20
ALTER TABLE t_charbase add f_gem BIGINT(20) UNSIGNED NOT NULL DEFAULT '0';
ALTER TABLE t_charbase add f_coupons BIGINT(20) UNSIGNED NOT NULL DEFAULT '0'; 
ALTER TABLE t_charbase add f_token BIGINT(20) UNSIGNED NOT NULL DEFAULT '0'; 

#flyer   2016-07-05 GM礼品管理表
CREATE TABLE IF NOT EXISTS t_gift(
    `giftid` BIGINT(20) UNSIGNED NOT NULL,
    `gifttype` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0',
    `pricetype` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0',
    `price` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0',
    `recycletype` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0',
    `recycle` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0',
    `senderprofittype` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0',
    `senderprofit` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0',
    `accepterprofittype` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0',
    `accepterprofit` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0',
    `begintime` BIGINT(10) UNSIGNED NOT NULL DEFAULT '0',
    `endtime` BIGINT(10) UNSIGNED NOT NULL DEFAULT '0',
    `num` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0',
    `sellnum` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0',
    `storenum` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0',
    `adtype` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0',
    `dec` varchar(50) NOT NULL DEFAULT '',
    PRIMARY KEY(giftid)
)ENGINE=MyISAM DEFAULT CHARSET=utf8;

#flyer   2016-07-11 GM礼品获得表
CREATE TABLE IF NOT EXISTS t_getgift(
    `id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
    `charid` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0',
    `nickname` varchar(50) NOT NULL DEFAULT '',
    `gettime` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0',
    `bidtime` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0',
    `gifttype` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0',
    `giftid` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0',
    `giftname` varchar(50) NOT NULL DEFAULT '',
    `giftdec` varchar(50) NOT NULL DEFAULT '',
    `recycletype` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0',
    `recycle` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0',
    PRIMARY KEY(id)
)ENGINE=MyISAM DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS t_bidcenter(
    `id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
    `begintime` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0',
    `endtime` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0',
    PRIMARY KEY(id)
)ENGINE=MyISAM DEFAULT CHARSET=utf8;


#flyer 2016-10-24  活动添加平台信息
ALTER TABLE t_playeractive add f_platemsg varchar(100) NOT NULL; 

#flyer 2016-10-26  活动礼品数量
ALTER TABLE t_playeractive add f_rewardmaxcnt BIGINT(20) UNSIGNED default 0;
ALTER TABLE t_playeractive add f_rewardcurcnt BIGINT(20) UNSIGNED default 0;

#flyer 2016-10-27  激活码
CREATE TABLE  IF NOT EXISTS `t_activecode` (
    `f_key` int(20) unsigned NOT NULL AUTO_INCREMENT COMMENT '激活码id',
    `f_name` varchar(50) binary NOT NULL DEFAULT '' COMMENT '名称',
    `f_acctype` varchar(50) binary NOT NULL DEFAULT '' COMMENT '平台' ,
    `f_overtime` bigint(10) unsigned NOT NULL DEFAULT '0' COMMENT '失效时间',

    `f_code` varchar(50) binary NOT NULL DEFAULT '' COMMENT '激活码',
    `f_allbinary` blob,
    PRIMARY KEY (`f_key`)
)ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='激活码' ;

#flyer 2016-11-01  添加子条件
ALTER TABLE t_playeractive add f_subcondition BIGINT(20) UNSIGNED default 0;

#flyer 2016-11-24  添加激活码类型
ALTER TABLE t_activecode add f_type BIGINT(20) UNSIGNED default 0;


