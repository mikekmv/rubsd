--- config/programs.m4.orig	Thu May  2 17:23:49 2002
+++ config/programs.m4	Thu May  2 17:31:04 2002
@@ -80,6 +80,14 @@
 [AC_REQUIRE([AC_CANONICAL_HOST])
 AC_MSG_CHECKING([for readline])
 
+case $host_os in netbsd* | openbsd* )
+  if echo __ELF__ | $CPP - 2>/dev/null | grep __ELF__ >/dev/null; then
+    objformat=a.out
+  else
+    objformat=ELF
+  fi
+esac
+
 AC_CACHE_VAL([pgac_cv_check_readline],
 [pgac_cv_check_readline=no
 for pgac_lib in "" " -ltermcap" " -lncurses" " -lcurses" ; do
@@ -87,12 +95,14 @@
     pgac_save_LIBS=$LIBS
     LIBS="${pgac_rllib}${pgac_lib} $LIBS"
     AC_TRY_LINK_FUNC([readline], [[
-      # NetBSD and OpenBSD have a broken linker that does not
+      # NetBSD and OpenBSD have a broken a.out linker that does not
       # recognize dependent libraries
       case $host_os in netbsd* | openbsd* )
-        case $pgac_lib in
-          *curses*) ;;
-          *) pgac_lib=" -lcurses" ;;
+        case $objformat in a.out )
+          case $pgac_lib in
+            *curses*) ;;
+            *) pgac_lib=" -lcurses" ;;
+          esac
         esac
       esac
 
