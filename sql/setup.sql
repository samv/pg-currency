--
-- first, define the datatype.  Turn off echoing so that expected file
-- does not depend on contents of citext.sql.
--
SET client_min_messages = warning;
\set ECHO none
\i currency.sql
RESET client_min_messages;
\set ECHO all

