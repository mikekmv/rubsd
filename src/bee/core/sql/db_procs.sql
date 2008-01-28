-- $Id$

BEGIN TRANSACTION;

-------------------------------------------------
--    Aux functions   
-------------------------------------------------

CREATE OR REPLACE FUNCTION  getfirst(bigint[])
   RETURNS bigint AS
$BODY$
   SELECT $1[array_lower($1, 1)];
$BODY$
  LANGUAGE 'sql' VOLATILE;

-------------------------------------------------
--    Resource lookups   
-------------------------------------------------

-- PROTO: bigint[] ress()

CREATE OR REPLACE FUNCTION ress()
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM resources WHERE NOT deleted ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] res4name (name varchar)

CREATE OR REPLACE FUNCTION res4name(varchar)
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM resources WHERE NOT deleted AND 
      (name = $1 OR ((name IS NULL) AND ($1 IS NULL))) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] res4name (name varchar, from bigint[])

CREATE OR REPLACE FUNCTION res4name(varchar, bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM resources WHERE NOT deleted AND 
      (name = $1 OR ((name IS NULL) AND ($1 IS NULL))) AND 
      id = ANY ($2) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] res4expr (expr varchar)

CREATE OR REPLACE FUNCTION res4expr(varchar)
   RETURNS bigint[] AS
$BODY$
DECLARE
   arg_expr ALIAS FOR $1;

   query text;
   rec   RECORD;
BEGIN
   IF arg_expr IS NULL THEN
      RETURN NULL;
   END IF;

   query =
'SELECT ARRAY(SELECT id FROM resources WHERE NOT deleted AND ' ||
arg_expr || ' ORDER BY id) AS array;';

-- RAISE NOTICE 'QUERY: <%>', query;

   FOR rec IN EXECUTE query LOOP
      RETURN rec.array;
   END LOOP;

   RETURN NULL;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--------------------------

-- PROTO: bigint[] res4expr (expr varchar, from bigint[])

CREATE OR REPLACE FUNCTION res4expr(varchar, bigint[])
   RETURNS bigint[] AS
$BODY$
DECLARE
   arg_expr ALIAS FOR $1;
   arg_from ALIAS FOR $2;

   query  text;
   rec    RECORD;
BEGIN
   IF arg_expr IS NULL OR arg_from IS NULL THEN
      RETURN NULL;
   END IF;

   query =
'SELECT ARRAY(SELECT id FROM resources WHERE NOT deleted AND ' ||
$1 || ' AND id = ANY (''{' || array_to_string(arg_from, ',') || '}'')
ORDER BY id) AS array;';

-- RAISE NOTICE 'QUERY: <%>', query;

   FOR rec IN EXECUTE query LOOP
      RETURN rec.array;
   END LOOP;

   RETURN NULL;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--------------------------

-- PROTO: SETOF resources rec4res (res bigint[])

CREATE OR REPLACE FUNCTION rec4res(bigint[])
   RETURNS SETOF resources AS
$BODY$
   SELECT * FROM resources WHERE NOT deleted AND 
      id = ANY ($1) ORDER BY id;
$BODY$
  LANGUAGE 'sql' VOLATILE;

-------------------------------------------------
--    Groups lookups   
-------------------------------------------------

-- PROTO: bigint[] groups()

CREATE OR REPLACE FUNCTION groups()
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM groups ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------


-- PROTO: bigint[] group4name (name varchar)

CREATE OR REPLACE FUNCTION group4name(varchar)
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM groups WHERE  
      (name = $1 OR ((name IS NULL) AND ($1 IS NULL))) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------


-- PROTO: bigint[] group4name (name varchar, from bigint[])

CREATE OR REPLACE FUNCTION group4name(varchar, bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM groups WHERE 
      (name = $1 OR ((name IS NULL) AND ($1 IS NULL))) AND 
      id = ANY ($2) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] group4expr (expr varchar)

CREATE OR REPLACE FUNCTION group4expr(varchar)
   RETURNS bigint[] AS
$BODY$
DECLARE
   arg_expr ALIAS FOR $1;

   query text;
   rec   RECORD;
BEGIN
   IF arg_expr IS NULL THEN
      RETURN NULL;
   END IF;

   query = 'SELECT ARRAY(SELECT id FROM groups WHERE ' ||
arg_expr || ' ORDER BY id) AS array;';

-- RAISE NOTICE 'QUERY: <%>', query;

   FOR rec IN EXECUTE query LOOP
      RETURN rec.array;
   END LOOP;

   RETURN NULL;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--------------------------

-- PROTO: bigint[] group4expr (expr varchar, from bigint[])

CREATE OR REPLACE FUNCTION group4expr(varchar, bigint[])
   RETURNS bigint[] AS
$BODY$
DECLARE
   arg_expr ALIAS FOR $1;
   arg_from ALIAS FOR $2;

   query  text;
   rec    RECORD;
BEGIN
   IF arg_expr IS NULL OR arg_from IS NULL THEN
      RETURN NULL;
   END IF;

   query = 'SELECT ARRAY(SELECT id FROM groups WHERE ' ||
$1 || ' AND id = ANY (''{' || array_to_string(arg_from, ',') || '}'')
ORDER BY id) AS array;';

-- RAISE NOTICE 'QUERY: <%>', query;

   FOR rec IN EXECUTE query LOOP
      RETURN rec.array;
   END LOOP;

   RETURN NULL;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--------------------------

-- PROTO: SETOF groups rec4group (group bigint[])

CREATE OR REPLACE FUNCTION rec4group(bigint[])
   RETURNS SETOF groups AS
$BODY$
   SELECT * FROM groups WHERE id = ANY ($1) ORDER BY id;
$BODY$
  LANGUAGE 'sql' VOLATILE;

-------------------------------------------------
--    User lookups   
-------------------------------------------------

-- PROTO: bigint[] users()

CREATE OR REPLACE FUNCTION users()
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM coreauth WHERE NOT deleted ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] user4name (name varchar)

CREATE OR REPLACE FUNCTION user4name(varchar)
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM coreauth WHERE NOT deleted AND 
      (name = $1 OR ((name IS NULL) AND ($1 IS NULL))) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] user4name (name varchar, from bigint[])

CREATE OR REPLACE FUNCTION user4name(varchar, bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM coreauth WHERE NOT deleted AND 
      (name = $1 OR ((name IS NULL) AND ($1 IS NULL))) AND 
      id = ANY ($2) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] user4expr (expr varchar)

CREATE OR REPLACE FUNCTION user4expr(varchar)
   RETURNS bigint[] AS
$BODY$
DECLARE
   arg_expr ALIAS FOR $1;

   query text;
   rec   RECORD;
BEGIN
   IF arg_expr IS NULL THEN
      RETURN NULL;
   END IF;

   query =
'SELECT ARRAY(SELECT id FROM coreauth WHERE NOT deleted AND ' ||
arg_expr || ' ORDER BY id) AS array;';

-- RAISE NOTICE 'QUERY: <%>', query;

   FOR rec IN EXECUTE query LOOP
      RETURN rec.array;
   END LOOP;

   RETURN NULL;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--------------------------

-- PROTO: bigint[] user4expr (expr varchar, from bigint[])

CREATE OR REPLACE FUNCTION user4expr(varchar, bigint[])
   RETURNS bigint[] AS
$BODY$
DECLARE
   arg_expr ALIAS FOR $1;
   arg_from ALIAS FOR $2;

   query  text;
   rec    RECORD;
BEGIN
   IF arg_expr IS NULL OR arg_from IS NULL THEN
      RETURN NULL;
   END IF;

   query =
'SELECT ARRAY(SELECT id FROM coreauth WHERE NOT deleted AND ' ||
$1 || ' AND id = ANY (''{' || array_to_string(arg_from, ',') || '}'')
ORDER BY id) AS array;';

-- RAISE NOTICE 'QUERY: <%>', query;

   FOR rec IN EXECUTE query LOOP
      RETURN rec.array;
   END LOOP;

   RETURN NULL;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--------------------------

-- PROTO: SETOF resources rec4user (user bigint[])

CREATE OR REPLACE FUNCTION rec4user(bigint[])
   RETURNS SETOF coreauth AS
$BODY$
   SELECT * FROM coreauth WHERE NOT deleted AND 
      id = ANY ($1) ORDER BY id;
$BODY$
  LANGUAGE 'sql' VOLATILE;

-------------------------------------------------
--    Tariff plan lookups   
-------------------------------------------------

-- PROTO: bigint[] plans()

CREATE OR REPLACE FUNCTION plans()
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM plans WHERE NOT deleted ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] plan4name (name varchar)

CREATE OR REPLACE FUNCTION plan4name(varchar)
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM plans WHERE NOT deleted AND 
      (name = $1 OR ((name IS NULL) AND ($1 IS NULL))) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] plan4name (name varchar, from bigint[])

CREATE OR REPLACE FUNCTION plan4name(varchar, bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM plans WHERE NOT deleted AND 
      (name = $1 OR ((name IS NULL) AND ($1 IS NULL))) AND 
      id = ANY ($2) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] plan4expr (expr varchar)

CREATE OR REPLACE FUNCTION plan4expr(varchar)
   RETURNS bigint[] AS
$BODY$
DECLARE
   arg_expr ALIAS FOR $1;

   query text;
   rec   RECORD;
BEGIN
   IF arg_expr IS NULL THEN
      RETURN NULL;
   END IF;

   query =
'SELECT ARRAY(SELECT id FROM plans WHERE NOT deleted AND ' ||
arg_expr || ' ORDER BY id) AS array;';

-- RAISE NOTICE 'QUERY: <%>', query;

   FOR rec IN EXECUTE query LOOP
      RETURN rec.array;
   END LOOP;

   RETURN NULL;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--------------------------

-- PROTO: bigint[] plan4expr (expr varchar, from bigint[])

