#ifndef __DB_INTERNAL_H__
#define __DB_INTERNAL_H__

#define SQLDATA(p) ((struct sqldata *)(p))
#define INTDATA(p) ((dbintdata_t *)((SQLDATA(p))->data))

typedef struct
{  char             * server;
   char             * dbname;
   int                rows;
   int                cols;
   struct sql_tuple * tuple;
} dbintdata_t;


#endif /* __DB_INTERNAL_H__ */

