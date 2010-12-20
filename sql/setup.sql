--
-- first, define the datatype.  Turn off echoing so that expected file
-- does not depend on contents of citext.sql.
--
SET client_min_messages = warning;
\set ECHO none
\i currency.sql
RESET client_min_messages;
\set ECHO all

create table wp_currencies (
       code char(3),
       num int,
       minor int,
       description text
);

\copy wp_currencies from 'data/iso4217.data'

INSERT INTO currency_rate
       (currency_number, currency_code, name, symbol, minor, rate)
VALUES
	(0, 'BTC', 'Bitcoin', '฿', 2, 1);

INSERT INTO currency_rate
       (currency_number, currency_code, name, minor, rate)
SELECT
	num, code, description, minor, (random()+0.5)^3
from
	wp_currencies;

-- some rates for testing
update currency_rate set rate = 3 where currency_code = 'NZD';
update currency_rate set rate = 4 where currency_code = 'USD';
update currency_rate set rate = 6 where currency_code = 'EUR';
update currency_rate set rate = 7 where currency_code = 'GBP';
