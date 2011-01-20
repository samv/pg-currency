/* contrib/currency/uninstall_currency.sql */

-- Adjust this setting to control where the objects get dropped.
SET search_path = public;

DROP OPERATOR CLASS tla_ops USING btree CASCADE;
DROP OPERATOR CLASS tla_ops USING hash CASCADE;

DROP CAST (tla AS text);
DROP CAST (text AS tla);

DROP OPERATOR = (tla, tla);
DROP OPERATOR <> (tla, tla);
DROP OPERATOR #<# (tla, tla);
DROP OPERATOR #<=# (tla, tla);
DROP OPERATOR #>=# (tla, tla);
DROP OPERATOR #># (tla, tla);

DROP FUNCTION ne(tla, tla);
DROP FUNCTION eq(tla, tla);
DROP FUNCTION le(tla, tla);
DROP FUNCTION lt(tla, tla);
DROP FUNCTION ge(tla, tla);
DROP FUNCTION gt(tla, tla);

DROP FUNCTION btcmp_tla(tla, tla);
DROP FUNCTION hash_tla(tla);

DROP FUNCTION tla_in(cstring);
DROP FUNCTION tla_out(tla);
DROP FUNCTION tla_send(tla);
DROP FUNCTION tla_recv(internal);
DROP FUNCTION tla_in_text(text);
DROP FUNCTION tla_out_text(tla);

DROP TYPE tla CASCADE;
