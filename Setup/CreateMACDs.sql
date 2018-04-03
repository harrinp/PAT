CREATE TABLE `MACD_EUR_USD_D` (
  `date` int(10) unsigned NOT NULL,
  `EMA26` decimal(8,5) DEFAULT NULL,
  `EMA12` decimal(8,5) DEFAULT NULL,
  `MACD` decimal(8,5) DEFAULT NULL,
  `sign` decimal(8,5) DEFAULT NULL,
  `result` decimal(8,5) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `MACD_EUR_USD_M1` (
  `date` int(10) unsigned NOT NULL,
  `EMA26` decimal(8,5) DEFAULT NULL,
  `EMA12` decimal(8,5) DEFAULT NULL,
  `MACD` decimal(8,5) DEFAULT NULL,
  `sign` decimal(8,5) DEFAULT NULL,
  `result` decimal(8,5) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

CREATE TABLE `MACD_EUR_USD_H1` (
  `date` int(10) unsigned NOT NULL,
  `EMA26` decimal(8,5) DEFAULT NULL,
  `EMA12` decimal(8,5) DEFAULT NULL,
  `MACD` decimal(8,5) DEFAULT NULL,
  `sign` decimal(8,5) DEFAULT NULL,
  `result` decimal(8,5) DEFAULT NULL
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
