$RuOBSD$
--- sound.c.orig	Fri Feb  1 07:49:13 2002
+++ sound.c	Fri Feb  1 07:49:17 2002
@@ -44,7 +44,7 @@
 
 #include <config.h>
 
-#if defined(HAVE_SYS_SOUNDCARD_H)
+#if defined(HAVE_SOUNDCARD_H)
 #include "osssound.h"
 #endif
 
@@ -177,7 +177,7 @@
 int f,ret;
 
 /* if we don't have any sound I/O code compiled in, don't do sound */
-#if !defined(HAVE_SYS_SOUNDCARD_H)	/* only type for now */
+#if !defined(HAVE_SOUNDCARD_H)	/* only type for now */
 return;
 #endif
 
@@ -196,7 +196,7 @@
 if(sound_stereo_ay || sound_stereo_beeper)
   sound_stereo=1;
 
-#if defined(HAVE_SYS_SOUNDCARD_H)
+#if defined(HAVE_SOUNDCARD_H)
 ret=osssound_init(&sound_freq,&sound_stereo);
 #endif
 
@@ -291,7 +291,7 @@
   {
   if(sound_buf)
     free(sound_buf);
-#if defined(HAVE_SYS_SOUNDCARD_H)
+#if defined(HAVE_SOUNDCARD_H)
   osssound_end();
 #endif
   sound_enabled=0;
@@ -756,7 +756,7 @@
 /* overlay AY sound */
 sound_ay_overlay();
 
-#if defined(HAVE_SYS_SOUNDCARD_H)
+#if defined(HAVE_SOUNDCARD_H)
 osssound_frame(sound_buf,sound_framesiz*sound_channels);
 #endif
 
