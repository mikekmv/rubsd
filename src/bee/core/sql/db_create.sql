-- $Id$

\set VERBOSITY terse

BEGIN TRANSACTION;

-- Core auth data

CREATE TABLE coreauth
(  id           bigserial  PRIMARY KEY,
   deleted      bool       NOT NULL DEFAULT FALSE,
   name         varchar    NOT NULL,               /* user name           */
   authkey      varchar    NOT NULL,               /* user key (password) */
   permask      bigint     NOT NULL DEFAULT 0      /* permissions mask    */
) WITHOUT OIDS;

-- Resources

CREATE TABLE resources
(  id           bigserial  PRIMARY KEY,
   deleted      bool       NOT NULL DEFAULT FALSE,
   name	        varchar    NOT NULL UNIQUE,          /* resource name  */
   descr        varchar    NOT NULL,                 /* description    */
   denominator  double precision NOT NULL DEFAULT 1, /* price denominator */
   time_shift   interval   NOT NULL DEFAULT '0',     /* time correction interval */
   q_gdata      varchar    NOT NULL,                 /* query, "view" for SELECT FROM */
   q_cmpident   varchar    NOT NULL,                 /* expression to compare ident with sample */
   q_gdcreate   varchar    NOT NULL,                 /* query, create/link gate data */
   q_gddrop     varchar,                             /* query, destroy gate data     */
   rulegen      varchar                              /* rules generator script path  */
) WITHOUT OIDS;

-- Persons (STUB)

CREATE TABLE persons
(  id           bigserial  PRIMARY KEY,
   deleted      bool       NOT NULL DEFAULT FALSE,    
   nickname     varchar    NOT NULL UNIQUE,        /* short nickname      */
   name         varchar,                           /* STUB: first   name  */
   midname      varchar,                           /* STUB: middle  name  */
   family       varchar,                           /* STUB: last    name  */
   company      varchar                            /* STUB: company name  */
) WITHOUT OIDS;

-- Tariff plans (tariff sets)

CREATE TABLE plans
(  id           bigserial  PRIMARY KEY, 
   deleted      bool       NOT NULL DEFAULT FALSE,
   name         varchar    NOT NULL UNIQUE,  /* tariff plan name */
   descr        varchar    NOT NULL          /* description */
) WITHOUT OIDS;

-- Tariffs

CREATE TABLE tariffs
(  id           bigserial  PRIMARY KEY,
   res_id       bigint     NOT NULL REFERENCES resources (id), /* resource id        */
   deleted      bool       NOT NULL DEFAULT FALSE,
   fgrant       bool       NOT NULL DEFAULT FALSE,     /* force grant gates for resource */
   price        double precision NOT NULL DEFAULT 0    /* price - numinator (default) */
) WITHOUT OIDS;

-- Tariff plans membership

CREATE TABLE plan_members
(  id           bigserial  PRIMARY KEY,
   plan_id      bigint     NOT NULL REFERENCES plans     (id), /* tariff set id       */
   tariff_id    bigint     NOT NULL REFERENCES tariffs   (id), /* member (tariff) id  */
   proto_mask   bigint     NOT NULL,                           /* protocol AND mask   */
   proto_test   bigint     NOT NULL,                           /* protocol test value */
   priority     integer    NOT NULL CHECK (priority > 0),      /* plan item priority  */
                           UNIQUE (plan_id, priority)
) WITHOUT OIDS;

-- Alternative tariff numinators

CREATE TABLE tariff_nums
(  id           bigserial  PRIMARY KEY,
   tariff_id    bigint     NOT NULL REFERENCES tariffs (id),  /* parent tariff  */
   price        double precision NOT NULL,                    /* alt. numinator */
   priority     integer    NOT NULL CHECK (priority > 0),
                           UNIQUE (tariff_id, priority),
   time_from    timestamptz,                                  /* date/time period  */
   time_to      timestamptz,                                  /* --//--            */
   daytime_from timetz,                                       /* daytime period    */
   daytime_to   timetz,                                       /* --//--            */
   weekday_from integer    CHECK (weekday_from >=0 AND weekday_from < 7), /* weekday period (0-6, 0-Sunday) */
   weekday_to   integer    CHECK (weekday_to >=0 AND weekday_to < 7)      /* --//-- (including given)       */
) WITHOUT OIDS;

