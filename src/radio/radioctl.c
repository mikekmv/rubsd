/* $RuOBSD: radioctl.c,v 1.2 2001/09/29 03:47:04 pva Exp $ */

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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <err.h>
#include <sys/ioctl.h>
#include "/sys/sys/radioio.h"

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
#define OPTION_REFERENCE		0x04
	"mono",
#define OPTION_MONO		0x05
	"stereo",
#define	OPTION_STEREO		0x06
	"sensitivity"
#define	OPTION_SENSITIVITY		0x07
};

#define OPTION_NONE		~0ul
#define VALUE_NONE		~0ul

extern char *__progname;
const char *onchar = "on";
#define ONCHAR_LEN	2
const char *offchar = "off";
#define OFFCHAR_LEN	3

u_long caps = 0;

static void     usage(void);
static void     print_vars(int, int);
static void     write_param(int, char *, int);
static u_int    parse_option(const char *);
static u_long   get_value(int, int);
static void     set_value(int, int, u_long);
static u_long   read_value(char *, int);
static void     print_value(int, const char *, int);

int
main(argc, argv)
	int argc;
	char **argv;
{
	char *radiodev = NULL;
	char optchar;
	char *param = NULL;
	int rd = -1;
	int silent = 0;
	int show_vars = 0;
	int set_param = 0;

	if (argc < 2) {
		usage();
		exit(1);
	}

	radiodev = getenv(RADIO_ENV);
	if (radiodev == NULL)
		radiodev = RADIODEVICE;

	while ((optchar = getopt(argc, argv, "aw:nf:")) != -1) {
		switch (optchar) {
		case 'f':
			radiodev = optarg;
			break;
		case 'a':
			show_vars = 1;
			break;
		case 'n':
			silent = 1;
			break;
		case 'w':
			set_param = 1;
			param = optarg;
			break;
		default:
			usage();
			exit(1);
		}

		argc -= optind;
		argv += optind;
	}

	rd = open(radiodev, O_RDONLY);
	if (rd < 0)
		err(1, "%s open error", radiodev);

	if (ioctl(rd, RIOCGCAPS, &caps) < 0)
		err(1, "RIOCGCAPS");

	if (argc > 1)
		print_value(rd, argv[optind], silent);

	if (set_param)
		write_param(rd, param, silent);

	if (show_vars)
		print_vars(rd, silent);

	if (close(rd) < 0)
		warn("%s close error", radiodev);

	return 0;
}

static void
usage(void) {
	char *usagestr = "Usage: %s [-f file] [-a] [-n] [-w name=value] [name]\n";
	printf(usagestr, __progname);
}

static void
print_vars(fd, silent)
	int fd, silent;
{
	u_long var;

	print_value(fd, varname[OPTION_VOLUME], silent);
	print_value(fd, varname[OPTION_FREQUENCY], silent);
	print_value(fd, varname[OPTION_MUTE], silent);

	if (caps & RADIO_CAPS_REFERENCE_FREQ)
		print_value(fd, varname[OPTION_REFERENCE], silent);
	if (caps & RADIO_CAPS_LOCK_SENSITIVITY)
		print_value(fd, varname[OPTION_SENSITIVITY], silent);

	if (ioctl(fd, RIOCGINFO, &var) < 0)
		warn("RIOCGINFO");
	if (caps & RADIO_CAPS_DETECT_SIGNAL)
		if (!silent)
			printf("%s=", "signal");
		printf("%s\n", var & RADIO_INFO_SIGNAL ? onchar : offchar);
	if (caps & RADIO_CAPS_DETECT_STEREO)
		if (!silent)
			printf("%s=", varname[OPTION_STEREO]);
		printf("%s\n", var & RADIO_INFO_STEREO ? onchar : offchar);

	puts("card capabilities:");
	if (caps & RADIO_CAPS_SET_MONO)
		puts("\tmanageable mono/stereo");
	if (caps & RADIO_CAPS_HW_SEARCH)
		puts("\thardware search");
	if (caps & RADIO_CAPS_HW_AFC)
		puts("\thardware AFC");
}

