/*	$RuOBSD$	*/

#ifndef FUSE_SUNSOUND_H
#define FUSE_SUNSOUND_H

int sunsound_init(int *, int *);
void sunsound_end(void);
void sunsound_frame(unsigned char *, int);

#endif /* FUSE_SUNSOUND_H */
