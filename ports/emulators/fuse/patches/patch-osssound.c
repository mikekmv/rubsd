$RuOBSD$
--- osssound.c.orig	Sun Oct 21 19:44:45 2001
+++ osssound.c	Fri Feb  1 07:54:22 2002
@@ -19,7 +19,7 @@
 
 #include <config.h>
 
-#if defined(HAVE_SYS_SOUNDCARD_H)	/* OSS sound */
+#if defined(HAVE_SOUNDCARD_H)	/* OSS sound */
 
 #include <stdio.h>
 #include <string.h>
@@ -29,7 +29,7 @@
 #include <sys/types.h>
 #include <sys/ioctl.h>
 #include <fcntl.h>
-#include <sys/soundcard.h>
+#include <soundcard.h>
 
 #include "types.h"
 #include "sound.h"
@@ -144,4 +144,4 @@
   }
 }
 
-#endif	/* HAVE_SYS_SOUNDCARD_H */
+#endif	/* HAVE_SOUNDCARD_H */
