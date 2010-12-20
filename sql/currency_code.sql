--
-- test the currency_code type
--

-- test the in/out functions
select 'EUR'::currency_code as EUR;
select 'gbp'::currency_code as GBP;
select 'nzd'::currency_code as NZD;

-- maxima & minima
select 'btc'::currency_code as BTC;
select 'xxx'::currency_code as XXX;

-- casts
select 'thb'::text::currency_code as THB;
select 'myr'::currency_code::text as MYR;

-- exceptions
select '0rz'::currency_code as err_badchar;
select 'NZQ'::currency_code as err_nocode;

-- cast from int2 is illegal
SELECT (0::int2)::currency_code AS err_nocast;

-- comparison functions - public
SELECT 'chf'::currency_code = 'CHF'::currency_code AS t;
SELECT 'sek'::currency_code <> 'sek'::currency_code AS f;
SELECT 'RUB'::currency_code = 'EEK'::currency_code AS f;
SELECT 'EUR'::currency_code <> 'USD'::currency_code AS t;
