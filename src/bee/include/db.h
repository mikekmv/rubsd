/* $RuOBSD: db.h,v 1.16 2008/12/26 05:25:20 shadow Exp $ */
#ifndef __DB_H__
#define __DB_H__

#include <sys/cdefs.h>
#include <sys/types.h>

#include <bee.h>
#include <links.h>
#include <tariffs.h>
//#include <res.h>
#include <conf.h>

#define ATAG_DELETED 	1
#define ATAG_BROKEN	2
#define ATAG_FROZEN	4
#define ATAG_OFF	8
#define ATAG_UNLIMIT	16
#define ATAG_PAYMAN	32
#define ATAG_RES	64

#define ACC_UNLIMIT  33           /* account is Unlimited     (valid)     */
#define NEGATIVE     1            /* account has negative balance (valid) */
#define SUCCESS      0            /* record readed            (valid)     */
#define NOT_FOUND    (-1)         /* record not found         (not loaded)*/
#define ACC_DELETED  (-2)         /* record marked as deleted (loaded)    */
#define IO_ERROR     (-3)         /* base i/o error           (not loaded)*/
#define ACC_BROKEN   (-50)        /* broken mark or crc       (loaded)    */
#define ACC_FROZEN   (-51)        /* frozen mark              (valid)     */
#define ACC_OFF      (-52)        /* turned off               (valid)     */

#define ACC_NOCHARGE (-100)       /* account has no charge on it (on charge transaction) */

#define BALANCE_NA   (-1e300)     /* balance is not available value */

// proto field of is_data
#define PROTO_RPORT        0x0000ffff
#define PROTO_IPPROTO      0x00ff0000
#define PROTO_CHARGE       0x40000000
#define PROTO_DIR          0x80000000

#define PROTO_CHARGE_HACK  0x44000000   /* bug compatibility */

#define PROTO2_LPORT       0x0000ffff
#define PROTO2_TPLAN       0x00ff0000
#define PROTO2_SUBTPLAN    0xff000000

typedef struct
{   int         tag;		// account tag   
    int         accno;		// account number
    double      balance;	// account balance
    time_t      start;		// account start date/time
    time_t      stop;		// account stop (expire) date/time
    int         tariff;		// tariff plan number
    time_t      summ_rsttime;	// summary values reset time (first day of next month)

    long long   inet_summ_in;   // summary month inet inbound counter (signed)
    double      money_summ;     // summary month money counter
    long long   inet_summ_out;  // summary month inet outbound counter (signed)
    long long   res_balance;	// resource count balance

    double      tcredit;	// temp credit
    long long   res_tcredit;	// temp resource credit
    int         reserv2[4];	// (reserved)

    int         reserv3[6];	// (reserved)
    time_t      upd_time;       // last account change time
    int         crc;            // record CRC
} acc_t;

typedef struct
{   int      tag;         // account tag   
    int      accno;       // account number
    double   balance;     // account balance
    time_t   start;	  // account start date/time
    time_t   stop;	  // account stop (expire) date/time
    int      reserv[1];   // account flags
    int      crc;         // record CRC
} acc_t_old;

typedef struct
{  int            res_id;
   int            user_id;
   u_int          value;
   int            proto_id;
   struct in_addr host;
   int            proto2;
   int            reserv[2];
} is_data_t;

typedef struct
{   time_t     time;      // (1) transaction time
    int        accno;     // (1) account number (or (-1) if unknown)
    double     sum;       // (2) transaction sum (signed)
    is_data_t  isdata;    // (8) count module data
    double     balance;   // (2) account balance before transaction
    int        serrno;    // (1) errno (0 - success)
    int        crc;       // (1) record CRC
} logrec_t; 

typedef struct
{  int   fd;
   int   fasync;
} accbase_t;

typedef struct
{  int   fd;
   int   fasync;
} logbase_t;

__BEGIN_DECLS

// low level functions
int       db_open     (char * filespec);
int       dbs_open    (char * filespec);
int       db_close    (int fd);
int       db_get      (int fd, long long  rec, void * buf,  long long  recsize);
int       db_put      (int fd, long long  rec, void * data, long long  recsize);
long long db_add      (int fd, void * data, long long  recsize);
int       db_shlock   (int fd);
int       db_exlock   (int fd);
int       db_unlock   (int fd);
long long db_reccount (int fd, long long  recsize);
int       db_sync     (int fd);

int count_crc (void * data, long long len);

// Account base functions
int acc_baseopen  (accbase_t * base, char * file);
int acc_baseclose (accbase_t * base);
int acc_baselock  (accbase_t * base);
int acc_baseunlock(accbase_t * base);
int acc_reccount  (accbase_t * base);
int acc_get       (accbase_t * base, int rec, acc_t * acc);
int acc_put       (accbase_t * base, int rec, acc_t * acc);
int acc_add       (accbase_t * base, acc_t * acc);
int acc_trans     (accbase_t * base, int rec, double sum, 
                   is_data_t * isdata, logbase_t * logbase);
// internal
int acci_get     (accbase_t * base, int rec, acc_t * acc);
int acci_get_old (accbase_t * base, int rec, acc_t_old * acc);
int acci_put     (accbase_t * base, int rec, acc_t * acc);

// asyncronous mode (speed optimization)
int acc_async_on   (accbase_t * base);
int acc_async_off  (accbase_t * base);

// Transaction log base functions
int        log_baseopen  (logbase_t * base, char * file);
int        log_baseclose (logbase_t * base);
int        log_baselock  (logbase_t * base);
int        log_baseunlock(logbase_t * base);
long long  log_reccount  (logbase_t * base);
int        log_get       (logbase_t * base, long long rec, logrec_t * data);
int        log_put       (logbase_t * base, long long rec, logrec_t * data);
long long  log_add       (logbase_t * base, logrec_t * data);

int        log_baseopen_sr  (logbase_t * base, char * file);

// internal
int        logi_get     (logbase_t * base, long long rec, logrec_t * data);
int        logi_put     (logbase_t * base, long long rec, logrec_t * data);

// asyncronous mode (speed optimization)
int        log_async_on   (logbase_t * base);
int        log_async_off  (logbase_t * base);


__END_DECLS

#endif /* __DB_H__ */
