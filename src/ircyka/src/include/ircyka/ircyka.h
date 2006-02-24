/*
 * $RuOBSD$
 *
 * Copyright (c) 2005-2006 Oleg Safiullin <form@pdp-11.org.ru>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice unmodified, this list of conditions, and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#ifndef __IRCYKA_H__
#define __IRCYKA_H__

#include <sys/types.h>
#include <sys/tree.h>
#include <sys/queue.h>


#define IRCYKA_VERSION_MAJOR		6
#define IRCYKA_VERSION_MINOR		6
#define IRCYKA_VERSION_PATCH		6

#define IRCYKA_CONF_USER		"ircyka"
#define IRCYKA_CONF_DIR			".ircyka"
#define IRCYKA_CONF_FILE		"ircyka.conf"
#define IRCYKA_CONF_NAME		"default"
#define IRCYKA_NICKDB_FILE		"ircyka.ndb"
#define IRCYKA_ACLDB_FILE		"ircyka.adb"

#define IRCYKA_DEF_HOST			"127.0.0.1"
#define IRCYKA_DEF_PORT			"6667"
#define IRCYKA_DEF_CHARSET		"ascii"

#define IRCYKA_MAX_STRLEN		512
#define IRCYKA_MAX_BUFLEN		8192
#define IRCYKA_MAX_NICKLEN		16
#define IRCYKA_MAX_USERLEN		16
#define IRCYKA_MAX_ARGS			16
#define IRCYKA_MAX_CHANNELS		32
#define IRCYKA_MAX_MODNAME		16

#define IRCYKA_QUIT_TIMEOUT		10
#define IRCYKA_ALARM_INTERVAL		5

#ifndef IRCYKA_DATA_DIR
#define IRCYKA_DATA_DIR			"/usr/local/lib/ircyka"
#endif

#define IRCYKA_MODULES_DIR		"modules"
#define IRCYKA_MODULE_DATA		"ircyka_module_data"

#define IRCYKA_MODULE(name, descr)	struct ircyka_module		\
					    ircyka_module_data = {	\
						IRCYKA_VERSION_MAJOR,	\
						0,			\
						name,			\
						descr,			\
						NULL,			\
						ircyka_module_entry,	\
						{ NULL, NULL, }		\
					}

#define irc_register(name, cb)		handler_register( \
					    &ircyka_irc_handlers, name, cb)
#define irc_unregister(cb)		handler_unregister( \
					    &ircyka_irc_handlers, cb);
#define cmd_register(name, cb)		handler_register( \
					    &ircyka_cmd_handlers, name, cb)
#define cmd_unregister(cb)		handler_unregister( \
					    &ircyka_cmd_handlers, cb);


struct ircyka_conf_var {
	int				icv_type;
#define ICVT_EOV			0
#define ICVT_NUM			1
#define ICVT_BOOL			2
#define ICVT_STR			3
	char				*icv_name;
	void				*icv_data;
	void				*icv_dflt;
#define ICV_EOV				{ ICVT_EOV, NULL, NULL, NULL }
#define ICV_NUM(n, v, d)		{ ICVT_NUM, n, &v, (void *)d }
#define ICV_BOOL(n, v, d)		{ ICVT_BOOL, n, &v, (void *)d }
#define ICV_STR(n, v, d)		{ ICVT_STR, n, &v, (void *)d }
#define ICV_LINK(l)			{ ICVT_LINK, NULL, l, NULL }
};

struct ircyka_qio {
	void				*iq_data;
	size_t				iq_len;
	SIMPLEQ_ENTRY(ircyka_qio)	iq_entry;
};

SIMPLEQ_HEAD(ircyka_qio_list, ircyka_qio);

struct ircyka_nick {
	char				in_nick[IRCYKA_MAX_NICKLEN];
	char				in_user[IRCYKA_MAX_USERLEN];
	char				*in_host;
	time_t				in_time;
	u_int32_t			in_flags;
#define INF_LOGON			0x00000001U
#define INF_PRIV			0x00000002U
#define INF_CHANOUT			0x00000004U
#define INF_AUTO			0x00000008U
#define INF_MAIL			0x00010000U
	LIST_HEAD(, ircyka_join)	in_joins;
	RB_ENTRY(ircyka_nick)		in_entry;
};

RB_HEAD(ircyka_nick_tree, ircyka_nick);

struct ircyka_channel {
	char				*ic_channel;
	time_t				ic_time;
	u_int32_t			ic_flags;
#define ICF_T				0x00000001U
#define ICF_N				0x00000002U
#define ICF_S				0x00000004U
#define ICF_I				0x00000008U
#define ICF_P				0x00000010U
#define ICF_M				0x00000020U
#define ICF_PERSIST			0x00008000U
	u_int32_t			ic_count;
	LIST_HEAD(, ircyka_join)	ic_joins;
	RB_ENTRY(ircyka_channel)	ic_entry;
};

RB_HEAD(ircyka_channel_tree, ircyka_channel);

struct ircyka_join {
	struct ircyka_channel		*ij_ic;
	struct ircyka_nick		*ij_in;
	time_t				ij_time;
	u_int32_t			ij_flags;
#define IJF_OPER			0x00000001U
#define IJF_HALFOP			0x00000002U
#define IJF_VOICE			0x00000004U
	LIST_ENTRY(ircyka_join)		ij_centry;
	LIST_ENTRY(ircyka_join)		ij_nentry;
};

struct ircyka_callback {
	struct ircyka_handler		*icb_ih;
	int				(*icb_cb)(int, char * const *);
	LIST_ENTRY(ircyka_callback)	icb_entry;
};

struct ircyka_handler {
	const char			*ih_name;
	LIST_HEAD(, ircyka_callback)	ih_callbacks;
	RB_ENTRY(ircyka_handler)	ih_entry;
};

RB_HEAD(ircyka_handler_tree, ircyka_handler);

struct ircyka_module {
	u_int32_t			im_rev;
	u_int32_t			im_flags;
	char				*im_name;
	char				*im_descr;
	void				*im_handle;
	int				(*im_exec)(int, const char *,
					    const char *);
#define IME_LOAD			1
#define IME_UNLOAD			2
#define IME_EXEC			3
	LIST_ENTRY(ircyka_module)	im_entry;
};

LIST_HEAD(ircyka_module_list, ircyka_module);

struct ircyka_alarm {
	void				(*ia_exec)(void *);
	void				*ia_arg;
	LIST_ENTRY(ircyka_alarm)	ia_entry;
};

LIST_HEAD(ircyka_alarm_list, ircyka_alarm);


extern struct ircyka_qio_list ircyka_qio;
extern struct ircyka_nick_tree ircyka_nicks;
extern struct ircyka_channel_tree ircyka_channels;
extern struct ircyka_handler_tree ircyka_irc_handlers;
extern struct ircyka_handler_tree ircyka_cmd_handlers;
extern struct ircyka_module_list ircyka_modules;
extern struct ircyka_alarm_list ircyka_alarms;

extern char *ircyka_conf_file;
extern char *ircyka_conf_name;
extern char *ircyka_conf_dir;
extern char *ircyka_ident;
extern char *ircyka_log_target;
extern void *ircyka_nickdb;
extern void *ircyka_acldb;
extern int ircyka_debug;
extern int ircyka_die;
extern int ircyka_phase;
extern int ircyka_delay;
extern int ircyka_got_alarm;

extern char *irc_host;
extern char *irc_port;
extern char *irc_user;
extern char *irc_name;
extern char *irc_nick;
extern char *irc_pass;
extern char *irc_domain;
extern char *irc_hostname;
extern char *irc_servname;
extern char *irc_channels;
extern char *irc_charset;
extern char *irc_partmsg;
extern char *irc_quitmsg;
extern char *irc_killmsg;
extern char *irc_bitchmsg;
extern char *irc_reconnect;
extern char *log_target;
extern int irc_joinall;
extern int irc_bitch;


__BEGIN_DECLS
int			conf_load(const char *, const char *,
			    struct ircyka_conf_var *);
void			conf_unload(struct ircyka_conf_var *);
struct ircyka_conf_var	*conf_find(struct ircyka_conf_var *, const char *,
			    int);
void			conf_setdef(struct ircyka_conf_var *, const char *,
			    int, void *);
void			conf_reset(struct ircyka_conf_var *, const char *,
			    int);

void			qio_init(int);
int			qio_enqueue(const void *, size_t);
ssize_t			qio_dequeue(void);
void			qio_flush(void);
ssize_t			qio_poll(void *, size_t);
int			qio_finish(int);
int			qio_printf(const char *, ...);

const char		*ircyka_sockaddr(const void *);
int			ircyka_connect(const char **);
void			ircyka_loop(int);

int			match_irc_init(void);
void			match_irc(char *);
int			match_cmd_init(void);
void			match_cmd(const char *, const char *, const char *,
			    const char *);
char			*match_cmd_parse(int *, char **, const char *);

int			irc_printf(const char *, ...);
int			irc_privmsg(const char *, const char *, ...);
int			irc_notice(const char *, const char *, ...);
int			irc_mode(const char *, const char *, u_int32_t);
int			irc_invite(struct ircyka_channel *, const char *);
int			irc_join(struct ircyka_channel *, const char *);
int			irc_part(struct ircyka_channel *, const char *,
			    const char *);
int			irc_parse(int *, char **, char *);

int			charset_init(const char *);
int			charset_strcmp(const char *, const char *);
char			charset_tolower(char);
void			charset_strtolower(char *);
void			charset_translate(char *, size_t);
void			charset_untranslate(char *, size_t);

struct ircyka_nick	*nick_find(const char *);
struct ircyka_nick	*nick_create(const char *, const char *);
int			nick_change(struct ircyka_nick *, const char *);
void			nick_destroy(struct ircyka_nick *);
void			nick_login(struct ircyka_nick *, const char *, int);
void			nick_logout(struct ircyka_nick *);
int			nick_flags(const char *, u_int32_t *);
char			*nick_aflags(u_int32_t);

struct ircyka_join	*join_create(struct ircyka_channel *,
			    struct ircyka_nick *, u_int32_t);
void			join_destroy(struct ircyka_join *);

void			channel_init(void);
struct ircyka_channel	*channel_find(const char *);
struct ircyka_channel	*channel_create(const char *);
void			channel_destroy(struct ircyka_channel *);
u_int32_t		channel_flags(const char *, u_int32_t);
char			*channel_aflags(u_int32_t);
int			channel_valid(const char *);

struct ircyka_handler	*handler_find(struct ircyka_handler_tree *,
			    const char *);
struct ircyka_handler	*handler_create(struct ircyka_handler_tree *,
			    const char *);
void			handler_destroy(struct ircyka_handler_tree *,
			    struct ircyka_handler *);
struct ircyka_callback	*handler_register(struct ircyka_handler_tree *,
			    const char *, int (*)(int, char * const *));
void			handler_unregister(struct ircyka_handler_tree *,
			    struct ircyka_callback *);

struct ircyka_module	*module_find(const char *);
struct ircyka_module	*module_load(const char *, const char *, const char *);
int			module_unload(struct ircyka_module *im, const char *,
			    const char *);

void			ircyka_alarm(void);
struct ircyka_alarm	*alarm_add(void (*)(void *), void *);
void			alarm_del(struct ircyka_alarm *);

RB_PROTOTYPE(ircyka_channel_tree, ircyka_channel, ic_entry, channel_compare)
RB_PROTOTYPE(ircyka_nick_tree, ircyka_nick, in_entry, nick_compare)
RB_PROTOTYPE(ircyka_handler_tree, ircyka_handler, ih_entry, handler_compare)
__END_DECLS

#endif	/* __IRCYKA_H__ */