-- Accounts

CREATE TABLE accs
(  id           bigserial  PRIMARY KEY,                      /* account number ?    */
   deleted      bool       NOT NULL DEFAULT FALSE,
   person_id    bigint     REFERENCES persons (id),          /* owner person        */
   frozen       bool       NOT NULL DEFAULT FALSE,           /* account is frozen   */
   paused       bool       NOT NULL DEFAULT FALSE,           /* account is paused   */
   fgrant       bool       NOT NULL DEFAULT FALSE,           /* force grant gates   */
   ffree        bool       NOT NULL DEFAULT FALSE,           /* force free          */
   balance      double precision NOT NULL DEFAULT 0,         /* debt balance        */
   credit       double precision NOT NULL DEFAULT 0,         /* maximum credit      */
   plan_id      bigint     NOT NULL REFERENCES plans (id) DEFAULT 2, /* default tariff plan */
   cr_plan_id   bigint,                                      /* credit tariff plan  */
   label        varchar                                      /* custom text marks   */
) WITHOUT OIDS;

-- Account groups

CREATE TABLE groups
(  id           bigserial  PRIMARY KEY,
   name         varchar    NOT NULL UNIQUE,         /* group name  */
   descr        varchar    NOT NULL                 /* description */ 
) WITHOUT OIDS;

-- Accounts group membership

CREATE TABLE group_members
(  group_id     bigint     NOT NULL REFERENCES groups (id),  /* group id   */
   acc_id       bigint     NOT NULL REFERENCES accs   (id),  /* account id */
                           UNIQUE (group_id, acc_id)
) WITHOUT OIDS;

-- Remainders (account balance parts)

CREATE TABLE remainders
(  id           bigserial,                                 /* remainder id */
   acc_id       bigint     NOT NULL REFERENCES accs (id),  /* parent account             */
   plan_id      bigint     REFERENCES plans     (id),      /* NULL or alt. tariff plan   */
   val          double precision NOT NULL,                 /* res.count or money         */
   res_id       bigint     REFERENCES resources (id)       /* resource or NULL for money */
) WITHOUT OIDS;

-- Resourсe gates

CREATE TABLE gates
(  id           bigserial  PRIMARY KEY,
   fdeny        bool       NOT NULL DEFAULT FALSE,              /* force deny  access    */
   fgrant       bool       NOT NULL DEFAULT FALSE,              /* force grant access    */
   ffree        bool       NOT NULL DEFAULT FALSE,              /* force free            */
   acc_id       bigint     NOT NULL REFERENCES accs      (id),  /* parent account        */
   res_id       bigint     NOT NULL REFERENCES resources (id),  /* resource id           */
   gdata_id     bigint     NOT NULL,                            /* ident data key  */
   UNIQUE(gdata_id, res_id)
) WITHOUT OIDS;

-- Event log root
CREATE TABLE eventlog
(  id           bigserial   PRIMARY KEY,
   link_id      bigint      REFERENCES eventlog  (id),
   time         timestamptz NOT NULL DEFAULT current_timestamp, /* timestampt  */
   type         integer     NOT NULL,               /* event type  */
   result       integer     NOT NULL DEFAULT 0      /* result code */
) WITHOUT OIDS;

-- Transaction log

CREATE TABLE translog
(  event_id     bigint     NOT NULL REFERENCES eventlog  (id),  /* event id            */   
   res_id       bigint     NOT NULL REFERENCES resources (id),  /* resource id         */
   acc_id       bigint     NOT NULL REFERENCES accs      (id),  /* account id          */
   gate_id      bigint     NOT NULL,                            /* gate id             */
   plan_id      bigint     REFERENCES plans     (id),           /* effective plan id   */
   tariff_id    bigint     REFERENCES tariffs   (id),           /* effective tariff id */
   counter      bigint     NOT NULL,                            /* resource count      */
   sum          double precision,                               /* sum (if any)        */
   proto        bigint     NOT NULL,                            /* resource subcode    */
   local        inet,                                           /* local  inet address */
   remote       inet,                                           /* remote inet address */
   lservid      integer,                                        /* ? "local  service id" */
   rservid      integer,                                        /* ? "remote service id" */
   balance      double precision                                /* summary money balance before transaction */
) WITHOUT OIDS;

