/* $RuOBSD: db.h,v 1.11 2007/09/11 04:32:32 shadow Exp $ */
#ifndef __DB_H__
#define __DB_H__

#include <sys/cdefs.h>

#define ATAG_DELETED 	1
#define ATAG_BROKEN	2
#define ATAG_FROZEN	4
#define ATAG_OFF	8
#define ATAG_UNLIMIT	16
#define ATAG_PAYMAN	32

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

struct _acc_t
{   int         tag;		// account tag   
    int         accno;		// account number
    money_t     balance;	// account balance
    time_t      start;		// account start date/time
    time_t      stop;		// account stop (expire) date/time
    int         tariff;		// tariff plan number
    time_t      summ_rsttime;	// summary values reset time (first day of next month)

    long long   inet_summ_in;   // summary month inet inbound counter (signed)
    money_t     money_summ;     // summary month money counter
    long long   inet_summ_out;  // summary month inet outbound counter (signed)
    int         reserv1[2];	// (reserved)

    int         reserv2[8];	// (reserved)

    int         reserv3[6];	// (reserved)
    time_t      upd_time;       // last account change time
    int         crc;            // record CRC
};

struct _acc_t_old
{   int      tag;         // account tag   
    int      accno;       // account number
    money_t  balance;     // account balance
    time_t   start;	  // account start date/time
    time_t   stop;	  // account stop (expire) date/time
    int      reserv[1];   // account flags
    int      crc;         // record CRC
};

struct _logrec_t
{   time_t     time;      // (1) transaction time
    int        accno;     // (1) account number (or (-1) if unknown)
    money_t    sum;       // (2) transaction sum (signed)
    is_data_t  isdata;    // (8) count module data
    money_t    balance;   // (2) account balance before transaction
    int        serrno;    // (1) errno (0 - success)
    int        crc;       // (1) record CRC
}; 

struct _accbase_t
{  int   fd;
   int   fasync;
};

struct _logbase_t
{  int   fd;
   int   fasync;
};

__BEGIN_DECLS

// low level functions
int db_open     (char * file);
int dbs_open     (char * file);
int db_close    (int fd);
int db_get      (int fd, int rec, void * buf, int len);
int db_put      (int fd, int rec, void * data, int len);
int db_add      (int fd, void * data, int len);
int db_shlock   (int fd);
int db_exlock   (int fd);
int db_unlock   (int fd);
int db_reccount (int fd, int len);
int db_sync     (int fd);

int count_crc (void * data, int len);

// Account base functions
int acc_baseopen  (accbase_t * base, char * file);
int acc_baseclose (accbase_t * base);
int acc_baselock  (accbase_t * base);
int acc_baseunlock(accbase_t * base);
int acc_reccount  (accbase_t * base);
int acc_get       (accbase_t * base, int rec, acc_t * acc);
int acc_put       (accbase_t * base, int rec, acc_t * acc);
int acc_add       (accbase_t * base, acc_t * acc);
int acc_trans     (accbase_t * base, int rec, money_t sum, 
                   is_data_t * isdata, logbase_t * logbase);
// internal
int acci_get     (accbase_t * base, int rec, acc_t * acc);
int acci_get_old (accbase_t * base, int rec, acc_t_old * acc);
int acci_put     (accbase_t * base, int rec, acc_t * acc);

// asyncronous mode (speed optimization)
int acc_async_on   (accbase_t * base);
int acc_async_off  (accbase_t * base);

// Transaction log base functions
int log_baseopen  (logbase_t * base, char * file);
int log_baseclose (logbase_t * base);
int log_baselock  (logbase_t * base);
int log_baseunlock(logbase_t * base);
int log_reccount  (logbase_t * base);
int log_get       (logbase_t * base, int rec, logrec_t * data);
int log_put       (logbase_t * base, int rec, logrec_t * data);
int log_add       (logbase_t * base, logrec_t * data);

int log_baseopen_sr  (logbase_t * base, char * file);

// internal
int logi_get     (logbase_t * base, int rec, logrec_t * data);
int logi_put     (logbase_t * base, int rec, logrec_t * data);

// asyncronous mode (speed optimization)
int log_async_on   (logbase_t * base);
int log_async_off  (logbase_t * base);


__END_DECLS

#endif /* __DB_H__ */