CREATE OR REPLACE FUNCTION plan4expr(varchar, bigint[])
   RETURNS bigint[] AS
$BODY$
DECLARE
   arg_expr ALIAS FOR $1;
   arg_from ALIAS FOR $2;

   query  text;
   rec    RECORD;
BEGIN
   IF arg_expr IS NULL OR arg_from IS NULL THEN
      RETURN NULL;
   END IF;

   query =
'SELECT ARRAY(SELECT id FROM plans WHERE NOT deleted AND ' ||
$1 || ' AND id = ANY (''{' || array_to_string(arg_from, ',') || '}'')
ORDER BY id) AS array;';

-- RAISE NOTICE 'QUERY: <%>', query;

   FOR rec IN EXECUTE query LOOP
      RETURN rec.array;
   END LOOP;

   RETURN NULL;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--------------------------

-- PROTO: SETOF resources rec4plan (user bigint[])

CREATE OR REPLACE FUNCTION rec4plan(bigint[])
   RETURNS SETOF plans AS
$BODY$
   SELECT * FROM plans WHERE NOT deleted AND 
      id = ANY ($1) ORDER BY id;
$BODY$
  LANGUAGE 'sql' VOLATILE;

-------------------------------------------------
--    Tariffs lookups   
-------------------------------------------------

-- PROTO: bigint[] tariffs()

CREATE OR REPLACE FUNCTION tariffs()
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM tariffs ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------


-- PROTO: bigint[] tariff4res (res bigint[])

CREATE OR REPLACE FUNCTION tariff4res(bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM tariffs WHERE  
      res_id = ANY ($1) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------


-- PROTO: bigint[] tariff4res (res bigint[], from bigint[])

CREATE OR REPLACE FUNCTION tariff4res(bigint[], bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM tariffs WHERE 
      res_id = ANY ($1) AND 
      id = ANY ($2) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] tariff4expr (expr varchar)

CREATE OR REPLACE FUNCTION tariff4expr(varchar)
   RETURNS bigint[] AS
$BODY$
DECLARE
   arg_expr ALIAS FOR $1;

   query text;
   rec   RECORD;
BEGIN
   IF arg_expr IS NULL THEN
      RETURN NULL;
   END IF;

   query = 'SELECT ARRAY(SELECT id FROM tariffs WHERE ' ||
arg_expr || ' ORDER BY id) AS array;';

-- RAISE NOTICE 'QUERY: <%>', query;

   FOR rec IN EXECUTE query LOOP
      RETURN rec.array;
   END LOOP;

   RETURN NULL;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--------------------------

-- PROTO: bigint[] tariff4expr (expr varchar, from bigint[])

CREATE OR REPLACE FUNCTION tariff4expr(varchar, bigint[])
   RETURNS bigint[] AS
$BODY$
DECLARE
   arg_expr ALIAS FOR $1;
   arg_from ALIAS FOR $2;

   query  text;
   rec    RECORD;
BEGIN
   IF arg_expr IS NULL OR arg_from IS NULL THEN
      RETURN NULL;
   END IF;

   query = 'SELECT ARRAY(SELECT id FROM tariffs WHERE ' ||
$1 || ' AND id = ANY (''{' || array_to_string(arg_from, ',') || '}'')
ORDER BY id) AS array;';

-- RAISE NOTICE 'QUERY: <%>', query;

   FOR rec IN EXECUTE query LOOP
      RETURN rec.array;
   END LOOP;

   RETURN NULL;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--------------------------

-- PROTO: SETOF tariffs rec4tariff (group bigint[])

CREATE OR REPLACE FUNCTION rec4tariff(bigint[])
   RETURNS SETOF tariffs AS
$BODY$
   SELECT * FROM tariffs WHERE id = ANY ($1) ORDER BY id;
$BODY$
  LANGUAGE 'sql' VOLATILE;

-------------------------------------------------
--    Tariff alternative prices (ALTs) lookups   
-------------------------------------------------

-- PROTO: bigint[] alts()

CREATE OR REPLACE FUNCTION alts()
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM tariff_nums ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------


-- PROTO: bigint[] alt4tariff (tariff bigint[])

CREATE OR REPLACE FUNCTION alt4tariff(bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM tariff_nums WHERE  
      tariff_id = ANY ($1) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] alt4tariff (tariff bigint[], from bigint[])

CREATE OR REPLACE FUNCTION alt4tariff(bigint[], bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM tariff_nums WHERE 
      tariff_id = ANY ($1) AND 
      id = ANY ($2) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] alt4time (time timestamptz, tariff bigint[])

CREATE OR REPLACE FUNCTION alt4time(timestamptz, bigint[])
   RETURNS bigint[] AS
$BODY$
DECLARE
   arg_time   ALIAS FOR $1;
   arg_tariff ALIAS FOR $2;

   timeshift  interval;
   corrtime   timestamptz;
   dtime      timetz;
   wday       integer;
   n          bigint;
BEGIN
-- Trap NULL arguments
   IF arg_time IS NULL OR arg_tariff IS NULL THEN
      RETURN NULL;
   END IF;

-- Load time shift
   SELECT time_shift INTO timeshift FROM resources WHERE 
      id = (SELECT res_id FROM tariffs WHERE
             id = getfirst(arg_tariff) AND
      NOT deleted);
   IF NOT FOUND THEN
      n = getfirst(arg_tariff);
      RAISE NOTICE 'tariff % is invalid', n;
      RETURN NULL; 
   END IF;
   
-- Correct time w/ resource time shift & count time values
   corrtime = arg_time - timeshift;
   dtime    = corrtime::timetz;
   wday     = extract(dow FROM corrtime);   

-- Find & return matching numinator id
   RETURN ARRAY
   (  SELECT id FROM tariff_nums WHERE  
      tariff_id = getfirst(arg_tariff) AND
      (corrtime >= time_from OR time_from IS NULL)    AND (corrtime < time_to OR time_to IS NULL)    AND
      (dtime >= daytime_from OR daytime_from IS NULL) AND (dtime < daytime_to OR daytime_to IS NULL) AND
      (wday >= weekday_from OR weekday_from IS NULL)  AND (wday <= weekday_to OR weekday_to IS NULL)
      ORDER BY priority DESC
      LIMIT 1
   );

END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--------------------------

-- PROTO: bigint[] alt4time (time timestamptz, tariff bigint[], from bigint[])

CREATE OR REPLACE FUNCTION alt4time(timestamptz, bigint[], bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM tariff_nums WHERE 
      id = ANY (alt4time($1, $2)) AND 
      id = ANY ($3) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] alt4expr (expr varchar)

CREATE OR REPLACE FUNCTION alt4expr(varchar)
   RETURNS bigint[] AS
$BODY$
DECLARE
   arg_expr ALIAS FOR $1;

   query text;
   rec   RECORD;
BEGIN
   IF arg_expr IS NULL THEN
      RETURN NULL;
   END IF;

   query = 'SELECT ARRAY(SELECT id FROM tariff_nums WHERE ' ||
arg_expr || ' ORDER BY id) AS array;';

-- RAISE NOTICE 'QUERY: <%>', query;

   FOR rec IN EXECUTE query LOOP
      RETURN rec.array;
   END LOOP;

   RETURN NULL;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--------------------------

-- PROTO: bigint[] alt4expr (expr varchar, from bigint[])

CREATE OR REPLACE FUNCTION alt4expr(varchar, bigint[])
   RETURNS bigint[] AS
$BODY$
DECLARE
   arg_expr ALIAS FOR $1;
   arg_from ALIAS FOR $2;

   query  text;
   rec    RECORD;
BEGIN
   IF arg_expr IS NULL OR arg_from IS NULL THEN
      RETURN NULL;
   END IF;

   query = 'SELECT ARRAY(SELECT id FROM tariff_nums WHERE ' ||
$1 || ' AND id = ANY (''{' || array_to_string(arg_from, ',') || '}'')
ORDER BY id) AS array;';

-- RAISE NOTICE 'QUERY: <%>', query;

   FOR rec IN EXECUTE query LOOP
      RETURN rec.array;
   END LOOP;

   RETURN NULL;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--------------------------

-- PROTO: SETOF tariff_nums rec4alt (alt bigint[])

CREATE OR REPLACE FUNCTION rec4alt(bigint[])
   RETURNS SETOF tariff_nums AS
$BODY$
   SELECT * FROM tariff_nums WHERE id = ANY ($1) ORDER BY id;
$BODY$
  LANGUAGE 'sql' VOLATILE;

-------------------------------------------------
--   Tariff plan items/members (PITs) lookups   
-------------------------------------------------

-- PROTO: bigint[] pits()

CREATE OR REPLACE FUNCTION pits()
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM plan_members ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] pit4plan (plan bigint[])

CREATE OR REPLACE FUNCTION pit4plan(bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM plan_members WHERE  
      plan_id = ANY ($1) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] pit4plan (plan bigint[], from bigint[])

CREATE OR REPLACE FUNCTION pit4plan(bigint[], bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM plan_members WHERE 
      plan_id = ANY ($1) AND 
      id = ANY ($2) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] pit4proto (res bigint[], proto bigint)

CREATE OR REPLACE FUNCTION pit4proto(bigint[], bigint)
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT pm.id FROM plan_members AS pm, tariffs AS t WHERE  
      pm.tariff_id = t.id AND
      t.res_id = ANY ($1) AND
      pm.proto_test  = ($2 & pm.proto_mask) 
      ORDER BY pm.priority DESC LIMIT 1);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] pit4proto (res bigint[], proto bigint, from bigint[])

CREATE OR REPLACE FUNCTION pit4proto(bigint[], bigint, bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT pm.id FROM plan_members AS pm, tariffs AS t WHERE 
      pm.tariff_id = t.id AND
      t.res_id = ANY ($1) AND 
      pm.proto_test  = ($2 & pm.proto_mask) AND 
      pm.id = ANY ($3) ORDER BY pm.priority DESC LIMIT 1);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] pit4all (plan bigint[], res bigint[], proto bigint)