-- Sessions log

CREATE TABLE sessionlog
(  id           bigserial  PRIMARY KEY,
   deleted      bool       NOT NULL DEFAULT FALSE,
   event_id     bigint     NOT NULL REFERENCES eventlog  (id),  /* event id            */   
   type         integer    NOT NULL,                            /* client type         */
   peer         inet       NOT NULL                             /* client address      */  
) WITHOUT OIDS;

-- Payments log

CREATE TABLE paylog
(  event_id     bigint     NOT NULL REFERENCES eventlog   (id), /* event id           */
   acc_id       bigint     NOT NULL REFERENCES accs       (id), /* account id         */
   res_id       bigint     REFERENCES resources           (id), /* resource or NULL   */
   val          double precision NOT NULL,                      /* res.count or money */
   plan_id      bigint     REFERENCES plans               (id), /* eff. tariff plan   */
   user_id      bigint     NOT NULL REFERENCES coreauth   (id), /* operator user id   */
   sess_id      bigint     NOT NULL REFERENCES sessionlog (id)  /* session id         */
) WITHOUT OIDS;

-- Log-ins log

CREATE TABLE loginlog
(  event_id    bigint   NOT NULL REFERENCES eventlog   (id), /* event id   */
   user_id     bigint   NOT NULL REFERENCES coreauth   (id), /* user id    */
   sess_id     bigint   NOT NULL REFERENCES sessionlog (id)  /* session id */
) WITHOUT OIDS;

-- "inet" default gate data table

CREATE TABLE inet_gatedata
(  id          bigserial  PRIMARY KEY,         /* item id                     */
   ident       cidr       NOT NULL UNIQUE      /* client host/network address */
) WITHOUT OIDS;

-- Pay-cards list

CREATE TABLE pcard_batches
(  id          bigserial    PRIMARY KEY,       /* batch number    */
   gen_time    timestamptz   NOT NULL,          /* generation time */
   comment     varchar                         /* comment         */
) WITHOUT OIDS;

CREATE TABLE paycards
(  id          bigserial    PRIMARY KEY,              /* card no              */
   printed     bool         NOT NULL DEFAULT FALSE,   /* printed & enabled    */
   pin         bigint       NOT NULL UNIQUE,          /* card PIN             */
   val         double precision NOT NULL,             /* sum                  */
   barcode     bigint       NOT NULL UNIQUE,          /* check code           */ 
   res_id      bigint       REFERENCES resources(id), /* resource id          */
   plan_id     bigint       REFERENCES plans    (id), /* explicit tariff plan id */
   batchno     bigint       NOT NULL REFERENCES pcard_batches (id), /* cards batch number   */
   gen_time    timestamptz  NOT NULL,                 /* generation date/time */
   expr_time   timestamptz  NOT NULL                  /* expiration date/time */
) WITHOUT OIDS;

-- Pay-cards actions book

CREATE TABLE pcard_actions
(  code        integer      PRIMARY KEY,
   descr       varchar      NOT NULL
) WITHOUT OIDS;     

-- Pay-cards log

CREATE TABLE pcardslog
(  event_id    bigint   NOT NULL REFERENCES eventlog (id),        /* event id        */
   action      integer  NOT NULL REFERENCES pcard_actions (code), /* action          */
   batchno     bigint   NOT NULL,
   card_id     bigint   NOT NULL,                                 /* card no         */
   pin         bigint   NOT NULL,                                 /* PIN code        */
   barcode     bigint   NOT NULL,                                 /* check code      */
   val         double precision NOT NULL,                         /* sum or value    */  
   res_id      bigint   REFERENCES resources (id),                /* resource        */
   plan_id     bigint   REFERENCES plans     (id),                /* tariff plan     */
   host        inet                                               /* STUB, user host */
) WITHOUT OIDS;

