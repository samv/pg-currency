
-- simple creation etc
select '100 eur'::currency as "100.00 eur";
select '-100 eur'::currency as "-100.00 eur";

-- check rounding rules
select '-100.005 eur'::currency as "-100.00 eur";
select '-100.006 eur'::currency as "-100.01 eur";
