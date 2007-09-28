-- $Id$

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 *									     *
 * (english)                 !!! ATTENTION !!!                               *
 * This script COMPLETELY DESTROYS all data and structure work database,     *
 * desined for debug puproses and SHOULD NOT be invoked in normal operation  *
 * and/or on "live" data.						     *
 *									     *
 * (russian, koi8-r)          !!! ВНИМАНИЕ !!!                               *
 * Этот скрипт ПОЛНОСТЬЮ УНИЧТОЖАЕТ все данные и структуру рабочей базы      *
 * данных, создан для отладки и НЕ ДОЛЖЕН выполняться при нормальной работе  *
 * и/или на "живых" данных.						     *
 *									     *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

BEGIN TRANSACTION;

-- Destroy functions

DROP FUNCTION cards_expire();
DROP FUNCTION cardbatch_null(bigint);
DROP FUNCTION card_null(bigint);
DROP FUNCTION cardbatch_emit(bigint);
DROP FUNCTION card_new (double precision, bigint, bigint, integer, bigint, bigint);
DROP FUNCTION card_utilize_user_v0(bigint, bigint, cidr);
-- DROP FUNCTION card_utluser(int8, int8, cidr);
DROP FUNCTION gate_mod(bigint[], varchar);
DROP FUNCTION gate_del(bigint[]);
DROP FUNCTION gate_new(bigint[], bigint[], varchar, varchar);
DROP FUNCTION acc_gatelist(bigint[]);
DROP FUNCTION acc_new();
DROP FUNCTION payment_new(bigint[], float8, bigint[], bigint[], bigint, bigint);
DROP FUNCTION res_transaction(bigint[], bigint, bigint, inet, inet, integer, integer);
DROP FUNCTION res_access_table(bigint[]);
DROP FUNCTION rec4batch(bigint[]);
DROP FUNCTION batch4expr(varchar, bigint[]);
DROP FUNCTION batch4expr(varchar);
DROP FUNCTION batches();
DROP FUNCTION rec4pcard(bigint[]);
DROP FUNCTION pcard4expr(varchar, bigint[]);
DROP FUNCTION pcard4expr(varchar);
DROP FUNCTION pcard4res(bigint[], bigint[]);
DROP FUNCTION pcard4res(bigint[]);
DROP FUNCTION pcard4batch(bigint[], bigint[]);
DROP FUNCTION pcard4batch(bigint[]);
DROP FUNCTION pcards();
DROP FUNCTION vrec4gate(bigint[]);
DROP FUNCTION rec4gate(bigint[]);
DROP FUNCTION gate4expr(varchar, bigint[]);
DROP FUNCTION gate4expr(varchar);
DROP FUNCTION gate4ident_hit(bigint[], sample varchar, bigint[]);
DROP FUNCTION gate4ident_hit(bigint[], varchar);
DROP FUNCTION gate4ident_strict(bigint[], sample varchar, bigint[]);
DROP FUNCTION gate4ident_strict(bigint[], sample varchar);
DROP FUNCTION gate4res(bigint[], bigint[]);
DROP FUNCTION gate4res(bigint[]);
DROP FUNCTION gate4acc(bigint[], bigint[]);
DROP FUNCTION gate4acc(bigint[]);
DROP FUNCTION gates();
DROP FUNCTION rec4rem(bigint[]);
DROP FUNCTION rem4expr(varchar, bigint[]);
DROP FUNCTION rem4expr(varchar);
DROP FUNCTION rem4accres(bigint[], bigint[], bigint[]);
DROP FUNCTION rem4accres(bigint[], bigint[]);
DROP FUNCTION rem4res(bigint[], bigint[]);
DROP FUNCTION rem4res(bigint[]);
DROP FUNCTION rem4acc(bigint[], bigint[]);
DROP FUNCTION rem4acc(bigint[]);
DROP FUNCTION rem4plan(bigint[], bigint[]);
DROP FUNCTION rem4plan(bigint[]);
DROP FUNCTION rems();
DROP FUNCTION rec4acc(bigint[]);
DROP FUNCTION acc4expr(varchar, bigint[]);
DROP FUNCTION acc4expr(varchar);
DROP FUNCTION acc4label(varchar, bigint[]);
DROP FUNCTION acc4label(varchar);
DROP FUNCTION acc4group(bigint[], bigint[]);
DROP FUNCTION acc4group(bigint[]);
DROP FUNCTION acc4person(bigint[], bigint[]);
DROP FUNCTION acc4person(bigint[]);
DROP FUNCTION acc4crplan(bigint[], bigint[]);
DROP FUNCTION acc4crplan(bigint[]);
DROP FUNCTION acc4plan(bigint[], bigint[]);
DROP FUNCTION acc4plan(bigint[]);
DROP FUNCTION accs();
DROP FUNCTION rec4person(bigint[]);
DROP FUNCTION person4expr(varchar, bigint[]);
DROP FUNCTION person4expr(varchar);
DROP FUNCTION persons();
DROP FUNCTION rec4pit(bigint[]);
DROP FUNCTION pit4expr(varchar, bigint[]);
DROP FUNCTION pit4expr(varchar);
DROP FUNCTION pit4all(bigint[], bigint[], bigint, bigint[]);
DROP FUNCTION pit4all(bigint[], bigint[], bigint);
DROP FUNCTION pit4proto(bigint[], bigint, bigint[]);
DROP FUNCTION pit4proto(bigint[], bigint);
DROP FUNCTION pit4plan(bigint[], bigint[]);
DROP FUNCTION pit4plan(bigint[]);
DROP FUNCTION pits();
DROP FUNCTION rec4alt(bigint[]);
DROP FUNCTION alt4expr(varchar, bigint[]);
DROP FUNCTION alt4expr(varchar);
DROP FUNCTION alt4time(timestamptz, bigint[], bigint[]);
DROP FUNCTION alt4time(timestamptz, bigint[]);
DROP FUNCTION alt4tariff(bigint[], bigint[]);
DROP FUNCTION alt4tariff(bigint[]);
DROP FUNCTION alts();
DROP FUNCTION rec4tariff(bigint[]);
DROP FUNCTION tariff4expr(varchar, bigint[]);
DROP FUNCTION tariff4expr(varchar);
DROP FUNCTION tariff4res(bigint[], bigint[]);
DROP FUNCTION tariff4res(bigint[]);
DROP FUNCTION tariffs();
DROP FUNCTION rec4plan(bigint[]);
DROP FUNCTION plan4expr(varchar, bigint[]);
DROP FUNCTION plan4expr(varchar);
DROP FUNCTION plan4name(varchar, bigint[]);
DROP FUNCTION plan4name(varchar);
DROP FUNCTION plans();
DROP FUNCTION rec4user(bigint[]);
DROP FUNCTION user4expr(varchar, bigint[]);
DROP FUNCTION user4expr(varchar);
DROP FUNCTION user4name(varchar, bigint[]);
DROP FUNCTION user4name(varchar);
DROP FUNCTION users();
DROP FUNCTION rec4group(bigint[]);
DROP FUNCTION group4expr(varchar, bigint[]);
DROP FUNCTION group4expr(varchar);
DROP FUNCTION group4name(varchar, bigint[]);
DROP FUNCTION group4name(varchar);
DROP FUNCTION groups();
DROP FUNCTION rec4res(bigint[]);
DROP FUNCTION res4expr(varchar, bigint[]);
DROP FUNCTION res4expr(varchar);
DROP FUNCTION res4name(varchar, bigint[]);
DROP FUNCTION res4name(varchar);
DROP FUNCTION ress();
DROP FUNCTION getfirst(bigint[]);

