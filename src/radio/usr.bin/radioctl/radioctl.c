/* $RuOBSD: radioctl.c,v 1.1 2001/10/03 05:53:35 gluk Exp $ */

/*
 * Copyright (c) 2001 Vladimir Popov <jumbo@narod.ru>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/ioctl.h>
#include <sys/radioio.h>

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define RADIO_ENV	"RADIODEVICE"
#define RADIODEVICE	"/dev/radio"

const char *varname[] = {
	"search",
#define OPTION_SEARCH		0x00
	"volume",
#define OPTION_VOLUME		0x01
	"frequency",
#define OPTION_FREQUENCY	0x02
	"mute",
#define OPTION_MUTE		0x03
	"reference",
#define OPTION_REFERENCE	0x04
	"mono",
#define OPTION_MONO		0x05
	"stereo",
#define	OPTION_STEREO		0x06
	"sensitivity"
#define	OPTION_SENSITIVITY	0x07
};

#define OPTION_NONE		~0u
#define VALUE_NONE		~0ul

extern char *__progname;
const char *onchar = "on";
#define ONCHAR_LEN	2
const char *offchar = "off";
#define OFFCHAR_LEN	3

static struct radio_info ri;
static int search;

static void	usage(void);
static void	print_vars(int);
static void	show_int_val(u_long, const char *, char *, int);
static void	show_float_val(float, const char *, char *, int);
static void	show_char_val(const char *, const char *, int);
static void	show_verbose(const char *, int);
static int	parse_option(const char *);
static void	print_value(int);
static int	write_param(char *, int);

static u_long   get_value(int);
static void     set_value(int, u_long);
static u_long   read_value(char *, int);
static void     warn_unsupported(int);

/*
 * Control behavior of a FM tuner - set frequency, volume etc
 */
int
main(int argc, char **argv)
{
	char *radiodev = NULL;
	char optchar;
	char *param = NULL;
	int rd = -1;
	int show_vars = 0;
	int set_param = 0;
	int silent = 0;
	int optv = 0;

	if (argc < 2) {
		usage();
		exit(1);
	}

	radiodev = getenv(RADIO_ENV);
	if (radiodev == NULL)
		radiodev = RADIODEVICE;

	while ((optchar = getopt(argc, argv, "af:nw:")) != -1) {
		switch (optchar) {
		case 'a':
			show_vars = 1;
			optv = 1;
			break;
		case 'f':
			radiodev = optarg;
			optv = 2;
			break;
		case 'n':
			silent = 1;
			optv = 1;
			break;
		case 'w':
			set_param = 1;
			param = optarg;
			optv = 2;
			break;
		default:
			usage();
			/* NOTREACHED */
		}

		argc -= optv;
		argv += optv;
	}

	rd = open(radiodev, O_RDONLY);
	if (rd < 0)
		err(1, "%s open error", radiodev);

	if (ioctl(rd, RIOCGINFO, &ri) < 0)
		err(1, "RIOCGINFO");

	if (argc > 1)
		if ((optv = parse_option(*(argv + 1))) != OPTION_NONE) {
			show_verbose(varname[optv], silent);
			print_value(parse_option(*(argv + 1)));
			putchar('\n');
		}

	if (set_param) {
		if ((optv = write_param(param, silent)) != OPTION_NONE) {
			if (optv == OPTION_SEARCH) {
				if (ioctl(rd, RIOCSSRCH, &search) < 0)
					err(1, "RIOCSSRCH");
			} else if (ioctl(rd, RIOCSINFO, &ri) < 0)
				err(1, "RIOCSINFO");
			if (ioctl(rd, RIOCGINFO, &ri) < 0)
				err(1, "RIOCGINFO");
			print_value(optv);
			putchar('\n');
		}
	}

	if (show_vars)
		print_vars(silent);

	if (close(rd) < 0)
		warn("%s close error", radiodev);

	return 0;
}

static void
usage(void)
{
	printf("Usage: %s [-f file] [-a] [-n] [-w name=value] [name]\n",
		__progname);
}

static void
show_verbose(const char * nick, int silent)
{
	if (!silent)
		printf("%s=", nick);
}

static void
show_int_val(u_long val, const char *nick, char *append, int silent)
{
	show_verbose(nick, silent);
	printf("%lu%s\n", val, append);
}