-- Create types

CREATE TYPE access_row AS (ctrl varchar, result bool);

-- virtual gate view

CREATE TYPE vgate AS
(  id bigint,        /* gate id              */
   fdeny  bool,      /* "gate disabled" flag */
   fgrant bool,      /* force grant flag     */
   ffree  bool,      /* force free flag      */
   acc_id bigint,    /* account id           */
   res_id bigint,    /* resource id          */
   ident  varchar,   /* gate ident           */ 
   ctrl   varchar    /* gate control data    */ 
);

-- Add default records

INSERT INTO coreauth (name, authkey, permask)
VALUES ('admin', '123456', 2147483648);

INSERT INTO resources (name, descr, denominator, time_shift, q_gdata, q_gdcreate, q_gddrop, q_cmpident)
VALUES ('inet', 'доступ в Интернет', 1024 * 1024, '5 minutes',
'(SELECT igd.id, igd.ident, igd.ident AS ctrl FROM inet_gatedata AS igd) AS gd',
'INSERT INTO inet_gatedata (ident) VALUES ($1);',
'DELETE FROM inet_gatedata WHERE id = $1;',
'$1 >>= $2'
);

INSERT INTO groups (name, descr) VALUES
('payman',   'управляется через payman');

INSERT INTO groups (name, descr) VALUES
('servers',  'собственные сервера');

INSERT INTO groups (name, descr) VALUES
('internal', 'внутренние пользователи');

INSERT INTO groups (name, descr) VALUES
('natural',  'физические лица');

INSERT INTO groups (name, descr) VALUES
('juro',     'юридические лица');

INSERT INTO groups (name, descr) VALUES
('vpn',      'пользователи VPN');

INSERT INTO groups (name, descr) VALUES
('leased',   'выделенщики');

INSERT INTO tariffs (res_id, fgrant) VALUES
(  (SELECT id FROM resources WHERE name   = 'inet'),
   TRUE
);

INSERT INTO tariffs (res_id, price) VALUES
(  (SELECT id FROM resources WHERE name   = 'inet'),
   3.0
);

INSERT INTO tariff_nums (tariff_id, price, priority,
daytime_from, daytime_to) VALUES
(  (SELECT id FROM tariffs WHERE fgrant = FALSE),
   1.8, 1, '02:00:00', '09:00:00'
);

INSERT INTO tariff_nums (tariff_id, price, priority,
daytime_from, daytime_to) VALUES
(  (SELECT id FROM tariffs WHERE fgrant = FALSE),
   3.5, 2, '15:00:00', '19:00:00'
);

INSERT INTO plans (name, descr)
VALUES ('unlimited', 'полный анлимит');

INSERT INTO plans (name, descr)
VALUES ('classic', 'классический');

INSERT INTO plan_members (plan_id, tariff_id, proto_mask, proto_test, priority) VALUES
(  (SELECT id FROM plans     WHERE name   = 'unlimited'),
   (SELECT id FROM tariffs   WHERE fgrant = TRUE), 
   0, 0, 1
);

INSERT INTO plan_members (plan_id, tariff_id, proto_mask, proto_test, priority) VALUES
(  (SELECT id FROM plans     WHERE name   = 'classic'),
   (SELECT id FROM tariffs   WHERE fgrant = FALSE),
   0, 0, 1
);

INSERT INTO pcard_actions (code, descr) VALUES (4,   'Аннулирование');
INSERT INTO pcard_actions (code, descr) VALUES (100, 'Генерация');
INSERT INTO pcard_actions (code, descr) VALUES (101, 'Выпуск');
INSERT INTO pcard_actions (code, descr) VALUES (102, 'Проверка');
INSERT INTO pcard_actions (code, descr) VALUES (103, 'Активация');
INSERT INTO pcard_actions (code, descr) VALUES (104, 'Аннулирование');
INSERT INTO pcard_actions (code, descr) VALUES (105, 'Снятие бана');
INSERT INTO pcard_actions (code, descr) VALUES (202, 'Проверка');
INSERT INTO pcard_actions (code, descr) VALUES (203, 'Активация');

COMMIT TRANSACTION;