static void
write_param(fd, param, silent)
	int fd;
	char *param;
	int silent;
{
	int paramlen = 0;
	int namelen = 0;
	char *topt = NULL;
	const char *badvalue = "bad value `%s'";
	int optval = OPTION_NONE;
	u_long var = VALUE_NONE;
	u_long addvar = VALUE_NONE;
	u_char sign = 0;

	if (param == NULL || *param == '\0')
		return;

	paramlen = strlen(param);
	namelen = strcspn(param, "=");
	if (namelen > paramlen - 2) {
		warnx(badvalue, param);
		return;
	}

	if ((topt = (char *)malloc(namelen + 1)) == NULL) {
		warn("memory allocation error");
		return;
	}
	strlcpy(topt, param, namelen + 1);
	optval = parse_option(topt);

	if (optval == OPTION_NONE) {
		warnx("bad name `%s'", topt);
		free(topt);
		return;
	}

	free(topt);

	topt = &param[namelen + 1];
	switch (*topt) {
	case '+':
	case '-':
		if ((addvar = read_value(topt + 1, optval)) == VALUE_NONE)
			break;
		if ((var = get_value(fd, optval)) == VALUE_NONE)
			break;
		sign++;
		if (*topt == '+')
			var += addvar;
		else
			var -= addvar;
		break;
	case 'o':
		addvar = paramlen - namelen - 1;
		if (strncmp(topt, offchar, addvar > OFFCHAR_LEN ? addvar : OFFCHAR_LEN) == 0)
			var = 0;
		else
			if (strncmp(topt, onchar, addvar > ONCHAR_LEN ? addvar : ONCHAR_LEN) == 0)
				var = 1;
		break;
	case 'u':
		addvar = paramlen - namelen - 1;
		if (strncmp(topt, "up", addvar > 2 ? addvar : 2) == 0)
			var = 1;
		break;
	case 'd':
		addvar = paramlen - namelen - 1;
		if (strncmp(topt, "down", addvar > 4 ? addvar : 4) == 0)
			var = 0;
		break;
	default:
		if (*topt > 47 && *topt < 58)
			var = read_value(topt, optval);
		break;
	}

	if (var == VALUE_NONE || (sign && addvar == VALUE_NONE)) {
		warnx(badvalue, topt);
		return;
	}

	set_value(fd, optval, var);
}

static u_int
parse_option(topt)
	const char *topt;
{
	u_int res = OPTION_NONE;
	int toptlen;

	if (topt == NULL)
		return OPTION_NONE;

	toptlen = strlen(topt);

	switch (*topt) {
	case 'v':
		if (strncmp(topt, varname[OPTION_VOLUME], toptlen) == 0)
			res = OPTION_VOLUME;
		break;
	case 'f':
		if (strncmp(topt, varname[OPTION_FREQUENCY], toptlen) == 0)
			res = OPTION_FREQUENCY;
		break;
	case 's':
		if (strncmp(topt, varname[OPTION_SEARCH], toptlen) == 0)
			res = OPTION_SEARCH;
		else if (strncmp(topt, varname[OPTION_STEREO], toptlen) == 0)
			res = OPTION_STEREO;
		else if (strncmp(topt, varname[OPTION_SENSITIVITY], toptlen) == 0)
			res = OPTION_SENSITIVITY;
		break;
	case 'm':
		if (strncmp(topt, varname[OPTION_MONO], toptlen) == 0)
			res = OPTION_MONO;
		else if (strncmp(topt, varname[OPTION_MUTE], toptlen) == 0)
			res = OPTION_MUTE;
		break;
	case 'r':
		if (strncmp(topt, varname[OPTION_REFERENCE], toptlen) == 0)
			res = OPTION_REFERENCE;
		break;
	default:
		res = OPTION_NONE;
		break;
	}

	return res;
}