CREATE OR REPLACE FUNCTION pit4all(bigint[], bigint[], bigint)
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT pm.id FROM plan_members AS pm, tariffs AS t WHERE  
      pm.tariff_id = t.id AND
      pm.plan_id = ANY ($1) AND
      t.res_id = ANY ($2) AND
      pm.proto_test  = ($3 & pm.proto_mask)
      ORDER BY pm.priority DESC LIMIT 1);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] pit4all (plan bigint[], res bigint[], proto bigint, from bigint[])

CREATE OR REPLACE FUNCTION pit4all(bigint[], bigint[], bigint, bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT pm.id FROM plan_members AS pm, tariffs AS t WHERE  
      pm.tariff_id = t.id AND
      pm.plan_id = ANY ($1) AND
      t.res_id = ANY ($2) AND
      pm.proto_test  = ($3 & pm.proto_mask) AND
      pm.id = ANY ($4)
      ORDER BY pm.priority DESC LIMIT 1);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] pit4expr (expr varchar)

CREATE OR REPLACE FUNCTION pit4expr(varchar)
   RETURNS bigint[] AS
$BODY$
DECLARE
   arg_expr ALIAS FOR $1;

   query text;
   rec   RECORD;
BEGIN
   IF arg_expr IS NULL THEN
      RETURN NULL;
   END IF;

   query = 'SELECT ARRAY(SELECT id FROM plan_members WHERE ' ||
arg_expr || ' ORDER BY id) AS array;';

-- RAISE NOTICE 'QUERY: <%>', query;

   FOR rec IN EXECUTE query LOOP
      RETURN rec.array;
   END LOOP;

   RETURN NULL;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--------------------------

-- PROTO: bigint[] pit4expr (expr varchar, from bigint[])

CREATE OR REPLACE FUNCTION pit4expr(varchar, bigint[])
   RETURNS bigint[] AS
$BODY$
DECLARE
   arg_expr ALIAS FOR $1;
   arg_from ALIAS FOR $2;

   query  text;
   rec    RECORD;
BEGIN
   IF arg_expr IS NULL OR arg_from IS NULL THEN
      RETURN NULL;
   END IF;

   query = 'SELECT ARRAY(SELECT id FROM plan_members WHERE ' ||
$1 || ' AND id = ANY (''{' || array_to_string(arg_from, ',') || '}'')
ORDER BY id) AS array;';

-- RAISE NOTICE 'QUERY: <%>', query;

   FOR rec IN EXECUTE query LOOP
      RETURN rec.array;
   END LOOP;

   RETURN NULL;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--------------------------

-- PROTO: SETOF plan_members rec4pit (pit bigint[])

CREATE OR REPLACE FUNCTION rec4pit(bigint[])
   RETURNS SETOF plan_members AS
$BODY$
   SELECT * FROM plan_members WHERE id = ANY ($1) ORDER BY id;
$BODY$
  LANGUAGE 'sql' VOLATILE;

-------------------------------------------------
--    Persons lookups   
-------------------------------------------------

-- PROTO: bigint[] persons()

CREATE OR REPLACE FUNCTION persons()
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM persons WHERE NOT deleted ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] person4expr (expr varchar)

CREATE OR REPLACE FUNCTION person4expr(varchar)
   RETURNS bigint[] AS
$BODY$
DECLARE
   arg_expr ALIAS FOR $1;

   query text;
   rec   RECORD;
BEGIN
   IF arg_expr IS NULL THEN
      RETURN NULL;
   END IF;

   query =
'SELECT ARRAY(SELECT id FROM persons WHERE NOT deleted AND ' ||
arg_expr || ' ORDER BY id) AS array;';

-- RAISE NOTICE 'QUERY: <%>', query;

   FOR rec IN EXECUTE query LOOP
      RETURN rec.array;
   END LOOP;

   RETURN NULL;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--------------------------

-- PROTO: bigint[] person4expr (expr varchar, from bigint[])

CREATE OR REPLACE FUNCTION person4expr(varchar, bigint[])
   RETURNS bigint[] AS
$BODY$
DECLARE
   arg_expr ALIAS FOR $1;
   arg_from ALIAS FOR $2;

   query  text;
   rec    RECORD;
BEGIN
   IF arg_expr IS NULL OR arg_from IS NULL THEN
      RETURN NULL;
   END IF;

   query =
'SELECT ARRAY(SELECT id FROM persons WHERE NOT deleted AND ' ||
$1 || ' AND id = ANY (''{' || array_to_string(arg_from, ',') || '}'')
ORDER BY id) AS array;';

-- RAISE NOTICE 'QUERY: <%>', query;

   FOR rec IN EXECUTE query LOOP
      RETURN rec.array;
   END LOOP;

   RETURN NULL;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--------------------------

-- PROTO: SETOF persons rec4person (person bigint[])

CREATE OR REPLACE FUNCTION rec4person(bigint[])
   RETURNS SETOF persons AS
$BODY$
   SELECT * FROM persons WHERE NOT deleted AND 
      id = ANY ($1) ORDER BY id;
$BODY$
  LANGUAGE 'sql' VOLATILE;

-------------------------------------------------
--    Accounts lookups   
-------------------------------------------------

-- PROTO: bigint[] accs()

CREATE OR REPLACE FUNCTION accs()
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM accs WHERE NOT deleted ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] acc4plan (plan bigint[])

CREATE OR REPLACE FUNCTION acc4plan(bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM accs WHERE  
      plan_id = ANY ($1) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] acc4plan (plan bigint[], from bigint[])

CREATE OR REPLACE FUNCTION acc4plan(bigint[], bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM accs WHERE 
      plan_id = ANY ($1) AND 
      id = ANY ($2) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] acc4crplan (crplan bigint[])

CREATE OR REPLACE FUNCTION acc4crplan(bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM accs WHERE  
      cr_plan_id = ANY ($1) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] acc4crplan (crplan bigint[], from bigint[])

CREATE OR REPLACE FUNCTION acc4crplan(bigint[], bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM accs WHERE 
      cr_plan_id = ANY ($1) AND 
      id = ANY ($2) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] acc4person (person bigint[])

CREATE OR REPLACE FUNCTION acc4person(bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM accs WHERE  
      person_id = ANY ($1) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] acc4person (person bigint[], from bigint[])

CREATE OR REPLACE FUNCTION acc4person(bigint[], bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM accs WHERE 
      person_id = ANY ($1) AND 
      id = ANY ($2) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] acc4group (group bigint[])

CREATE OR REPLACE FUNCTION acc4group(bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT a.id FROM accs AS a, group_members AS gm WHERE  
      a.id = gm.acc_id AND
      gm.group_id = ANY ($1) ORDER BY a.id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] acc4group (group bigint[], from bigint[])

CREATE OR REPLACE FUNCTION acc4group(bigint[], bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT a.id FROM accs AS a, group_members AS gm WHERE  
      a.id = gm.acc_id AND
      gm.group_id = ANY ($1) AND
      a.id = ANY ($2) ORDER BY a.id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] acc4label (label varchar)

CREATE OR REPLACE FUNCTION acc4label(varchar)
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM accs WHERE NOT deleted AND 
      (label = $1 OR ((label IS NULL) AND ($1 IS NULL))) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] acc4label (label varchar, from bigint[])

CREATE OR REPLACE FUNCTION acc4label(varchar, bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM accs WHERE NOT deleted AND 
      (label = $1 OR ((label IS NULL) AND ($1 IS NULL))) AND 
      id = ANY ($2) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] acc4expr (expr varchar)

CREATE OR REPLACE FUNCTION acc4expr(varchar)
   RETURNS bigint[] AS
$BODY$
DECLARE
   arg_expr ALIAS FOR $1;

   query text;
   rec   RECORD;
BEGIN
   IF arg_expr IS NULL THEN
      RETURN NULL;
   END IF;

   query =
'SELECT ARRAY(SELECT id FROM accs WHERE NOT deleted AND ' ||
arg_expr || ' ORDER BY id) AS array;';

-- RAISE NOTICE 'QUERY: <%>', query;

   FOR rec IN EXECUTE query LOOP
      RETURN rec.array;
   END LOOP;

   RETURN NULL;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--------------------------

-- PROTO: bigint[] acc4expr (expr varchar, from bigint[])

CREATE OR REPLACE FUNCTION acc4expr(varchar, bigint[])
   RETURNS bigint[] AS
$BODY$
DECLARE
   arg_expr ALIAS FOR $1;
   arg_from ALIAS FOR $2;

   query  text;
   rec    RECORD;
BEGIN
   IF arg_expr IS NULL OR arg_from IS NULL THEN
      RETURN NULL;
   END IF;

   query =
'SELECT ARRAY(SELECT id FROM accs WHERE NOT deleted AND ' ||
$1 || ' AND id = ANY (''{' || array_to_string(arg_from, ',') || '}'')
ORDER BY id) AS array;';

-- RAISE NOTICE 'QUERY: <%>', query;

   FOR rec IN EXECUTE query LOOP
      RETURN rec.array;
   END LOOP;

   RETURN NULL;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--------------------------

-- PROTO: SETOF accs rec4acc (acc bigint[])

CREATE OR REPLACE FUNCTION rec4acc(bigint[])
   RETURNS SETOF accs AS
$BODY$
   SELECT * FROM accs WHERE NOT deleted AND 
      id = ANY ($1) ORDER BY id;
$BODY$
  LANGUAGE 'sql' VOLATILE;

-------------------------------------------------
--    Payment remainders lookups   
-------------------------------------------------

-- PROTO: bigint[] rems()

CREATE OR REPLACE FUNCTION rems()
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM remainders ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] rem4plan (plan bigint[])

CREATE OR REPLACE FUNCTION rem4plan(bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM remainders WHERE  
      plan_id = ANY ($1) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] rem4plan (plan bigint[], from bigint[])