-- Destroy types

DROP TYPE access_row;
DROP TYPE vgate;

-- Destroy tables

TRUNCATE   pcardslog;
DROP TABLE pcardslog;
TRUNCATE   pcard_actions;
DROP TABLE pcard_actions;
TRUNCATE   paycards;
DROP TABLE paycards;
TRUNCATE   pcard_batches;
DROP TABLE pcard_batches;
TRUNCATE   loginlog;
DROP TABLE loginlog;
TRUNCATE   paylog;
DROP TABLE paylog;
TRUNCATE   sessionlog;
DROP TABLE sessionlog;
TRUNCATE   translog;
DROP TABLE translog;
TRUNCATE   eventlog;
DROP TABLE eventlog;
TRUNCATE   gates;
DROP TABLE gates;
TRUNCATE   remainders;
DROP TABLE remainders;
TRUNCATE   group_members;
DROP TABLE group_members;
TRUNCATE   groups;
DROP TABLE groups;
TRUNCATE   accs;
DROP TABLE accs;
TRUNCATE   tariff_nums;
DROP TABLE tariff_nums;
TRUNCATE   plan_members;
DROP TABLE plan_members;
TRUNCATE   tariffs;
DROP TABLE tariffs;
TRUNCATE   plans;
DROP TABLE plans;
TRUNCATE   persons;
DROP TABLE persons;
TRUNCATE   resources;
DROP TABLE resources;
TRUNCATE   coreauth;
DROP TABLE coreauth;

TRUNCATE   inet_gatedata;
DROP TABLE inet_gatedata;

COMMIT TRANSACTION;