static u_long
get_value(fd, optval)
	int fd;
	int optval;
{
	u_long var = VALUE_NONE;

	switch (optval) {
	case OPTION_VOLUME:
		if (ioctl(fd, RIOCGVOLU, &var) < 0)
			warn("RIOCGVOLU");
		break;
	case OPTION_FREQUENCY:
		if (ioctl(fd, RIOCGFREQ, &var) < 0)
			warn("RIOCGFREQ");
		break;
	case OPTION_REFERENCE:
		if (caps & RADIO_CAPS_REFERENCE_FREQ)
			if (ioctl(fd, RIOCGREFF, &var) < 0)
				warn("RIOCGREFF");
		break;
	case OPTION_MONO:
	case OPTION_STEREO:
		if (caps & RADIO_CAPS_SET_MONO)
			if (ioctl(fd, RIOCGMONO, &var) < 0)
				warn("RIOCGMONO");
		break;
	case OPTION_SENSITIVITY:
		if (caps & RADIO_CAPS_LOCK_SENSITIVITY)
			if (ioctl(fd, RIOCGLOCK, &var) < 0)
				warn("RIOCGLOCK");
		break;
	case OPTION_MUTE:
		if (ioctl(fd, RIOCGMUTE, &var) < 0)
			warn("RIOCGMUTE");
		break;
	}

	return var;
}

static void
set_value(fd, optval, var)
	int fd;
	int optval;
	u_long var;
{

	if (var == VALUE_NONE)
		return;

	switch (optval) {
	case OPTION_VOLUME:
		if (ioctl(fd, RIOCSVOLU, &var) < 0)
			warn("RIOCSVOLU");
		break;
	case OPTION_FREQUENCY:
		if (ioctl(fd, RIOCSFREQ, &var) < 0)
			warn("RIOCSFREQ");
		break;
	case OPTION_REFERENCE:
		if (caps & RADIO_CAPS_REFERENCE_FREQ)
			if (ioctl(fd, RIOCSREFF, &var) < 0)
				warn("RIOCSREFF");
		break;
	case OPTION_STEREO:
		var = !var;
	case OPTION_MONO:
		if (caps & RADIO_CAPS_SET_MONO)
			if (ioctl(fd, RIOCSMONO, &var) < 0)
				warn("RIOCSMONO");
		break;
	case OPTION_SENSITIVITY:
		if (caps & RADIO_CAPS_LOCK_SENSITIVITY)
			if (ioctl(fd, RIOCSLOCK, &var) < 0)
				warn("RIOCSLOCK");
		break;
	case OPTION_SEARCH:
		if (caps & RADIO_CAPS_HW_SEARCH)
			if (ioctl(fd, RIOCSSRCH, &var) < 0)
				warn("RIOCSSRCH");
		break;
	case OPTION_MUTE:
		if (ioctl(fd, RIOCSMUTE, &var) < 0)
			warn("RIOCSMUTE");
		break;
	}
}

static u_long
read_value(str, optval)
	char *str;
{
	if (str == NULL || *str == '\0')
		return VALUE_NONE;

	if (optval == OPTION_FREQUENCY)
		return (u_long)1000*atof(str);
	else
		return (u_long)strtol(str, (char **)NULL, 10);

	return VALUE_NONE;
}

static void
print_value(fd, str, silent)
	int fd;
	const char *str;
	int silent;
{
	int optval;
	u_long var, mhz;

	if (str == NULL || *str == '\0')
		return;

	optval = parse_option(str);
	if (optval == OPTION_NONE)
		return;

	var = get_value(fd, optval);
	if (var == VALUE_NONE)
		return;

	if (!silent)
		printf("%s=", str);

	switch (optval) {
	case OPTION_FREQUENCY:
		mhz = var/1000;
		printf("%u.%uMHz\n", (u_int)mhz, (u_int)var/10 - (u_int)mhz*100);
		break;
	case OPTION_REFERENCE:
		printf("%ukHz\n", (u_int)var);
		break;
	case OPTION_SENSITIVITY:
		printf("%umkV\n", (u_int)var);
		break;
	case OPTION_MUTE:
	case OPTION_MONO:
		printf("%s\n", var ? onchar : offchar);
		break;
	case OPTION_STEREO:
		printf("%s\n", var ? offchar : onchar);
		break;
	default:
		printf("%u\n", (u_int)var);
		break;
	}
}