static void
show_float_val(float val, const char *nick, char *append, int silent)
{
	show_verbose(nick, silent);
	printf("%.2f%s\n", val, append);
}

static void
show_char_val(const char *val, const char *nick, int silent)
{
	show_verbose(nick, silent);
	printf("%s\n", val);
}

/*
 * Print all available parameters
 */
static void
print_vars(int silent)
{
	show_int_val(ri.volume, varname[OPTION_VOLUME], "", silent);

	show_float_val((float)ri.freq / 1000., varname[OPTION_FREQUENCY],
			"MHz", silent);

	show_char_val(ri.mute ? onchar : offchar, varname[OPTION_MUTE], silent);

	if (ri.caps & RADIO_CAPS_REFERENCE_FREQ)
		show_int_val(ri.rfreq, varname[OPTION_REFERENCE], "kHz", silent);
	if (ri.caps & RADIO_CAPS_LOCK_SENSITIVITY)
		show_int_val(ri.lock, varname[OPTION_SENSITIVITY], "mkV", silent);

	if (ri.caps & RADIO_CAPS_DETECT_SIGNAL) {
		show_verbose("signal", silent);
		printf("%s\n", ri.info & RADIO_INFO_SIGNAL ? onchar : offchar);
	}
	if (ri.caps & RADIO_CAPS_DETECT_STEREO) {
		show_verbose(varname[OPTION_STEREO], silent);
		printf("%s\n", ri.info & RADIO_INFO_STEREO ? onchar : offchar);
	}

	if (!silent)
		puts("card capabilities:");
	if (ri.caps & RADIO_CAPS_SET_MONO)
		puts("\tmanageable mono/stereo");
	if (ri.caps & RADIO_CAPS_HW_SEARCH)
		puts("\thardware search");
	if (ri.caps & RADIO_CAPS_HW_AFC)
		puts("\thardware AFC");
}

/*
 * Convert string to integer representation of a parameter
 */
static int
parse_option(const char *topt)
{
	int res, toptlen, varlen, len, varsize;

	if (topt == NULL || *topt == '\0')
		return OPTION_NONE;

	varsize = sizeof(varname) / sizeof(varname[0]);
	toptlen = strlen(topt);

	for (res = 0; res < varsize; res++) {
		varlen = strlen(varname[res]);
		len = toptlen > varlen ? toptlen : varlen;
		if (strncmp(topt, varname[res], len) == 0)
			return res;
	}

	warnx("bad name `%s'", topt);
	return OPTION_NONE;
}

/*
 * Print current value of the parameter.
 */
static void
print_value(int optval)
{
	if (optval == OPTION_NONE)
		return;

	switch (optval) {
	case OPTION_SEARCH:
		/* FALLTHROUGH */
	case OPTION_FREQUENCY:
		printf("%.2fMHz", (float)ri.freq / 1000.);
		break;
	case OPTION_REFERENCE:
		printf("%ukHz", ri.rfreq);
		break;
	case OPTION_SENSITIVITY:
		printf("%umkV", ri.lock);
		break;
	case OPTION_MUTE:
		printf(ri.mute ? onchar : offchar);
		break;
	case OPTION_MONO:
		printf(ri.stereo ? offchar : onchar);
		break;
	case OPTION_STEREO:
		printf(ri.stereo ? onchar : offchar);
		break;
	case OPTION_VOLUME:
	default:
		printf("%u", ri.volume);
		break;
	}
}

/*
 * Set new value of a parameter
 */
