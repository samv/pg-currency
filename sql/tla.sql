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

-- comparison functions - only for b-tree use
SELECT 'OWL'::tla #<# 'OWL'::tla AS f;
SELECT 'OWL'::tla #<=# 'OWL'::tla AS t;
SELECT 'OWL'::tla #>=# 'OWL'::tla AS t;
SELECT 'OWL'::tla #># 'OWL'::tla AS f;

SELECT 'YAK'::tla #<# 'ELK'::tla AS f;
SELECT 'YAK'::tla #<=# 'ELK'::tla AS f;
SELECT 'YAK'::tla #>=# 'ELK'::tla AS t;
SELECT 'YAK'::tla #># 'ELK'::tla AS t;

SELECT 'ELK'::tla #<# 'YAK'::tla AS t;
SELECT 'ELK'::tla #<=# 'YAK'::tla AS t;
SELECT 'ELK'::tla #>=# 'YAK'::tla AS f;
SELECT 'ELK'::tla #># 'YAK'::tla AS f;

-- create some tables, testing the indexing capability
create table scrabble (
       word tla,
       primary key (word),
       score integer null
);

create table twl98 (
       primary key (word)
) inherits (scrabble);

\copy twl98 from 'data/twl98.data'

create table sowpods (
       primary key (word)
) inherits (scrabble);

\copy sowpods from 'data/sowpods.data'

-- test order by
select * from sowpods order by word limit 10;

-- test joining on the column
select count(*) as common from sowpods join twl98 using (word);
select count(*) as twl98_only from twl98 left join sowpods using (word)
       where sowpods.word is null;
select count(*) as sowpods_only from sowpods left join twl98 using (word)
       where twl98.word is null;

-- test send/recv
\copy sowpods to 'data/sowpods.data.bin' with binary;

create temporary table sowpods_reload (
       primary key (word)
) inherits (scrabble);

\copy sowpods_reload from 'data/sowpods.data.bin' with binary;

select ( select count(*) from sowpods_reload ) - ( select count(*) from sowpods ) as zero;

select ( select count(*) from sowpods ) - ( select count(*) from sowpods join sowpods_reload using (word) ) as zero;

create table scrabble_letters (
       letter char(1),
       primary key (letter),
       score int2
);

\copy scrabble_letters from 'data/scrabble_letters.tsv'

-- score the tables!
create or replace function score(text) returns bigint as $$
       select sum(score)
       from regexp_split_to_table($1, '') as x
            join scrabble_letters on (letter = x)
$$ language 'sql';

-- test that the casts work for functions which take text arguments
update scrabble
   set score = score(word);

-- improve your scrabble game
select a.word, a.score from sowpods a join twl98 using (word) order by a.score desc limit 20;