CREATE OR REPLACE FUNCTION rem4plan(bigint[], bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM remainders WHERE 
      plan_id = ANY ($1) AND 
      id = ANY ($2) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] rem4acc (acc bigint[])

CREATE OR REPLACE FUNCTION rem4acc(bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM remainders WHERE  
      acc_id = ANY ($1) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] rem4acc (acc bigint[], from bigint[])

CREATE OR REPLACE FUNCTION rem4acc(bigint[], bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM remainders WHERE 
      acc_id = ANY ($1) AND 
      id = ANY ($2) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] rem4res (res bigint[])

CREATE OR REPLACE FUNCTION rem4res(bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM remainders WHERE  
      res_id = ANY ($1) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] rem4res (res bigint[], from bigint[])

CREATE OR REPLACE FUNCTION rem4res(bigint[], bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM remainders WHERE 
      res_id = ANY ($1) AND 
      id = ANY ($2) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] rem4accres (acc bigint[], res bigint[])

CREATE OR REPLACE FUNCTION rem4accres(bigint[], bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM remainders WHERE  
      acc_id = ANY ($1) AND 
      res_id = ANY ($2) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] rem4accres (acc bigint[], res bigint[], from bigint[])

CREATE OR REPLACE FUNCTION rem4accres(bigint[], bigint[], bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM remainders WHERE 
      acc_id = ANY ($1) AND 
      res_id = ANY ($2) AND 
      id = ANY ($3) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] rem4expr (expr varchar)

CREATE OR REPLACE FUNCTION rem4expr(varchar)
   RETURNS bigint[] AS
$BODY$
DECLARE
   arg_expr ALIAS FOR $1;

   query text;
   rec   RECORD;
BEGIN
   IF arg_expr IS NULL THEN
      RETURN NULL;
   END IF;

   query = 'SELECT ARRAY(SELECT id FROM remainders WHERE ' ||
arg_expr || ' ORDER BY id) AS array;';

-- RAISE NOTICE 'QUERY: <%>', query;

   FOR rec IN EXECUTE query LOOP
      RETURN rec.array;
   END LOOP;

   RETURN NULL;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--------------------------

-- PROTO: bigint[] rem4expr (expr varchar, from bigint[])

CREATE OR REPLACE FUNCTION rem4expr(varchar, bigint[])
   RETURNS bigint[] AS
$BODY$
DECLARE
   arg_expr ALIAS FOR $1;
   arg_from ALIAS FOR $2;

   query  text;
   rec    RECORD;
BEGIN
   IF arg_expr IS NULL OR arg_from IS NULL THEN
      RETURN NULL;
   END IF;

   query = 'SELECT ARRAY(SELECT id FROM remainders WHERE ' ||
$1 || ' AND id = ANY (''{' || array_to_string(arg_from, ',') || '}'')
ORDER BY id) AS array;';

-- RAISE NOTICE 'QUERY: <%>', query;

   FOR rec IN EXECUTE query LOOP
      RETURN rec.array;
   END LOOP;

   RETURN NULL;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--------------------------

-- PROTO: SETOF remainders rec4rem (rem bigint[])

CREATE OR REPLACE FUNCTION rec4rem(bigint[])
   RETURNS SETOF remainders AS
$BODY$
   SELECT * FROM remainders WHERE id = ANY ($1) ORDER BY id;
$BODY$
  LANGUAGE 'sql' VOLATILE;

-------------------------------------------------
--    Gates lookups   
-------------------------------------------------

-- PROTO: bigint[] gates()

CREATE OR REPLACE FUNCTION gates()
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM gates ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] gate4acc (acc bigint[])

CREATE OR REPLACE FUNCTION gate4acc(bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM gates WHERE  
      acc_id = ANY ($1) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] gate4acc (acc bigint[], from bigint[])

CREATE OR REPLACE FUNCTION gate4acc(bigint[], bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM gates WHERE 
      acc_id = ANY ($1) AND 
      id = ANY ($2) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] gate4res (res bigint[])

CREATE OR REPLACE FUNCTION gate4res(bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM gates WHERE
      res_id = ANY ($1) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] gate4res (res bigint[], from bigint[])

CREATE OR REPLACE FUNCTION gate4res(bigint[], bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM gates WHERE 
      res_id = ANY ($1) AND 
      id = ANY ($2) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] gate4ident_strict (res bigint[], sample varchar)

CREATE OR REPLACE FUNCTION gate4ident_strict(bigint[], sample varchar)
   RETURNS bigint[] AS
$BODY$
DECLARE
  arg_res    ALIAS FOR $1;
  arg_sample ALIAS FOR $2;

  rec      RECORD;
  query    text;
BEGIN

   query =
'SELECT ARRAY (SELECT g.id
FROM gates   AS g, ' ||
(SELECT q_gdata FROM resources WHERE id = getfirst(arg_res)) ||
' WHERE
g.gdata_id   = gd.id  AND
g.res_id     = ' || (getfirst(arg_res))::text ||
' AND gd.ident = ' || quote_literal(arg_sample) || '
ORDER by g.id) AS array';
 
-- RAISE NOTICE 'QUERY: <%>', query;

   FOR rec IN EXECUTE query LOOP
          RETURN rec.array;
   END LOOP;

   RETURN NULL;

END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--------------------------

-- PROTO: bigint[] gate4ident_strict (res bigint[], sample varchar, from bigint[])

CREATE OR REPLACE FUNCTION gate4ident_strict(bigint[], sample varchar, bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM gates WHERE 
      id = ANY (gate4ident_strict($1, $2)) AND 
      id = ANY ($3) ORDER BY id);   
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] gate4ident_hit (res bigint[], sample varchar)

CREATE OR REPLACE FUNCTION gate4ident_hit(bigint[], varchar)
   RETURNS bigint[] AS
$BODY$
DECLARE
  arg_res    ALIAS FOR $1;
  arg_sample ALIAS FOR $2;

  res      RECORD;
  rec      RECORD;
  query    text;
  cmpexpr  text;
BEGIN

-- Load resource record
   SELECT * INTO res FROM resources WHERE id = getfirst(arg_res) AND NOT deleted;
   IF NOT FOUND THEN RETURN NULL; END IF;

-- Make compare expression
   cmpexpr = replace(res.q_cmpident, '$2', quote_literal(arg_sample));
   cmpexpr = replace(cmpexpr, '$1', 'gd.ident');

   query =
'SELECT ARRAY (SELECT g.id
FROM gates   AS g, ' || res.q_gdata || ' WHERE
g.gdata_id   = gd.id  AND
g.res_id     = ' || (res.id)::text ||
' AND ' || cmpexpr || '
ORDER by g.id) AS array';
 
-- RAISE NOTICE 'QUERY: <%>', query;

   FOR rec IN EXECUTE query LOOP
          RETURN rec.array;
   END LOOP;

   RETURN NULL;

END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--------------------------

-- PROTO: bigint[] gate4ident_hit (res bigint[], sample varchar, from bigint[])

CREATE OR REPLACE FUNCTION gate4ident_hit(bigint[], sample varchar, bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM gates WHERE 
      id = ANY (gate4ident_hit($1, $2)) AND 
      id = ANY ($3) ORDER BY id);   
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] gate4expr (expr varchar)

CREATE OR REPLACE FUNCTION gate4expr(varchar)
   RETURNS bigint[] AS
$BODY$
DECLARE
   arg_expr ALIAS FOR $1;

   query text;
   rec   RECORD;
BEGIN
   IF arg_expr IS NULL THEN
      RETURN NULL;
   END IF;

   query = 'SELECT ARRAY(SELECT id FROM gates WHERE ' ||
arg_expr || ' ORDER BY id) AS array;';

-- RAISE NOTICE 'QUERY: <%>', query;

   FOR rec IN EXECUTE query LOOP
      RETURN rec.array;
   END LOOP;

   RETURN NULL;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--------------------------

-- PROTO: bigint[] gate4expr (expr varchar, from bigint[])

CREATE OR REPLACE FUNCTION gate4expr(varchar, bigint[])
   RETURNS bigint[] AS
$BODY$
DECLARE
   arg_expr ALIAS FOR $1;
   arg_from ALIAS FOR $2;

   query  text;
   rec    RECORD;
BEGIN
   IF arg_expr IS NULL OR arg_from IS NULL THEN
      RETURN NULL;
   END IF;

   query = 'SELECT ARRAY(SELECT id FROM gates WHERE ' ||
$1 || ' AND id = ANY (''{' || array_to_string(arg_from, ',') || '}'')
ORDER BY id) AS array;';

-- RAISE NOTICE 'QUERY: <%>', query;

   FOR rec IN EXECUTE query LOOP
      RETURN rec.array;
   END LOOP;

   RETURN NULL;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--------------------------

-- PROTO: SETOF gates rec4gate (gate bigint[])

CREATE OR REPLACE FUNCTION rec4gate(bigint[])
   RETURNS SETOF gates AS
$BODY$
   SELECT * FROM gates WHERE id = ANY ($1) ORDER BY id;
$BODY$
  LANGUAGE 'sql' VOLATILE;
-------------------------

-- PROTO: SETOF vgate vrec4gate (gate bigint[])

CREATE OR REPLACE FUNCTION vrec4gate(bigint[])
   RETURNS SETOF vgate AS
$BODY$
DECLARE
  res     RECORD;
  rec     vgate;
  query   text;
BEGIN

   FOR res IN SELECT * FROM resources WHERE NOT deleted LOOP

   query =
'SELECT g.id, g.fdeny, g.fgrant, g.ffree, g.acc_id,
    g.res_id,
    gd.ident::varchar AS ident,
    gd.ctrl::varchar  AS ctrl  
