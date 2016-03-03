-- Create database and user
-- CREATE DATABASE `gareth_dev`;
-- GRANT ALL PRIVILEGES ON gareth_dev.* TO 'gareth'@'%' identified BY 'gareth';
-- FLUSH PRIVILEGES;
-- USE `gareth_dev`;

DROP TABLE IF EXISTS `g_message`;
DROP TABLE IF EXISTS `g_filter_alert`;
DROP TABLE IF EXISTS `g_filter_clause`;
DROP TABLE IF EXISTS `g_filter`;
DROP TABLE IF EXISTS `g_alert_smtp`;
DROP TABLE IF EXISTS `g_alert_http_header`;
DROP TABLE IF EXISTS `g_alert_http`;

CREATE TABLE `g_alert_http` (
  `ah_id` int(11) NOT NULL AUTO_INCREMENT,
  `ah_name` varchar(64) NOT NULL UNIQUE,
  `ah_method` varchar(16),
  `ah_url` varchar(128) NOT NULL,
  `ah_body` varchar(512),
  PRIMARY KEY (`ah_id`)
);

CREATE TABLE `g_alert_http_header` (
  `ahh_id` int(11) NOT NULL AUTO_INCREMENT,
  `ah_id` int(11) NOT NULL,
  `ahh_key` varchar(64) NOT NULL,
  `ahh_value` varchar(128) DEFAULT NULL,
  PRIMARY KEY (`ahh_id`),
  KEY `ah_id` (`ah_id`),
  CONSTRAINT `alert_http_header_ibfk_1` FOREIGN KEY (`ah_id`) REFERENCES `g_alert_http` (`ah_id`) ON DELETE CASCADE
);

CREATE TABLE `g_alert_smtp` (
  `as_id` int(11) NOT NULL AUTO_INCREMENT,
  `as_name` varchar(64) NOT NULL UNIQUE,
  `as_host` varchar(128) NOT NULL,
  `as_port` int(5) DEFAULT 0 NOT NULL,
  `as_tls` int(1) DEFAULT 0 NOT NULL,
  `as_check_ca` int(1) DEFAULT 1 NOT NULL,
  `as_user` varchar(128) DEFAULT NULL,
  `as_password` varchar(128) DEFAULT NULL,
  `as_from` varchar(128) NOT NULL,
  `as_to` varchar(128) NOT NULL,
  `as_cc` varchar(128) DEFAULT NULL,
  `as_bcc` varchar(128) DEFAULT NULL,
  `as_subject` varchar(128) NOT NULL,
  `as_body` varchar(512) NOT NULL,
  PRIMARY KEY (`as_id`)
);

CREATE TABLE `g_filter` (
  `f_id` int(11) NOT NULL AUTO_INCREMENT,
  `f_name` varchar(64) NOT NULL UNIQUE,
  `f_description` varchar(128),
  PRIMARY KEY (`f_id`)
);

CREATE TABLE `g_filter_clause` (
  `fc_id` int(11) NOT NULL AUTO_INCREMENT,
  `f_id` int(11) NOT NULL,
  `fc_element` int(2) NOT NULL,
  `fc_operator` int(1) DEFAULT 0,
  `fc_value` varchar(128),
  PRIMARY KEY (`fc_id`),
  CONSTRAINT `filter_condition_ibfk_1` FOREIGN KEY (`f_id`) REFERENCES `g_filter` (`f_id`) ON DELETE CASCADE
);

CREATE TABLE `g_filter_alert` (
  `f_id` int(11) NOT NULL,
  `ah_name` varchar(64) NULL,
  `as_name` varchar(64) NULL,
  CONSTRAINT `filter_alert_ibfk_1` FOREIGN KEY (`f_id`) REFERENCES `g_filter` (`f_id`) ON DELETE CASCADE,
  CONSTRAINT `filter_alert_ibfk_2` FOREIGN KEY (`ah_name`) REFERENCES `g_alert_http` (`ah_name`) ON DELETE CASCADE,
  CONSTRAINT `filter_alert_ibfk_3` FOREIGN KEY (`as_name`) REFERENCES `g_alert_smtp` (`as_name`) ON DELETE CASCADE
);

CREATE TABLE `g_message` (
  `m_id` int(11) NOT NULL AUTO_INCREMENT,
  `m_date` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `m_priority` int(11) NOT NULL,
  `m_source` varchar(64) NOT NULL,
  `m_text` varchar(256) NOT NULL,
  `m_tags` varchar(576),
  PRIMARY KEY (`m_id`)
);
