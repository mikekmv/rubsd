$RuOBSD$
--- main.c.orig	Fri Feb  1 05:13:41 2002
+++ main.c	Fri Feb  1 05:14:09 2002
@@ -20,7 +20,9 @@
 #include <string.h>
 #include <stdlib.h>
 #include <unistd.h>
+#ifndef __OpenBSD__
 #include <getopt.h>
+#endif
 #include "main.h"
 #include "sound.h"
 #include "ui.h"
