/*
 *		$Id$
 */


#include <stdlib.h>


/*
 * challenge() fill buffer pointed by *challenge and size n by random data.
 * challenge() always return 0 on success. If returned value is 1 then
 * incoming parameter n is invalid.
 */

int challenge(challenge,n)
char *challenge;
int n;
{
	long rn;

	char *p;
	
	while(n > 0) {
		rn = random();
		p = (char*)&rn;
		while(p < ((char*)&rn + sizeof(rn))){
				*challenge++ = *p++;
				if ( --n == 0 )
					return 0;
		}
	}
	return(-1);
}

char* bin2ascii(buf,size)
unsigned char *buf;
int size;
{
	static const char hex[]="0123456789abcdef";
	char	*p;
	int i;

	p = malloc(size*2 + 1);
	for ( i=0 ; i<size ; i++ ) {
        	p[i+i] = hex[buf[i] >> 4];
        	p[i+i+1] = hex[buf[i] & 0x0f];
	}
	p[i+i] = '\0';

	return p;
}

char ascii2hex(c)
unsigned char c;
{
	if( c >= 0x30 && c < 0x40 )
		return (c - 0x30);
	if( c > 0x60 && c <= 0x66 )
		return (c - 0x57);
	return (-1);
}

char* ascii2bin(buf,size)
char *buf;
int size;
{
	char	*p;
	int	i,nsize;

	nsize = size/2;
	p = malloc(nsize);
	for ( i = 0; i<nsize; i++) {
		p[i] = (ascii2hex(buf[i+i]) << 4) | ascii2hex(buf[i+i+1]);
	}
	return p;
}