FROM gates   AS g, ' ||
(SELECT q_gdata FROM resources WHERE id = res.id) ||
' WHERE
g.gdata_id   = gd.id  AND
g.id         = ANY (''{' || array_to_string($1, ',') || '}'') AND
g.res_id     = ' || res.id::text ||
' ORDER by g.id;';

-- RAISE NOTICE 'QUERY: <%>', query;

      FOR rec IN EXECUTE query LOOP
          RETURN NEXT rec;
      END LOOP;
   END LOOP;

   RETURN;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;

-------------------------------------------------
--    Payment cards lookups   
-------------------------------------------------

-- PROTO: bigint[] pcards()

CREATE OR REPLACE FUNCTION pcards()
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM paycards ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] pcard4batch (batch bigint[])

CREATE OR REPLACE FUNCTION pcard4batch(bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM paycards WHERE  
      batchno = ANY ($1) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] pcard4batch (batch bigint[], from bigint[])

CREATE OR REPLACE FUNCTION pcard4batch(bigint[], bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM paycards WHERE 
      batchno = ANY ($1) AND 
      id = ANY ($2) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] pcard4res (res bigint[])

CREATE OR REPLACE FUNCTION pcard4res(bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM paycards WHERE
      res_id = ANY ($1) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] pcard4res (res bigint[], from bigint[])

CREATE OR REPLACE FUNCTION pcard4res(bigint[], bigint[])
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM paycards WHERE 
      res_id = ANY ($1) AND 
      id = ANY ($2) ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] pcard4expr (expr varchar)

CREATE OR REPLACE FUNCTION pcard4expr(varchar)
   RETURNS bigint[] AS
$BODY$
DECLARE
   arg_expr ALIAS FOR $1;

   query text;
   rec   RECORD;
BEGIN
   IF arg_expr IS NULL THEN
      RETURN NULL;
   END IF;

   query = 'SELECT ARRAY(SELECT id FROM paycards WHERE ' ||
arg_expr || ' ORDER BY id) AS array;';

-- RAISE NOTICE 'QUERY: <%>', query;

   FOR rec IN EXECUTE query LOOP
      RETURN rec.array;
   END LOOP;

   RETURN NULL;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--------------------------

-- PROTO: bigint[] pcard4expr (expr varchar, from bigint[])

CREATE OR REPLACE FUNCTION pcard4expr(varchar, bigint[])
   RETURNS bigint[] AS
$BODY$
DECLARE
   arg_expr ALIAS FOR $1;
   arg_from ALIAS FOR $2;

   query  text;
   rec    RECORD;
BEGIN
   IF arg_expr IS NULL OR arg_from IS NULL THEN
      RETURN NULL;
   END IF;

   query = 'SELECT ARRAY(SELECT id FROM paycatds WHERE ' ||
$1 || ' AND id = ANY (''{' || array_to_string(arg_from, ',') || '}'')
ORDER BY id) AS array;';

-- RAISE NOTICE 'QUERY: <%>', query;

   FOR rec IN EXECUTE query LOOP
      RETURN rec.array;
   END LOOP;

   RETURN NULL;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--------------------------

-- PROTO: SETOF paycards rec4pcard (pcard bigint[])

CREATE OR REPLACE FUNCTION rec4pcard(bigint[])
   RETURNS SETOF paycards AS
$BODY$
   SELECT * FROM paycards WHERE id = ANY ($1) ORDER BY id;
$BODY$
  LANGUAGE 'sql' VOLATILE;

-------------------------------------------------
--    Payment cards batches lookups   
-------------------------------------------------

-- PROTO: bigint[] batches()

CREATE OR REPLACE FUNCTION batches()
   RETURNS bigint[] AS
$BODY$
   SELECT ARRAY(SELECT id FROM pcard_batches ORDER BY id);
$BODY$
  LANGUAGE 'sql' VOLATILE;
--------------------------

-- PROTO: bigint[] batch4expr (expr varchar)

CREATE OR REPLACE FUNCTION batch4expr(varchar)
   RETURNS bigint[] AS
$BODY$
DECLARE
   arg_expr ALIAS FOR $1;

   query text;
   rec   RECORD;
BEGIN
   IF arg_expr IS NULL THEN
      RETURN NULL;
   END IF;

   query = 'SELECT ARRAY(SELECT id FROM pcard_batches WHERE ' ||
arg_expr || ' ORDER BY id) AS array;';

-- RAISE NOTICE 'QUERY: <%>', query;

   FOR rec IN EXECUTE query LOOP
      RETURN rec.array;
   END LOOP;

   RETURN NULL;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--------------------------

-- PROTO: bigint[] batch4expr (expr varchar, from bigint[])

CREATE OR REPLACE FUNCTION batch4expr(varchar, bigint[])
   RETURNS bigint[] AS
$BODY$
DECLARE
   arg_expr ALIAS FOR $1;
   arg_from ALIAS FOR $2;

   query  text;
   rec    RECORD;
BEGIN
   IF arg_expr IS NULL OR arg_from IS NULL THEN
      RETURN NULL;
   END IF;

   query = 'SELECT ARRAY(SELECT id FROM pcard_batches WHERE ' ||
$1 || ' AND id = ANY (''{' || array_to_string(arg_from, ',') || '}'')
ORDER BY id) AS array;';

-- RAISE NOTICE 'QUERY: <%>', query;

   FOR rec IN EXECUTE query LOOP
      RETURN rec.array;
   END LOOP;

   RETURN NULL;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;
--------------------------

-- PROTO: SETOF pcard_batches rec4batch (batch bigint[])

CREATE OR REPLACE FUNCTION rec4batch(bigint[])
   RETURNS SETOF pcard_batches AS
$BODY$
   SELECT * FROM pcard_batches WHERE id = ANY ($1) ORDER BY id;
$BODY$
  LANGUAGE 'sql' VOLATILE;

-- =========================================================
--                 Main functions
-- =========================================================

-- Get gates access table for resourse
-- (used by 'update' to construct access tables)
-- PROTO: SETOF access_row	res_access_table(res_id bigint[])
          			
CREATE OR REPLACE FUNCTION res_access_table(bigint[])
  RETURNS SETOF access_row AS
$BODY$
DECLARE
  arg_resid  ALIAS FOR $1;
  res        bigint;
  rec        access_row; 
BEGIN

   res = getfirst(arg_resid);

   FOR rec IN EXECUTE
