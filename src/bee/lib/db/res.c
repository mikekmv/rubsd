#include <bee.h>
#include <db.h>
#include <res.h>

int         resourcecnt = 8;

resource_t  resource[] =
{
   {0, NULL, NULL,   "inet",  "/usr/local/bin/beepfrules.sh", 1},
   {0, NULL, NULL,   "mail",  NULL, 0},
   {0, NULL, NULL,  "adder",  NULL, 0},
   {0, NULL, NULL,  "intra",  NULL, 0},
   {0, NULL, NULL,   "list",  NULL, 0},
   {0, NULL, NULL,  "login",  NULL, 0},
   {0, NULL, NULL,  "label",  NULL, 0},
   {0, NULL, NULL,"maxrate",  NULL, 0}
};