static int
write_param(char *param, int silent)
{
	int paramlen = 0;
	int namelen = 0;
	char *topt = NULL;
	const char *badvalue = "bad value `%s'";
	u_int optval = OPTION_NONE;
	u_long var = VALUE_NONE;
	u_long addvar = VALUE_NONE;
	u_char sign = 0;

	if (param == NULL || *param == '\0')
		return optval;

	paramlen = strlen(param);
	namelen = strcspn(param, "=");
	if (namelen > paramlen - 2) {
		warnx(badvalue, param);
		return optval;
	}

	paramlen -= ++namelen;

	if ((topt = (char *)malloc(namelen)) == NULL) {
		warn("memory allocation error");
		return optval;
	}
	strlcpy(topt, param, namelen);
	optval = parse_option(topt);

	if (optval == OPTION_NONE) {
		free(topt);
		return optval;
	}

	if (!silent)
		printf("%s: ", topt);

	free(topt);

	topt = &param[namelen];
	switch (*topt) {
	case '+':
	case '-':
		if ((addvar = read_value(topt + 1, optval)) == VALUE_NONE)
			break;
		if ((var = get_value(optval)) == VALUE_NONE)
			break;
		sign++;
		if (*topt == '+')
			var += addvar;
		else
			var -= addvar;
		break;
	case 'o':
		if (strncmp(topt, offchar,
			paramlen > OFFCHAR_LEN ? paramlen : OFFCHAR_LEN) == 0)
			var = 0;
		else
			if (strncmp(topt, onchar,
				paramlen > ONCHAR_LEN ? paramlen : ONCHAR_LEN) == 0)
				var = 1;
		break;
	case 'u':
		if (strncmp(topt, "up", paramlen > 2 ? paramlen : 2) == 0)
			var = 1;
		break;
	case 'd':
		if (strncmp(topt, "down", paramlen > 4 ? paramlen : 4) == 0)
			var = 0;
		break;
	default:
		if (*topt > 47 && *topt < 58)
			var = read_value(topt, optval);
		break;
	}

	if (var == VALUE_NONE || (sign && addvar == VALUE_NONE)) {
		warnx(badvalue, topt);
		return OPTION_NONE;
	}

	print_value(optval);
	printf(" -> ");

	set_value(optval, var);

	return optval;
}

/*
 * Returns current value of parameter optval
 */
static u_long
get_value(int optval)
{
	u_long var = VALUE_NONE;

	switch (optval) {
	case OPTION_VOLUME:
		var = ri.volume;
		break;
	case OPTION_SEARCH:
		if (ri.caps & RADIO_CAPS_HW_SEARCH)
			var = search;
		break;
	case OPTION_FREQUENCY:
		var = ri.freq;
		break;
	case OPTION_REFERENCE:
		if (ri.caps & RADIO_CAPS_REFERENCE_FREQ)
			var = ri.rfreq;
		break;
	case OPTION_MONO:
		if (ri.caps & RADIO_CAPS_SET_MONO)
			var = !ri.stereo;
		break;
	case OPTION_STEREO:
		if (ri.caps & RADIO_CAPS_SET_MONO)
			var = ri.stereo;
		break;
	case OPTION_SENSITIVITY:
		if (ri.caps & RADIO_CAPS_LOCK_SENSITIVITY)
			var = ri.lock;
		break;
	case OPTION_MUTE:
		var = ri.mute;
		break;
	}

	if (var == VALUE_NONE)
		warn_unsupported(optval);

	return var;
}

/*
 * Convert string to float or unsigned integer
 */
static u_long
read_value(char *str, int optval)
{
	u_long val;

	if (str == NULL || *str == '\0')
		return VALUE_NONE;

	if (optval == OPTION_FREQUENCY)
		val = (u_long)1000 * atof(str);
	else
		val = (u_long)strtol(str, (char **)NULL, 10);

	return val;
}

static void
warn_unsupported(int optval)
{
	warnx("driver does not support `%s'", varname[optval]);
}

/*
 * Set card parameter optval to value var
 */
static void
set_value(int optval, u_long var)
{
	int unsupported = 0;

	if (var == VALUE_NONE)
		return;

	switch (optval) {
	case OPTION_VOLUME:
		ri.volume = (u_int8_t)var;
		break;
	case OPTION_FREQUENCY:
		ri.freq = var;
		break;
	case OPTION_REFERENCE:
		if (ri.caps & RADIO_CAPS_REFERENCE_FREQ) {
			ri.rfreq = var;
		} else unsupported++;
		break;
	case OPTION_MONO:
		var = !var;
		/* FALLTHROUGH */
	case OPTION_STEREO:
		if (ri.caps & RADIO_CAPS_SET_MONO) {
			ri.stereo = var;
		} else unsupported++;
		break;
	case OPTION_SENSITIVITY:
		if (ri.caps & RADIO_CAPS_LOCK_SENSITIVITY) {
			ri.lock = var;
		} else unsupported++;
		break;
	case OPTION_MUTE:
		ri.mute = var;
		break;
	case OPTION_SEARCH:
		if (ri.caps & RADIO_CAPS_HW_SEARCH)
			search = var;
		break;
	}

	if ( unsupported )
		warn_unsupported(optval);
}
