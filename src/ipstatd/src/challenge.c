/*	$RuOBSD: challenge.c,v 1.10 2002/03/14 06:53:34 tm Exp $	*/

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include "extern.h"

char*
challenge(int size)
{
	long	rn;
	char	*p, *buf, *bp;
	int	n = size;

	buf = malloc(size);
	bp = buf;
	
	while(n > 0) {
		rn = random();
		p = (char*)&rn;
		while(p < ((char*)&rn + sizeof(rn))){
				*bp++ = *p++;
				if (--n == 0)
					break;
		}
	}
	bp = bin2ascii(buf, size);
	free(buf);
	return (bp);
}

char*
bin2ascii(unsigned char *buf, int size)
{
	static const char hex[]="0123456789abcdef";
	char	*p;
	int i;

	p = malloc(size * 2 + 1);
	for (i=0 ; i < size; i++) {
        	p[i + i] = hex[buf[i] >> 4];
        	p[i + i + 1] = hex[buf[i] & 0x0f];
	}
	p[i + i] = '\0';

	return (p);
}

char
ascii2hex(unsigned char c)
{
	if (c >= 0x30 && c < 0x40)
		return (c - 0x30);
	if (c > 0x60 && c <= 0x66)
		return (c - 0x57);
	return (-1);
}

char*
ascii2bin(char *buf, int size)
{
	char	*p;
	int	i, nsize;

	nsize = size / 2;
	p = malloc(nsize);
	for (i = 0; i < nsize; i++) {
		p[i] = (ascii2hex(buf[i + i]) << 4) | ascii2hex(buf[i + i + 1]);
	}

	return (p);
}
