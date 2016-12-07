DROP TABLE IF EXISTS `account_zone`;
CREATE TABLE IF NOT EXISTS `account_zone` (
  `account` varchar(50) binary NOT NULL DEFAULT '' COMMENT '账号',
  `acctype` BIGINT(20) UNSIGNED default 0 COMMENT '平台',
  `passwd` varchar(100) binary DEFAULT NULL COMMENT '密码',
  PRIMARY KEY (`account` ,`acctype`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='玩家账号信息';

CREATE TABLE IF NOT EXISTS `t_zone` (
  `game` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '游戏ID：预留',
  `zone` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '区编号：预留',
  `zoneType` tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT '区类型：预留',
  `ip` varchar(100) NOT NULL DEFAULT '' COMMENT 'IP',
  `port` smallint(5) unsigned NOT NULL DEFAULT '0' COMMENT '管理服务器Ip',
  `name` varchar(100) NOT NULL DEFAULT '' COMMENT '游戏名：预留',
  `type` tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT '游戏类型：预留',
  `cap` varchar(16) NOT NULL DEFAULT '预留字段' COMMENT '预留字段',
  `x` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '预留字段',
  `y` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '预留字段',
  `desc` varchar(100) NOT NULL DEFAULT '预留字段' COMMENT '预留字段',
  `IsUse` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '预留字段',
  `desc_order` varchar(100) NOT NULL DEFAULT '预留字段' COMMENT '预留字段',
  `destGame` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '预留字段',
  `destZone` int(10) unsigned NOT NULL DEFAULT '0' COMMENT '预留字段',
  `operation` varchar(255) NOT NULL DEFAULT '预留字段' COMMENT '预留字段',
  `flag_test` varchar(100) NOT NULL DEFAULT '预留字段' COMMENT '预留字段',
  PRIMARY KEY (`game`, `zone`),
  UNIQUE KEY `ip_port` (`ip` ,`port`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='服务器区表';

