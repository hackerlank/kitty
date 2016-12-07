#flyer  角色信息
DROP TABLE IF EXISTS `t_charbase`; 
CREATE TABLE  `t_charbase` (
    `f_charid` bigint(20) NOT NULL DEFAULT '0' COMMENT '角色ID' ,
    `f_acctype` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '角色平台' ,
    `f_account` varchar(50) binary NOT NULL DEFAULT '' COMMENT '玩家账户' ,
    `f_nickname` varchar(50) binary NOT NULL DEFAULT '' COMMENT '玩家昵称',
    `f_level` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '玩家等级',
    `f_sex` tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT '玩家性别',
    `f_createtime` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '创角色时间',
    `f_onlinetime` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '最近上线时间',
    `f_offlinetime` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '最近离线时间',
    `f_areaType` int(10) unsigned NOT NULL DEFAULT '1' COMMENT '玩家城市',
    `f_exp` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '玩家经验',
    `f_lang` tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT '玩家语言',
    `f_setname` tinyint(3) unsigned NOT NULL DEFAULT '0'  COMMENT '玩家是否改名',
    `f_guideid` int(11) unsigned NOT NULL DEFAULT '0'  COMMENT '引导位置',
    `f_gem` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0' COMMENT '玩家钻石',
    `f_coupons` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0' COMMENT '玩家点券',
    `f_token` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0' COMMENT '玩家代币',
    `f_allbinary` blob,
    PRIMARY KEY (`f_charid`),
    UNIQUE  KEY `accountkey` (`f_acctype`,`f_account`),
    KEY `f_nickname` (`f_nickname`)
)ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='玩家数据' ;

CREATE TABLE IF NOT EXISTS `t_serverlist`(
    `ID` int(10) unsigned NOT NULL AUTO_INCREMENT COMMENT '服务器ID ',
    `TYPE` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '服务器类型 ',
    `NAME` varchar(32) NOT NULL DEFAULT '' COMMENT '服务器名称 ',
    `IP` varchar(16) NOT NULL DEFAULT '127.0.0.1' COMMENT 'IP ',
    `PORT` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '端口 ',
    `EXTIP` varchar(16) NOT NULL DEFAULT '127.0.0.1' COMMENT '外部IP ',
    `EXTPORT` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '外部端口 ',
    `NETTYPE` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '网络类型 ',
    `DBTABLE` varchar(100) NOT NULL DEFAULT '' COMMENT '相关功能 ',
    PRIMARY KEY (`ID`)
) ENGINE=MyISAM AUTO_INCREMENT=2204 DEFAULT CHARSET=utf8 COMMENT='服务器列表';

