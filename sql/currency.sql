-- simple creation etc - value rendered is passed back as it came in
select '100 eur'::currency as "100 eur";
select '-100 eur'::currency as "-100 eur";

select '100.00 eur'::currency as "100.00 eur";
select '-100.00 eur'::currency as "-100.00 eur";

select '100eur'::currency as "100 eur";

-- these values make no sense, but who cares :)
select '-100.005 eur'::currency as "-100.005 eur";
select '-100.006 eur'::currency as "-100.006 eur";

-- formatting for display requires the currency_rates table
select format('100 nzd'::currency) as "NZD 100.00";
select format('-100.006 eur'::currency) as "€ -100.01";
select #('123.456 gbp'::currency) as "£ 123.46";

-- test conversion
select change('100 nzd'::currency, 'btc') as "300.00 BTC";
select format(change('100 nzd'::currency, 'gbp')) as "£ 42.86";
select #('123.456 usd'::currency->'eur') as "€ 82.30";

-- code and value
select code('100nzd'::currency) as "NZD";
select value('100nzd'::currency) as "100";

-- and the other way around
select currency('20', 'nzd') as "20 NZD";
select ('20'||'nzd')::currency as "20 NZD";

-- the below might be appropriate if 'tla' were actually a currency
-- code type, but if it's just a generic three letters, it makes no
-- sense.
select 20 * 'nzd'::tla as ERROR;

-- casting to money : locale dependent test (FIXME)
select currency('20', 'nzd')::money as "$60.00";

-- test comparisons
select '100 nzd'::currency = '100 nzd'::currency as t;
select '100 nzd'::currency = '300 btc'::currency as t;
select '100 nzd'::currency =  '50 eur'::currency as t;
select '100 nzd'::currency = '101 nzd'::currency as f;
select '100 nzd'::currency = '301 btc'::currency as f;
select '100 nzd'::currency =  '51 eur'::currency as f;

select '100 nzd'::currency = '100 nzd'::currency as t;
select '300 btc'::currency = '100 nzd'::currency as t;
select '50 eur'::currency  = '100 nzd'::currency as t;
select '101 nzd'::currency = '100 nzd'::currency as f;
select '301 btc'::currency = '100 nzd'::currency as f;
select '51 eur'::currency  = '100 nzd'::currency as f;

-- !=
select '100 nzd'::currency != '100 nzd'::currency as f;
select '100 nzd'::currency != '300 btc'::currency as f;
select '100 nzd'::currency !=  '50 eur'::currency as f;
select '100 nzd'::currency != '101 nzd'::currency as t;
select '100 nzd'::currency != '301 btc'::currency as t;
select '100 nzd'::currency !=  '51 eur'::currency as t;

-- don't bother with all cases for the other operators
select '60 usd'::currency = '40 eur'::currency as t;
select '61 usd'::currency = '40 eur'::currency as f;
select '60 usd'::currency = '41 eur'::currency as f;

select '60 usd'::currency != '40 eur'::currency as f;
select '61 usd'::currency != '40 eur'::currency as t;
select '60 usd'::currency != '41 eur'::currency as t;

select '60 usd'::currency < '40 eur'::currency as f;
select '61 usd'::currency < '40 eur'::currency as f;
select '60 usd'::currency < '41 eur'::currency as t;

select '60 usd'::currency <= '40 eur'::currency as t;
select '61 usd'::currency <= '40 eur'::currency as f;
select '60 usd'::currency <= '41 eur'::currency as t;

select '60 usd'::currency > '40 eur'::currency as f;
select '61 usd'::currency > '40 eur'::currency as t;
select '60 usd'::currency > '41 eur'::currency as f;

select '60 usd'::currency >= '40 eur'::currency as t;
select '61 usd'::currency >= '40 eur'::currency as t;
select '60 usd'::currency >= '41 eur'::currency as f;

create table amounts (
       description text,
       value currency
);

\copy amounts from 'data/amounts.data'
-- test sorting
--select max(value) from amounts;
select description, value from amounts order by value desc limit 3;
select description, value from amounts order by value asc limit 3;

-- check the hashing function (which is not immutable)
select hash_currency('100 eur'::currency) = hash_currency('100 eur'::currency) as t;
select hash_currency('100 eur'::currency) != hash_currency('101 eur'::currency) as t;
select hash_currency('100 eur'::currency) != hash_currency('100 nzd'::currency) as t;
select hash_currency('40 eur'::currency) = hash_currency('60 usd'::currency) as t;
select hash_currency('100 eur'::currency) = hash_currency('100.00 eur'::currency) as t;
