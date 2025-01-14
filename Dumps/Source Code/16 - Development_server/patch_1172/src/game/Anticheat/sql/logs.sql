# Logs database

DROP TABLE IF EXISTS `logs_anticheat`;

CREATE TABLE `logs_anticheat` (
  `id` INT(10) UNSIGNED NOT NULL AUTO_INCREMENT,
  `time` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `realm` INT(10) UNSIGNED NOT NULL,
  `account` INT(10) UNSIGNED NOT NULL,
  `ip` VARCHAR(16) NOT NULL,
  `fingerprint` int(10) unsigned NOT NULL,
  `actionMask` INT(10) UNSIGNED DEFAULT NULL,
  `player` VARCHAR(32) NOT NULL,
  `info` VARCHAR(512) NOT NULL,
  PRIMARY KEY (`id`),
  KEY `account` (`account`),
  KEY `ip` (`ip`),
  KEY `time` (`time`),
  KEY `realm` (`realm`)
) ENGINE=INNODB AUTO_INCREMENT=34 DEFAULT CHARSET=utf8;

DROP TABLE IF EXISTS `logs_spamdetect`;

CREATE TABLE `logs_spamdetect` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `realm` int(10) unsigned NOT NULL,
  `accountId` int(11) DEFAULT '0',
  `fromIP` varchar(16) NOT NULL,
  `fromFingerprint` int(10) unsigned NOT NULL,
  `comment` varchar(8192) NOT NULL DEFAULT '',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;