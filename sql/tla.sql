--
-- test the tla type
--

-- test the in/out functions
select 'EUR'::tla as EUR;
select 'gbp'::tla as GBP;
select 'nzd'::tla as NZD;

-- maxima & minima
select 'btc'::tla as BTC;
select 'xxx'::tla as XXX;

-- casts
select 'thb'::text::tla as THB;
select 'myr'::tla::text as MYR;

-- exceptions
select '0rz'::tla as err_badchar;

-- cast from int2 is illegal
SELECT (0::int2)::tla AS err_nocast;

-- comparison functions - public
SELECT 'chf'::tla = 'CHF'::tla AS t;
SELECT 'sek'::tla <> 'sek'::tla AS f;
SELECT 'RUB'::tla = 'EEK'::tla AS f;
SELECT 'EUR'::tla <> 'USD'::tla AS t;