'SELECT
   gd.ctrl::varchar,
   (  NOT g.fdeny AND NOT a.frozen AND NOT a.paused AND
      (  a.fgrant
         OR
         coalesce((SELECT fgrant FROM plan_members AS pm, tariffs AS t WHERE
            pm.tariff_id = t.id AND pm.proto_mask = 0 AND t.res_id = ' ||

res::text ||

'           AND pm.plan_id = a.plan_id AND NOT t.deleted), FALSE)
         OR
         g.fgrant
         OR
         (SELECT rm.val FROM remainders AS rm WHERE rm.acc_id = a.id AND rm.res_id = ' ||

res::text || ') IS NOT NULL

         OR
         (  a.balance +
            coalesce((SELECT sum(val) FROM remainders AS rm WHERE rm.acc_id = a.id AND rm.res_id IS NULL), 0)
         ) > (-a.credit)
      )
   ) AS result
FROM gates   AS g,' ||

(SELECT q_gdata FROM resources WHERE id = res) || 
', 
   accs         AS a
WHERE NOT a.deleted AND
   g.gdata_id   = gd.id  AND
   g.acc_id     = a.id   AND
   g.res_id     = ' ||

res::text ||

' AND
   g.acc_id     = a.id;'

   LOOP 
       RETURN NEXT rec;
   END LOOP; 

   RETURN;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;


-- Resource expense transaction (writes off some funds from corresponding account)
-- PROTO: boolean		res_transaction(gate_id, value, proto, local_ip, remote_ip, lserv, rserv)

CREATE OR REPLACE FUNCTION res_transaction(bigint[], bigint, bigint, inet, inet, integer, integer)
  RETURNS boolean AS
$BODY$
DECLARE
   arg_gateid ALIAS FOR $1;
   arg_value  ALIAS FOR $2;
   arg_proto  ALIAS FOR $3;
   arg_lip    ALIAS FOR $4;
   arg_rip    ALIAS FOR $5;
   arg_lserv  ALIAS FOR $6;
   arg_rserv  ALIAS FOR $7;

   account    RECORD;
   gate       RECORD;
   remainder  RECORD;
   tariff     RECORD;
   res        RECORD;

   oldbal     float8;
   valremain  bigint;
   frem       bool;
   planid     bigint;
   tariffid   bigint;
   num        float8;
   cost       float8;
   tempcount  bigint;
   curtime    timestamptz;
   last       float8;

BEGIN
-- Trap NULLs
   IF arg_gateid IS NULL OR arg_value IS NULL OR arg_proto IS NULL THEN
      RETURN FALSE;   
   END IF;

-- Store current time (for future price count)
   curtime = current_timestamp;   

-- Load gate record
   SELECT * INTO gate FROM gates WHERE id = getfirst(arg_gateid);
   IF NOT FOUND THEN RETURN FALSE; END IF;
    
-- Load resource record
   SELECT * INTO res FROM resources WHERE id = gate.res_id AND NOT deleted;
   IF NOT FOUND THEN RETURN FALSE; END IF;

-- Load Account record (by id from Gate)
   SELECT * INTO account FROM accs WHERE id = gate.acc_id AND NOT deleted;
   IF NOT FOUND THEN RETURN FALSE; END IF;

-- Do nothing (success) if counter value is zero or negative 
   IF arg_value <= 0 THEN
      RETURN TRUE;
   END IF;
   
-- Count summary balance before transaction (sum money remainders and debth balance)
   SELECT sum(val) INTO oldbal FROM remainders WHERE acc_id = account.id AND res_id IS NULL;
   oldbal = coalesce(oldbal, 0) + account.balance;

-- Force-free flag is set ...
   IF account.ffree OR gate.ffree THEN
   -- Just Log counter value w/ NULL sum & tariff 
      INSERT INTO eventlog (type, time, result) VALUES (1, 'now', 1);
      IF NOT FOUND THEN RETURN FALSE; END IF;
       
      INSERT INTO translog (event_id, acc_id, gate_id, counter, proto, local, remote, lservid, rservid, balance, res_id)
         VALUES (currval('eventlog_id_seq'), gate.acc_id, gate.id, arg_value, arg_proto, arg_lip,
         arg_rip, arg_lserv, arg_rserv, oldbal, res.id);  
      IF NOT FOUND THEN RETURN FALSE; END IF;

      RETURN TRUE;
   END IF;

-- Initialize value remain
   valremain = arg_value;

-- Repeat while any value remain
   WHILE valremain > 0 LOOP
   -- Recount balance before transaction (not for first loop)
      IF valremain != arg_value THEN
         SELECT sum(val) INTO oldbal FROM remainders WHERE acc_id = account.id AND res_id IS NULL;
         oldbal = coalesce(oldbal, 0) + account.balance;
      END IF; 
   -- Main part (begin-end block for EXIT)
      BEGIN  
      -- First, try to load remainder for given resource (FIFO policy)
         SELECT * INTO remainder FROM remainders WHERE acc_id = account.id AND res_id = res.id ORDER BY id LIMIT 1;
         IF FOUND THEN
         -- Decrease remainder, delete on zero
            IF round(remainder.val) > valremain THEN
            -- Decrease remaider
               UPDATE remainders SET val = val - valremain WHERE id = remainder.id;
               IF NOT FOUND THEN RETURN FALSE; END IF;
            -- Log (counter only, no sum or tariff)
               INSERT INTO eventlog (type, time, result) VALUES (1, 'now', 0);
               IF NOT FOUND THEN RETURN FALSE; END IF;
             
               INSERT INTO translog (event_id, acc_id, gate_id, counter, proto, local, remote, lservid, rservid,
                  balance, res_id)
                  VALUES (currval('eventlog_id_seq'), gate.acc_id, gate.id, valremain, arg_proto, arg_lip,
                  arg_rip, arg_lserv, arg_rserv, oldbal, res.id);
               IF NOT FOUND THEN RETURN FALSE; END IF;
            -- Zero value remain & continue (to leave cycle)
               valremain = 0;
               EXIT;  
            ELSE
            -- Delete remainder  
               DELETE FROM remainders WHERE id = remainder.id;
               IF NOT FOUND THEN RETURN FALSE; END IF;
            -- Log   
               INSERT INTO eventlog (type, time, result) VALUES (1, 'now', 0);
               IF NOT FOUND THEN RETURN FALSE; END IF;

               INSERT INTO translog (event_id, acc_id, gate_id, counter, proto, local, remote, lservid, rservid,
                  balance, res_id)
                  VALUES (currval('eventlog_id_seq'), gate.acc_id, gate.id, remainder.val, arg_proto, arg_lip,
                  arg_rip, arg_lserv, arg_rserv, oldbal, res.id);
               IF NOT FOUND THEN RETURN FALSE; END IF;
            -- Decrease value remain by remainder value & continue
               valremain = valremain - round(remainder.val);
               EXIT;  
            END IF;
         END IF;

      -- No resource remainder, try to load money remainder (FIFO policy)
         SELECT * INTO remainder FROM remainders WHERE acc_id = account.id AND res_id IS NULL ORDER BY id LIMIT 1;
      -- Store result flag
         frem = TRUE;
         IF NOT FOUND THEN frem = FALSE; END IF;

      -- Selecting tariff plan - account plan (default/credit) or remainder plan (if any, if NOT NULL)
         planid = account.plan_id;

         IF account.balance <= 0 AND account.balance > (- account.credit) AND account.cr_plan_id IS NOT NULL THEN
            planid = account.cr_plan_id;
         END IF;

         IF frem THEN
            IF remainder.plan_id IS NOT NULL THEN planid = remainder.plan_id; END IF;
         END IF;

      -- Check tariff plan existence
         IF (SELECT id FROM plans WHERE id = planid AND NOT deleted) IS NULL THEN
            RETURN FALSE;
         END IF;

      -- Select tariff from tariff plan by resource and proto value
         SELECT tariff_id INTO tariffid FROM plan_members AS pm
            WHERE pm.id = ANY (pit4all(ARRAY[planid], ARRAY[res.id], arg_proto));
         IF NOT FOUND THEN RETURN FALSE; END IF;

      -- Load tariff record
         SELECT * INTO tariff FROM tariffs WHERE id = tariffid AND NOT deleted;
         IF NOT FOUND THEN RETURN FALSE; END IF;

      -- Solve with price numenator (find matching alternative w/ hightest priority or use default)
         num = price(rec4alt(alt4time(curtime, ARRAY[tariffid])));
         IF num IS NULL THEN
            num = tariff.price; 
         END IF;

      -- Count cost for value remain 
         cost = (valremain * num) / res.denominator;

      -- Deal with remaider or account debth balance
         IF frem THEN
         -- Decrease money remainder, delete on zero
            IF remainder.val > cost THEN
            -- If cost is not zero - update remainder   
               IF cost > 0 THEN
                  UPDATE remainders SET val = val - cost WHERE id = remainder.id;
                  IF NOT FOUND THEN RETURN FALSE; END IF;
               END IF;
            -- Log
               INSERT INTO eventlog (type, time, result) VALUES (1, 'now', 0);
               IF NOT FOUND THEN RETURN FALSE; END IF;

               INSERT INTO translog (event_id, acc_id, gate_id, counter, proto, local, remote, lservid, rservid, balance,
                  tariff_id, sum, res_id, plan_id)
                  VALUES (currval('eventlog_id_seq'), gate.acc_id, gate.id, valremain, arg_proto, arg_lip,
                  arg_rip, arg_lserv, arg_rserv, oldbal, tariffid, cost, res.id, planid);
               IF NOT FOUND THEN RETURN FALSE; END IF;
            -- Zero value remain & continue
               valremain = 0;
               EXIT;
            ELSE
            -- Delete remainder 
               DELETE FROM remainders WHERE id = remainder.id;
               IF NOT FOUND THEN RETURN FALSE; END IF;
            -- Log
               INSERT INTO eventlog (type, time, result) VALUES (1, 'now', 0);
               IF NOT FOUND THEN RETURN FALSE; END IF;
            -- Count counter value can be bought with remainder sum (to log & decrease vaule remain) 
               tempcount = round(remainder.val * rem.denominator / num);

               INSERT INTO translog (event_id, acc_id, gate_id, counter, proto, local, remote, lservid, rservid, balance,
                         tariff_id, sum, res_id, plan_id)
                  VALUES (currval('eventlog_id_seq'), gate.acc_id, gate.id, tempcount, arg_proto, arg_lip,
                  arg_rip, arg_lserv, arg_rserv, oldbal, tariffid, remainder.val, res.id, planid);
               IF NOT FOUND THEN RETURN FALSE; END IF;
            -- Decrease vaule remain & continue 
               valremain = valremain - tempcount;
               EXIT;  
            END IF;
         ELSE
         -- No remainders, deal with account debth balance
         -- Ensure credit transaction isolation
            IF account.balance > 0 THEN
               last = 0;
            ELSIF account.balance > (-account.credit) THEN
               last = (-account.credit); 
            ELSIF account.balance <= (-account.credit) THEN
               last = 1;  -- special value
            END IF;

            IF last > 0 OR account.balance - cost >= last THEN
               IF cost > 0 THEN
                  UPDATE accs SET balance = balance - cost WHERE id = account.id;
                  IF NOT FOUND THEN RETURN FALSE; END IF;
               END IF;
            -- Log
               INSERT INTO eventlog (type, time, result) VALUES (1, 'now', 0);
               IF NOT FOUND THEN RETURN FALSE; END IF;

               INSERT INTO translog (event_id, acc_id, gate_id, counter, proto, local, remote, lservid, rservid, balance,
                  tariff_id, sum, res_id, plan_id)
                  VALUES (currval('eventlog_id_seq'), gate.acc_id, gate.id, valremain, arg_proto, arg_lip,
                  arg_rip, arg_lserv, arg_rserv, oldbal, tariffid, cost, res.id, planid);
               IF NOT FOUND THEN RETURN FALSE; END IF;
            -- Zero value remain & continue
               valremain = 0;
               EXIT;
            ELSE
               UPDATE accs SET balance = last WHERE id = account.id;
               IF NOT FOUND THEN RETURN FALSE; END IF;

               cost = balance - last;

            -- Log
               INSERT INTO eventlog (type, time, result) VALUES (1, 'now', 0);
               IF NOT FOUND THEN RETURN FALSE; END IF;
            -- Count counter value can be bought with remainder sum (to log & decrease vaule remain)
               tempcount = round(cost * res.denominator / num);

               INSERT INTO translog (event_id, acc_id, gate_id, counter, proto, local, remote, lservid, rservid, balance,
                  tariff_id, sum, res_id, plan_id)
                  VALUES (currval('eventlog_id_seq'), gate.acc_id, gate.id, tempcount, arg_proto, arg_lip,
                  arg_rip, arg_lserv, arg_rserv, oldbal, tariffid, cost, res.id, planid);
               IF NOT FOUND THEN RETURN FALSE; END IF;
            -- Decrease value remain & continue
               valremain = valremain - tempcount;
               EXIT;
            END IF;
         END IF;  
      END;
   END LOOP;

-- Return success
   RETURN TRUE;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;


-- Add new payment
-- PROTO: boolean		payment_new(acc_id bigint[], value float8, plan_id bigint[], res_id bigint[], user_id bigint, session_id bigint)

CREATE OR REPLACE FUNCTION payment_new(bigint[], float8, bigint[], bigint[], bigint, bigint)
  RETURNS boolean AS
$BODY$
DECLARE
   arg_accid  ALIAS FOR $1;
   arg_value  ALIAS FOR $2;
   arg_plan   ALIAS FOR $3;
   arg_res    ALIAS FOR $4;
   arg_userid ALIAS FOR $5;
   arg_sessid ALIAS FOR $6;

   account    RECORD;

   valremain  float8;
BEGIN

-- Trap NULLs
   IF arg_accid IS NULL OR arg_value IS NULL THEN
      RETURN FALSE;   
   END IF;

-- Resource payment cannot have tariff plan
   IF arg_plan IS NOT NULL AND arg_res IS NOT NULL THEN
      RETURN FALSE;
   END IF;

-- Load account
   SELECT * INTO account FROM accs WHERE id = getfirst(arg_accid) AND NOT deleted;
   IF NOT FOUND THEN RETURN FALSE; END IF;

-- Check user id  
   PERFORM id FROM coreauth WHERE id = arg_userid AND NOT deleted;
   IF NOT FOUND THEN RETURN FALSE; END IF;

-- Check session id
   PERFORM id FROM sessionlog WHERE id = arg_sessid;
   IF NOT FOUND THEN RETURN FALSE; END IF;

   IF arg_plan IS NOT NULL THEN
      PERFORM id FROM plans WHERE id = getfirst(arg_plan) AND NOT deleted;
      IF NOT FOUND THEN RETURN FALSE; END IF;
   END IF;

   valremain = arg_value;

   IF arg_res IS NOT NULL THEN
      PERFORM id FROM resources WHERE id = getfirst(arg_res) AND NOT deleted;
      IF NOT FOUND THEN RETURN FALSE; END IF;
   ELSE
      IF account.balance < 0 THEN
         IF account.balance < arg_value THEN
            UPDATE accs SET balance = 0 WHERE id = account.id;
            IF NOT FOUND THEN RETURN FALSE; END IF;
            valremain = valremain + account.balance;
         ELSE
            UPDATE accs SET balance = balance + arg_value WHERE id = account.id;
            IF NOT FOUND THEN RETURN FALSE; END IF;
            valremain = 0; 
         END IF;    
      END IF;
   END IF;

-- Insert value remain onto remainder
   IF valremain > 0 THEN
      INSERT INTO remainders (acc_id, plan_id, val, res_id)
         VALUES (account.id, getfirst(arg_plan), valremain, getfirst(arg_res));
   END IF;   

-- Log payment (NB: w/ original value)
   INSERT INTO eventlog (type, time, result) VALUES (0, 'now', 0);
   IF NOT FOUND THEN RETURN FALSE; END IF;
   
   INSERT INTO paylog (event_id, acc_id, res_id, val, plan_id, user_id, sess_id)
      VALUES(currval('eventlog_id_seq'), account.id, getfirst(arg_res), arg_value,
         getfirst(arg_plan), arg_userid, arg_sessid);
   IF NOT FOUND THEN RETURN FALSE; END IF;

-- Return success
   RETURN TRUE;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;


-- Add new account (w/ default values, unowned) & return id
-- PROTO: bigint		acc_new()

CREATE OR REPLACE FUNCTION acc_new()
  RETURNS int8 AS
$BODY$
BEGIN
   INSERT INTO accs DEFAULT VALUES;
   IF NOT FOUND THEN
      RETURN (-1);
   END IF;

   RETURN currval('accs_id_seq');
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;

-- Return account gates list
-- PROTO: SETOF RECORD		acc_gatelist(acc_id bigint[])

CREATE OR REPLACE FUNCTION acc_gatelist(bigint[])
  RETURNS SETOF RECORD AS
$BODY$
DECLARE
  gate_id bigint;
  res     RECORD;
  rec     RECORD;
BEGIN

   FOR res IN SELECT * FROM resources WHERE NOT deleted LOOP
      FOR rec IN EXECUTE
'SELECT
(SELECT name FROM resources WHERE id = g.res_id) AS res,
CASE g.fgrant WHEN TRUE THEN ''G'' ELSE ''-'' END ||
CASE g.ffree  WHEN TRUE THEN ''E'' ELSE ''-'' END ||
CASE g.fdeny  WHEN TRUE THEN ''D'' ELSE ''-'' END AS flags,
CASE WHEN g.fdeny THEN              ''(DISABLE)''
     WHEN g.fgrant AND g.ffree THEN ''(UNLIMIT)''
     WHEN g.fgrant THEN             ''(FGRANT )''
     WHEN g.ffree  THEN             ''(FFREE  )''
     ELSE                           ''(ENABLED)''
END AS result,
gd.ident::varchar,
gd.ctrl::varchar
FROM gates   AS g, ' ||
(SELECT q_gdata FROM resources WHERE id = res.id) ||
', accs         AS a
WHERE NOT a.deleted AND
g.gdata_id   = gd.id  AND
g.acc_id     = a.id   AND
g.acc_id     = ' || getfirst($1)::text || ' AND
g.res_id     = ' || res.id::text ||
' ORDER by g.id;'
      LOOP
          RETURN NEXT rec;
      END LOOP;
   END LOOP; 

   RETURN;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;


-- Add new gate
-- PROTO: boolean		gate_new(res_id bigint[], acc_id bigint[], ident varchar, ctrl varchar)

CREATE OR REPLACE FUNCTION gate_new(bigint[], bigint[], varchar, varchar)
  RETURNS bool AS
$BODY$
DECLARE
   arg_res    ALIAS FOR $1;
   arg_accid  ALIAS FOR $2;
   arg_ident  ALIAS FOR $3;
   arg_ctrl   ALIAS FOR $4;

   resource RECORD;
   rec      RECORD;
   query    text;
   expr     text;
   expr2    text;
BEGIN
-- Trap NULLs
   IF arg_res IS NULL OR arg_accid IS NULL OR arg_ident IS NULL THEN
      RAISE NOTICE 'NULL arguments';
      RETURN FALSE;
   END IF;
   
-- Load resource record
   SELECT * INTO resource FROM resources WHERE id = getfirst(arg_res) AND NOT deleted;
   IF NOT FOUND THEN 
      RAISE NOTICE 'Invalid resource';
      RETURN FALSE; 
   END IF;

-- Check account id
   PERFORM id FROM accs WHERE id = getfirst(arg_accid) AND NOT deleted;
   IF NOT FOUND THEN 
      RAISE NOTICE 'Invalid account';
      RETURN FALSE; 
   END IF;

-- Test gate data duplication (inclusion)
   expr  = replace(resource.q_cmpident, '$2', quote_literal(arg_ident));
   expr  = replace(expr, '$1', 'gd.ident');
   expr2 = replace(resource.q_cmpident, '$2', 'gd.ident');
   expr2 = replace(expr2, '$1', quote_literal(arg_ident));
	
   FOR rec IN EXECUTE 'SELECT gd.id FROM ' || resource.q_gdata ||
                      ' WHERE ' || expr || ' OR ' || expr2 || ' ;'
   LOOP
      EXIT;
   END LOOP;

   IF rec.id IS NOT NULL THEN
      RAISE NOTICE 'Duplicate data exist';
      RETURN FALSE;
   END IF;

-- Add gate data record
   query = resource.q_gdcreate;

   query = replace(query, '$1', quote_literal(arg_ident));
   query = replace(query, '$2', quote_literal(coalesce(arg_ctrl, arg_ident)));

-- Execute gatedata create query
   EXECUTE query;

-- Get gatedata id (STUB, search for _equal_ ident)
   FOR rec IN EXECUTE 'SELECT gd.id FROM ' || resource.q_gdata ||
                      ' WHERE gd.ident = ' || quote_literal(arg_ident) ||
                      ' ;'
   LOOP
      EXIT;
   END LOOP;

   IF rec.id IS NULL THEN
      RAISE NOTICE 'Data was not added';
      RETURN FALSE;
   END IF;

-- Add gate record
   INSERT INTO gates (acc_id, res_id, gdata_id) VALUES (getfirst(arg_accid), resource.id, rec.id);
   IF NOT FOUND THEN
      RAISE NOTICE 'Gate was not added';
      RETURN FALSE; 
   END IF;

   RETURN TRUE;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;

-- Delete gate 
-- PROTO: boolean		gate_del(gate_id bigint[])

CREATE OR REPLACE FUNCTION gate_del(bigint[])
  RETURNS bool AS
$BODY$
DECLARE
   arg_gateid  ALIAS FOR $1;

   gate      RECORD;
   resource  RECORD;

   query     text;
BEGIN
-- Trap NULLs
   IF arg_gateid IS NULL THEN
      RETURN FALSE;
   END IF;
   
-- Load gate
   SELECT * INTO gate FROM gates WHERE id = getfirst(arg_gateid);
   IF NOT FOUND THEN RETURN FALSE; END IF;

-- Load resource record
   SELECT * INTO resource FROM resources WHERE id = gate.res_id AND NOT deleted;
   IF NOT FOUND THEN RETURN FALSE; END IF;

-- Delete gate data
   query = replace(resource.q_gddrop, '$1', (gate.gdata_id)::text);  
   EXECUTE query;

-- Delete gate
   DELETE FROM gates WHERE id = gate.id;

   RETURN TRUE;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;


-- Modify gate w/ given SET expression
-- PROTO: boolean		gate_mod(gate_id bigint[], setexpt varchar)

CREATE OR REPLACE FUNCTION gate_mod(bigint[], varchar)
  RETURNS bool AS
$BODY$
DECLARE
   arg_gateid  ALIAS FOR $1;
   arg_expr    ALIAS FOR $2;
BEGIN
-- Trap NULLs
   IF arg_gateid IS NULL OR arg_expr IS NULL THEN
      RETURN FALSE;
   END IF;
   
-- Execute query w/ expression
   EXECUTE 'UPDATE gates SET ' || arg_expr || ' WHERE id = ' || (getfirst(arg_gateid))::text || ' ;';

   RETURN TRUE;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;

-- (in design) Native card utilization
-- PROTO: double precision	pcard_utluser(card_id int8, pin int8, host cidr)

CREATE OR REPLACE FUNCTION pcard_utluser(int8, int8, cidr)
  RETURNS double precision AS
$BODY$
DECLARE
   arg_cardid  ALIAS FOR $1;
   arg_pin     ALIAS FOR $2;
   arg_host    ALIAS FOR $3;

   sum   double precision;
   n     integer;
   u     integer;
   slip  RECORD;
BEGIN
-- Check faults number for given host
   SELECT count(*) INTO n 
      FROM eventlog AS el, pcardslog AS pcl
      WHERE
      el.id     = pcl.event_id AND
      el.type   = 4            AND
      el.result = (-1)         AND
      pcl.host  = arg_host     AND
      age(el.time) <= interval '1 day';
   SELECT count(*) INTO u
      FROM eventlog AS el, pcardslog AS pcl
      WHERE
      el.id      = pcl.event_id AND
      el.type    = 4            AND
      el.result  = 0            AND
      pcl.host   = arg_host     AND
      pcl.action = 105          AND
      age(el.time) <= interval '1 day';
   IF (n - u * 10) >= 10 THEN
      RETURN (-5);
   END IF;

-- Load gate (STUB: inet gate) by host address
   SELECT * INTO gate FROM gates WHERE id = gate_hit('inet', arg_host) AND NOT fdeny;
   IF NOT FOUND THEN
      RETURN (-3);
   END IF;

   SELECT * INTO account FROM accs WHERE id = gate.acc_id AND NOT deleted;
   IF NOT FOUND THEN
      RETURN (-3);
   END IF;
   
-- Fail if unlimited
   IF account.ffree THEN
      RETURN (-8);
   END IF;

-- Fail if frozen
   IF account.frozen THEN
      RETURN (-7);
   END IF;   

-- Check card & get its copy
   SELECT * INTO slip FROM paycards WHERE id = arg_cardid AND pin = arg_pin AND printed;
   IF NOT FOUND THEN
   -- Log fault
      INSERT INTO eventlog  (time, type, result) VALUES ('now', 4, (-1));
      INSERT INTO pcardslog (event_id, action, card_id, pin, val, host) VALUES (
         currval('eventlog_id_seq'), 203, arg_cardid, arg_pin, 0, arg_host);
   -- Return error          
       RETURN (-2);
   END IF;

-- Destroy card
   DELETE FROM paycards WHERE id = arg_cardid;

-- Log card event
   INSERT INTO eventlog  (time, type, result) VALUES ('now', 4, 0);
   INSERT INTO pcardslog (event_id, action, card_id, pin, val, res_id, plan_id, host, batchno) VALUES (
      currval('eventlog_id_seq'), 203,
      arg_cardid, arg_pin, slip.val, slip.res_id, slip.plan_id, arg_host, slip.batchno);

--   payment_new(account.id, slip.val, "varchar", "varchar", , int8)

-- Return positive sum
   RETURN slip.val;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;

-- Utilize card w/ number & PIN-code (by user interface)
-- BEEv0 assistance version
-- PROTO: double precision	pcard_utluser_v0(card_id bigint, bigint, cidr)

CREATE OR REPLACE FUNCTION pcard_utluser_v0(int8, int8, cidr)
  RETURNS float8 AS
$BODY$
DECLARE
   sum   double precision;
   n     integer;
   u     integer;
   slip  RECORD;
BEGIN
-- Check faults number for given host
   SELECT count(*) INTO n 
      FROM eventlog AS el, pcardslog AS pcl
      WHERE
      el.id     = pcl.event_id AND
      el.type   = 4            AND
      el.result = (-1)         AND
      pcl.host  = $3           AND
      age(el.time) <= interval '1 day';
   SELECT count(*) INTO u
      FROM eventlog AS el, pcardslog AS pcl
      WHERE
      el.id      = pcl.event_id AND
      el.type    = 4            AND
      el.result  = 0            AND
      pcl.host   = $3           AND
      pcl.action = 105          AND
      age(el.time) <= interval '1 day';
   IF (n - u * 10) >= 10 THEN
      RETURN (-5);
   END IF;

-- Check card & get its copy
   SELECT * INTO slip FROM paycards WHERE id = $1 AND pin = $2 AND printed;
   IF NOT FOUND THEN
   -- Log fault
      INSERT INTO eventlog  (time, type, result) VALUES ('now', 4, (-1));
      INSERT INTO pcardslog (event_id, action, card_id, pin, val, host, batchno, barcode) VALUES (
         currval('eventlog_id_seq'), 203, $1, $2, 0, $3);
   -- Return error          
       RETURN (-2);
   END IF;

-- Fault resource-type cards
   IF slip.res_id IS NOT NULL THEN
       RETURN (-10);
   END IF;

-- Destroy card
   DELETE FROM paycards WHERE id = $1;

-- Log card event
   INSERT INTO eventlog  (time, type, result) VALUES ('now', 4, 0);
   INSERT INTO pcardslog (event_id, action, card_id, pin, val, host, batchno, barcode) VALUES (
      currval('eventlog_id_seq'), 203,
      $1, $2, slip.val, $3, slip.batchno, slip.barcode);

-- Return positive sum
   RETURN slip.val;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;

-- Add new single card
-- PROTO: boolean		pcard_new (value, pin, barcode, days, batch, res_id)                                  

CREATE OR REPLACE FUNCTION pcard_new(float8, int8, int8, int4, int8, int8)
  RETURNS bool AS
$BODY$
BEGIN
   INSERT INTO paycards (val, pin, barcode, gen_time, expr_time, batchno, res_id)
      VALUES ($1, $2, $3, 'now', 'now'::timestamp + ('1 day'::interval * $4),
      $5, $6);

   INSERT INTO eventlog (time, type, result) 
      VALUES ('now', 4, 0);

   INSERT INTO pcardslog (event_id, action, card_id, pin, barcode, val, res_id, batchno)
      VALUES (currval('eventlog_id_seq'), 100, currval('paycards_id_seq'),
      $2, $3, $1, $6, $5);

   RETURN TRUE;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;

-- Emit cards batch
-- PROTO: boolean		batch_emit (batch_id bigint)

CREATE OR REPLACE FUNCTION batch_emit(int8)
  RETURNS bool AS
$BODY$
DECLARE
   card RECORD;
BEGIN
   FOR card IN SELECT * FROM paycards WHERE batchno = $1 AND NOT printed ORDER BY id LOOP
      UPDATE paycards SET printed = TRUE WHERE id = card.id;

      INSERT INTO eventlog (time, type, result) 
         VALUES ('now', 4, 0);

      INSERT INTO pcardslog (event_id, action, card_id, pin, barcode, val, res_id, batchno)
         VALUES (currval('eventlog_id_seq'), 101, card.id, card.pin, card.barcode,
         card.val, card.res_id, card.batchno);
   END LOOP;   

   RETURN TRUE;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;

-- Annul card
-- PROTO: boolean		pcard_null(card_id bigint)

CREATE OR REPLACE FUNCTION pcard_null(int8)
  RETURNS bool AS
$BODY$
DECLARE
   card RECORD;
BEGIN
   FOR card IN SELECT * FROM paycards WHERE id = $1 ORDER BY id LOOP
      DELETE FROM paycards WHERE id = card.id;

      INSERT INTO eventlog (time, type, result) 
         VALUES ('now', 4, 0);

      INSERT INTO pcardslog (event_id, action, card_id, pin, barcode, val, res_id, batchno)
         VALUES (currval('eventlog_id_seq'), 104, card.id, card.pin, card.barcode,
         card.val, card.res_id, card.batchno);
   END LOOP;   

   RETURN TRUE;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;

-- Annul cards batch
-- PROTO: boolean		batch_null(batch_id)

CREATE OR REPLACE FUNCTION batch_null(int8)
  RETURNS bool AS
$BODY$
DECLARE
   card RECORD;
BEGIN
   FOR card IN SELECT * FROM paycards WHERE batchno = $1 ORDER BY id LOOP
      DELETE FROM paycards WHERE id = card.id;

      INSERT INTO eventlog (time, type, result) 
         VALUES ('now', 4, 0);

      INSERT INTO pcardslog (event_id, action, card_id, pin, barcode, val, res_id, batchno)
         VALUES (currval('eventlog_id_seq'), 104, card.id, card.pin, card.barcode,
         card.val, card.res_id, card.batchno);
   END LOOP;   

   RETURN TRUE;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;

-- Annul expired cards (to call periodically for automatic expiration)
-- PROTO: boolean		pcards_expire()

CREATE OR REPLACE FUNCTION pcards_expire()
  RETURNS bool AS
$BODY$
DECLARE
   card RECORD;
BEGIN
   FOR card IN SELECT * FROM paycards WHERE expr_time < 'now'::timestamptz ORDER BY id LOOP
      DELETE FROM paycards WHERE id = card.id;

      INSERT INTO eventlog (time, type, result) 
         VALUES ('now', 4, 0);

      INSERT INTO pcardslog (event_id, action, card_id, pin, barcode, val, res_id, batchno)
         VALUES (currval('eventlog_id_seq'), 4, card.id, card.pin, card.barcode,
         card.val, card.res_id, card.batchno);
   END LOOP;   

   RETURN TRUE;
END;
$BODY$
  LANGUAGE 'plpgsql' VOLATILE;

-- Create new paycards batch
-- PROTO: bigint		batch_new(comment)

CREATE OR REPLACE FUNCTION batch_new(varchar)
  RETURNS bigint AS
$BODY$
BEGIN
   INSERT INTO pcard_batches (gen_time, comment)
      VALUES ('now', $1);

   RETURN currval('pcard_batches_id_seq');
END;
$BODY$
LANGUAGE 'plpgsql' VOLATILE;

COMMIT TRANSACTION;

