/* $RuOBSD$ */
/* $Id$ */

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "autoconf.h"   /* For GNU autoconf variables */

#include <sys/types.h>
#include <ctype.h>

#if STDC_HEADERS
# include <stdlib.h>
#endif

#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#if HAVE_FCNTL_H
# include <fcntl.h>
#endif

#if HAVE_STRINGS_H
# include <strings.h>
#endif

#if HAVE_SYS_FILE_H
# include <sys/file.h>
#endif

#if HAVE_SYS_IOCTL_H
# include <sys/ioctl.h>
#endif

#if HAVE_SYSLOG_H
# include <syslog.h>
#endif

#if HAVE_SIGNAL_H
# include <signal.h>
#endif

#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#ifndef HAVE_ISBLANK
# define	isblank(c)	( c == 0x20 || c == '\t' )
#endif

#include <sys/stat.h>
#include <sys/param.h>

#include <sys/socket.h>
#include <poll.h>

#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>
#include <resolv.h>

#include <netinet/tcp.h>
#include <netinet/ip_icmp.h>

#ifndef linux
# include <netinet/ip_var.h>
#endif

#ifndef __P
#if defined(STDC_HEADERS) || defined(__cplusplus) || defined(c_plusplus)
#define __P(args) args
#else
#define __P(args) ()
#endif
#endif /*!__P*/

#ifndef BYTE_ORDER
#ifdef WORDS_BIGENDIAN
# define BYTE_ORDER 4321
#else
# define BYTE_ORDER 1234
#endif
#endif

#if HAVE_MD5
# include <md5.h>
#else
# include "md5.h"
#endif

#endif /* _CONFIG_H_ */