DROP TABLE IF EXISTS `t_version`;
CREATE TABLE `t_version` (
    `f_updatetime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP COMMENT '更新时间 ',
    `f_version` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '版本 ',
    KEY `index_time` (`f_updatetime`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='版本管理';

DROP TABLE IF EXISTS `relation`;
CREATE TABLE IF NOT EXISTS relation(
    `playera` BIGINT(20) UNSIGNED COMMENT '玩家A ',
    `playerb` BIGINT(20) UNSIGNED COMMENT '玩家B ',
    `relation` BIGINT(20) UNSIGNED COMMENT '关系 ',
    PRIMARY KEY(playera ,playerb)
)ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='玩家关系列表';
#flyer 20150-08-07
CREATE TABLE IF NOT EXISTS t_recordbinary(
    `f_allbinary` blob default NULL  COMMENT '全服数据 '
)ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='全服数据表';

#flyer   20150-08-17 邮件表
DROP TABLE IF EXISTS `email`;
CREATE TABLE IF NOT EXISTS email(
    `sender` BIGINT(20) UNSIGNED COMMENT '发送者 ',
    `receiver` BIGINT(20) UNSIGNED COMMENT '接受者 ',
    `time` BIGINT(20) UNSIGNED COMMENT '发送时间 ',
    `conten` blob default NULL COMMENT '邮件类容 ', 
    PRIMARY KEY(sender ,receiver,time)
)ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='邮件';

#flyer 2015-08-28
#ALTER TABLE t_serverlist add DBTABLE varchar(100) NOT NULL;

#yhs 2015-09-06 家族表
DROP TABLE IF EXISTS `t_family`;
CREATE TABLE IF NOT EXISTS `t_family` (
    `f_familyid` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT '家庭ID',
    `f_charid` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT '家长' ,
    `f_name` varchar(50)  NOT NULL DEFAULT '' COMMENT '家庭名',
    `f_icon` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '家徽',
    `f_limittype` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '加入条件0,public 1, no public 2,need requir',
    `f_limitow` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '等级需求',
    `f_limithigh` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '等级需求',
    `f_level` int(10) unsigned NOT NULL DEFAULT '1' COMMENT '家庭等级',
    `f_ranking` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '家庭排名',
    `f_lastranking` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '昨日家庭排名',
    `f_notice`   varchar(500) NOT NULL DEFAULT '' COMMENT '家庭公告',
    `f_createtime`  int(10) unsigned NOT NULL DEFAULT '0' COMMENT '创建事件',
    `f_score`  int(10) unsigned NOT NULL DEFAULT '0' COMMENT '家庭积分',
    `f_contributionlast`  int(10) unsigned NOT NULL DEFAULT '0' COMMENT '昨日贡献',
    `f_orderlist`  varchar(50)  NOT NULL DEFAULT '' COMMENT '家庭订单',

    PRIMARY KEY (`f_familyid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='家族表';


DROP TABLE IF EXISTS `t_lastflush`;
CREATE TABLE IF NOT EXISTS `t_lastflush` (
    `f_id` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT '系统类型 ',
    `f_opTimer` int(10)  unsigned NOT NULL DEFAULT '0'  COMMENT '刷新时间' ,
    PRIMARY KEY (`f_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='系统刷新管理';

#yhs 2015-09-15 家族表
DROP TABLE IF EXISTS `t_familymember`;
CREATE TABLE IF NOT EXISTS `t_familymember` (
    `f_familyid` bigint(20) unsigned NOT NULL DEFAULT '0'  COMMENT '家族ID ',
    `f_charid` bigint(20) unsigned NOT NULL DEFAULT '0' COMMENT '玩家ID ',
    `f_type` int(3)  unsigned NOT NULL DEFAULT '0' COMMENT '0 is member ,1 is apply',
    `f_opTimer` int(10)  unsigned NOT NULL DEFAULT '0'  COMMENT '操作时间' ,
    `f_contributionlast` int(10)  unsigned NOT NULL DEFAULT '0'  COMMENT '昨日贡献',
    `f_contributionranklast` int(10)  unsigned NOT NULL DEFAULT '0'  COMMENT '昨日贡献排名',
    `f_isgetaward` int(10)  unsigned NOT NULL DEFAULT '0'  COMMENT '昨日贡献是否领奖 ',
    `f_contribution` int(10)  unsigned NOT NULL DEFAULT '0'  COMMENT '今日贡献',
    PRIMARY KEY (`f_familyid` ,`f_charid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='家族成员表';

#flyer   20150-10-15 游艺竞拍表
DROP TABLE IF EXISTS auction;
CREATE TABLE IF NOT EXISTS auction(
    `id`  BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT COMMENT '竞拍ID ',
    `charid` BIGINT(20) UNSIGNED COMMENT '玩家ID ',
    `reward` BIGINT(20) UNSIGNED COMMENT '奖励 ',
    `auctioncnt` BIGINT(20) UNSIGNED COMMENT '竞拍次数 ',
    PRIMARY KEY(id)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='游艺竞拍表';

#flyer   20150-10-23 礼品表
DROP TABLE IF EXISTS gift;
CREATE TABLE IF NOT EXISTS gift(
    `sender` BIGINT(20) UNSIGNED COMMENT '发送者 ',
    `receiver` BIGINT(20) UNSIGNED COMMENT '接受者 ',
    `time` BIGINT(20) UNSIGNED COMMENT '发送时间 ',
    `conten` blob default NULL COMMENT '发送礼品 ', 
    PRIMARY KEY(sender ,receiver ,time)
)ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='礼品表';


#flyer   20150-10-26 兑现表
DROP TABLE IF EXISTS cash;
CREATE TABLE IF NOT EXISTS cash(
    `id` BIGINT(20) UNSIGNED COMMENT '订单id ',
    `receiver` BIGINT(20) UNSIGNED COMMENT '接受者id ',
    `time` BIGINT(20) UNSIGNED COMMENT ' 订单生成时间',
    `status` BIGINT(20) UNSIGNED COMMENT ' 订单状态',
    `acceptername` varchar(20) NOT NULL COMMENT ' 订单接收者昵称',
    `address` varchar(100) NOT NULL COMMENT ' 地址',
    `phone` varchar(100) NOT NULL COMMENT ' 手机号',
    `deliverycompany` varchar(100) NOT NULL COMMENT ' 快递公司',
    `deliverynum` varchar(100) NOT NULL COMMENT ' 快递单号',
    PRIMARY KEY(id)
)ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='';

#yhs
CREATE TABLE IF NOT EXISTS servernotice(
    `id` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0' COMMENT ' ',
    `f_lang` INT(10) UNSIGNED NOT NULL DEFAULT '0'  COMMENT 'language' ,
    `f_time` INT(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'timer' ,
    `f_notice`   varchar(500) NOT NULL DEFAULT '' COMMENT 'notice' ,
    `f_adtype` INT(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT 'adtype',
    PRIMARY KEY(id)
)ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='预设系统消息';
#yhs 2015-11-19
DROP TABLE IF EXISTS `t_staticnpc`;
CREATE TABLE `t_staticnpc` (
    `f_npcid` bigint(20) NOT NULL DEFAULT '0' COMMENT 'npcid ',
    `f_level` int(11) unsigned NOT NULL DEFAULT '0' COMMENT 'npc等级 ',
    `f_npcbinary` blob COMMENT '静态NPC数据 ',
    PRIMARY KEY (`f_npcid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='静态Npc';
source ./dataconfig.sql

#flyer   20150-11-25 gm服务器器表
CREATE TABLE IF NOT EXISTS t_gmserver(
    `game` BIGINT(20) UNSIGNED COMMENT '游戏编号',
    `zone` BIGINT(20) UNSIGNED COMMENT ' 区号',
    `ip` varchar(100) NOT NULL COMMENT ' 管理服务器ip',
    `port` BIGINT(20) UNSIGNED COMMENT ' 管理服务器端口',
    `name` varchar(20) NOT NULL COMMENT ' 区名',
    PRIMARY KEY(game,zone)
)ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='gm服务器器表';

#flyer   20150-11-25 gm账号表
DROP TABLE IF EXISTS t_gmadminer;
CREATE TABLE IF NOT EXISTS t_gmadminer(
    `account` varchar(100) NOT NULL COMMENT '账号 ',
    `passwd` varchar(100) NOT NULL COMMENT '密码 ',
    `des` varchar(20) NOT NULL COMMENT ' 描述',
    `permission` BIGINT(20) UNSIGNED COMMENT '权限 ',
    PRIMARY KEY(account)
)ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='gm账号表';

#flyer   20150-12-24 封号表
DROP TABLE IF EXISTS t_forbid;
CREATE TABLE IF NOT EXISTS t_forbid(
    `charid` BIGINT(20) UNSIGNED COMMENT ' 角色ID',
    `endtime` BIGINT(20) UNSIGNED COMMENT ' 结束时间',
    `reason`  varchar(100) NOT NULL COMMENT ' 原因',
    PRIMARY KEY(charid)
)ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='封号表';

#flyer   20150-12-25 禁言表
DROP TABLE IF EXISTS t_forbidsys;
CREATE TABLE IF NOT EXISTS t_forbidsys(
    `charid` BIGINT(20) UNSIGNED COMMENT ' 角色ID',
    `endtime` BIGINT(20) UNSIGNED COMMENT ' 结束时间',
    `reason`  varchar(100) NOT NULL COMMENT ' 原因',
    PRIMARY KEY(charid)
)ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='禁言表';

#yhs 2016-05-03 合建表
DROP TABLE IF EXISTS `t_unitybuild`;
CREATE TABLE IF NOT EXISTS `t_unitybuild` (
    `f_id` bigint(20) NOT NULL DEFAULT '0' COMMENT 'id' COMMENT '合建ID ',
    `f_unitybuildinfo` blob COMMENT ' 合建详情',
    PRIMARY KEY (`f_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='合建表';

#yhs 2016-06-08 个人活动表
CREATE TABLE IF NOT EXISTS `t_playeractive` (
    `f_type` bigint(20) NOT NULL COMMENT '活动类型 ',
    `f_subtype` bigint(20) NOT NULL COMMENT '活动子类型 ',
    `f_id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT '活动ID ',
    `f_begintime` INT(10) UNSIGNED NOT NULL COMMENT '开始时间 ',
    `f_endtime`  INT(10) UNSIGNED NOT NULL COMMENT '结束时间 ',
    `f_condition` INT(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '条件 ',
    `f_conditionparam` INT(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '条件参数 ',
    `f_preactive` INT(10) UNSIGNED NOT NULL DEFAULT '0' COMMENT '前置活动 ',
    `f_award` varchar(100) NOT NULL  COMMENT '20_20 ,1_1000 ' ,
    `f_title` varchar(20) NOT NULL COMMENT '标题 ',
    `f_desc`  varchar(100) NOT NULL COMMENT '内容 ',
    `f_open`  INT(10)  UNSIGNED NOT NULL DEFAULT '1' COMMENT '是否开启 ',
    `f_platemsg` varchar(100) NOT NULL  COMMENT '平台id_商品id ' ,
    `f_rewardmaxcnt` bigint(20) NOT NULL COMMENT '礼品最大数量',
    `f_rewardcurcnt` bigint(20) NOT NULL COMMENT '礼品已领取数量 ',
    `f_subcondition` bigint(20) NOT NULL COMMENT '子条件',
    `f_modifytimer` timestamp NOT NULL DEFAULT current_timestamp COMMENT '更改时间 ',
    PRIMARY KEY (`f_id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='';

#flyer   201606-08-14 资源服务器器表
CREATE TABLE IF NOT EXISTS t_resourceserver(
    `game` BIGINT(20) UNSIGNED COMMENT '游戏ID ',
    `zone` BIGINT(20) UNSIGNED COMMENT ' 区号',
    `ip` varchar(100) NOT NULL COMMENT ' 管理服务器ip',
    `port` BIGINT(20) UNSIGNED COMMENT ' 管理服务器端口',
    `name` varchar(20) NOT NULL COMMENT ' 区名',
    PRIMARY KEY(game,zone)
)ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='资源服务器器表';

#flyer   2016-07-05 GM礼品管理表
DROP TABLE IF EXISTS t_gift;
CREATE TABLE IF NOT EXISTS t_gift(
    `giftid` BIGINT(20) UNSIGNED NOT NULL COMMENT '礼品id',
    `gifttype` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0' COMMENT '礼品类型',
    `pricetype` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0' comment '价格类型',
    `price` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0' comment '价格',
    `recycletype` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0' comment '回收价格类型',
    `recycle` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0' comment '回收价格',
    `senderprofittype` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0' comment '赠送者收益类型',
    `senderprofit` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0' comment '赠送者收益',
    `accepterprofittype` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0' comment '接受者收益类型',
    `accepterprofit` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0' comment '接受者收益',
    `begintime` BIGINT(10) UNSIGNED NOT NULL DEFAULT '0' comment '开始时间',
    `endtime` BIGINT(10) UNSIGNED NOT NULL DEFAULT '0' comment '结束时间',
    `num` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0' comment '总数',
    `sellnum` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0' comment '已卖出数量',
    `storenum` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0' comment '库存数量',
    `adtype` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0' comment '广告类型',
    `dec` varchar(50) NOT NULL DEFAULT '' comment '描述',
    PRIMARY KEY(giftid)
)ENGINE=MyISAM DEFAULT CHARSET=utf8;

#flyer   2016-07-11 GM礼品获得表
DROP TABLE IF EXISTS t_getgift;
CREATE TABLE IF NOT EXISTS t_getgift(
    `id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
    `charid` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0' comment '归属者ID',
    `nickname` varchar(50) NOT NULL DEFAULT '' comment '归属者昵称',
    `gettime` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0' comment '获得时间',
    `bidtime` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0' comment '竞拍次数',
    `gifttype` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0' comment '礼物类型',
    `giftid` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0' comment '礼物id',
    `giftname` varchar(50) NOT NULL DEFAULT '' comment '礼物名称',
    `giftdec` varchar(50) NOT NULL DEFAULT '' comment '礼物描述',
    `recycletype` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0' comment '回收价格类型',
    `recycle` BIGINT(4) UNSIGNED NOT NULL DEFAULT '0' comment '回收价格',
    PRIMARY KEY(id)
)ENGINE=MyISAM DEFAULT CHARSET=utf8;


#flyer 2016-09-30
DROP TABLE IF EXISTS t_bidcenter;
CREATE TABLE IF NOT EXISTS t_bidcenter(
    `id` BIGINT(20) UNSIGNED NOT NULL AUTO_INCREMENT,
    `begintime` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0',
    `endtime` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0',
    PRIMARY KEY(id)
)ENGINE=MyISAM DEFAULT CHARSET=utf8;

#flyer 2016-10-27  激活码
DROP TABLE IF EXISTS `t_activecode`; 
CREATE TABLE  `t_activecode` (
    `f_key` int(20) unsigned NOT NULL AUTO_INCREMENT COMMENT '激活码id',
    `f_name` varchar(50) binary NOT NULL DEFAULT '' COMMENT '名称',
    `f_acctype` varchar(50) binary NOT NULL DEFAULT '' COMMENT '平台' ,
    `f_type` BIGINT(20) UNSIGNED NOT NULL DEFAULT '0', commit '激活码类型',
    `f_overtime` bigint(10) unsigned NOT NULL DEFAULT '0' COMMENT '失效时间',
    `f_code` varchar(50) binary NOT NULL DEFAULT '' COMMENT '激活码',
    `f_allbinary` blob,
    PRIMARY KEY (`f_key`)
)ENGINE=InnoDB DEFAULT CHARSET=utf8 COMMENT='激活码' ;


