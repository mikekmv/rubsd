// File Shell
// Inits file

#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>

#include "global.h"

/*
Do initialize by InIt:
(there 's a place for checks for critical errors)
1. Check curses functionality(&ability in future)
2.

Do Initialize by OpenShell:
(no place for critical errors)
*Create screen, save old IIP, update screen sizes
*Cmd structure reset


Do Initialize Frames
Make Frames chars

*/

int
InIt()
{
    return ProbeUnicon();
}

void
OutIt()
{
    return;
}

void
OpenShell()
{
    CreateUnicon();
    Color();
}

void
CloseShell()
{
    erase();refresh();  // Q&D version
    KillUnicon();
}

char           *
MakeFrames(char *fname, char *entry)
{
    int             fd = -1;	/* frames file descriptor */
    int             fcc = 0;	/* frames chars counter; */
    struct stat     st;
    char           *buf = NULL;
    char           *frameset = NULL;
    char           *sp;		/* start of frames entry */
    char           *ep;		/* end of frames entry */
    char           *tok;	/* current token */
    char           *hp;		/* 'x' position if hexadecimal */
#define EOFE	';'		/* end of frames entry sign */
#define FSZ	18		/* frames size */
#define TS	" \n\t"		/* tokens separator */

    if (entry && fname) {
	if (!stat(fname, &st)) {
	    if (st.st_size > 0) {
		if ((fd = open(fname, O_RDONLY)) != -1) {
		    buf = new char  [st.st_size + 1];

		    if (read(fd, buf, st.st_size) == st.st_size) {

			if ((sp = strstr(buf, entry)) && (ep = strchr(sp, EOFE))) {
			    *ep = (char) 0;
			    tok = strtok(sp, TS);
			    frameset = new char[FSZ + 1];
			    while ((tok = strtok(NULL, TS))) {
				if (fcc > FSZ) {
				    frameset[fcc] = (char) 0;
				    break;
				}
				if ((hp = strchr(tok, 'x'))) {
				    if (hp != tok &&
					(frameset[fcc] = strtoul(--hp,
						      (char **) NULL, 16)));

				    else {
					fprintf(stderr, "broken entry %s in %s\n",
						entry, fname);
					delete          frameset;
					frameset = NULL;
					break;
				    }
				} else
				    frameset[fcc] = *tok;
				++fcc;
			    }

			} else {
			    fprintf(stderr, "entry %s not found in %s\n",
				    entry, fname);
			}
		    } else {
			perror("read()");
		    }
		} else {
		    perror("open()");
		}
	    } else {
		fprintf(stderr, "file %s has 0 size\n", fname);
	    }
	} else {
	    perror("stat()");
	}
    }
    if (buf)
	delete          buf;
    if (fd != -1)
	(void) close(fd);
    return frameset;
}
