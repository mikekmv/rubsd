$RuOBSD$
--- spsound.c.orig	Thu Jan 31 03:37:29 2002
+++ spsound.c	Thu Jan 31 03:38:54 2002
@@ -329,8 +329,11 @@
 #ifdef SUN_SOUND
 
 #include <sys/audioio.h>
+#include <sys/ioctl.h>
 
+#ifndef __OpenBSD__
 #define HAVE_SOUND_FLUSH
+#endif
 #ifdef HAVE_SOUND_FLUSH
 #include <stropts.h>
 #include <sys/conf.h>
