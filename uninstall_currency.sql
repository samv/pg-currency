/* contrib/currency/uninstall_currency.sql */

-- Adjust this setting to control where the objects get dropped.
SET search_path = public;

DROP OPERATOR CLASS currency_code_ops USING btree CASCADE;
DROP OPERATOR CLASS currency_code_ops USING hash CASCADE;

DROP OPERATOR = (currency_code, currency_code);
DROP OPERATOR <> (currency_code, currency_code);
DROP OPERATOR < (currency_code, currency_code);
DROP OPERATOR <= (currency_code, currency_code);
DROP OPERATOR >= (currency_code, currency_code);
DROP OPERATOR > (currency_code, currency_code);

DROP FUNCTION ne(currency_code, currency_code);
DROP FUNCTION eq(currency_code, currency_code);
DROP FUNCTION le(currency_code, currency_code);
DROP FUNCTION lt(currency_code, currency_code);
DROP FUNCTION ge(currency_code, currency_code);
DROP FUNCTION gt(currency_code, currency_code);

DROP FUNCTION btcmp_currency_code(currency_code, currency_code);
DROP FUNCTION hash_currency_code(currency_code);

DROP FUNCTION currency_code_in(cstring);
DROP FUNCTION currency_code_out(currency_code);

DROP TYPE currency_code CASCADE;
