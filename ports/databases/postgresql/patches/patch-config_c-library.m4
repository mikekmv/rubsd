$RuOBSD$

--- config/c-library.m4.orig	Thu May  2 02:44:44 2002
+++ config/c-library.m4	Thu May  2 02:56:36 2002
@@ -196,3 +196,34 @@
   AC_DEFINE([STRING_H_WITH_STRINGS_H], 1,
             [Define if string.h and strings.h may both be included])
 fi])
+
+
+# PGAC_CHECK_MEMBER(AGGREGATE.MEMBER,
+#                   [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND],
+#                   [INCLUDES])
+# -----------------------------------------------------------
+
+AC_DEFUN([PGAC_CHECK_MEMBER],
+[changequote(<<, >>)dnl
+dnl The name to #define.
+define(<<pgac_define_name>>, translit(HAVE_$1, [a-z .*], [A-Z__P]))dnl
+dnl The cache variable name.
+define(<<pgac_cache_name>>, translit(pgac_cv_member_$1, [ .*], [__p]))dnl
+changequote([, ])dnl
+AC_CACHE_CHECK([for $1], [pgac_cache_name],
+[AC_TRY_COMPILE([$4],
+[static ]patsubst([$1], [\..*])[ pgac_var;
+if (pgac_var.]patsubst([$1], [^[^.]*\.])[)
+return 0;],
+[pgac_cache_name=yes],
+[pgac_cache_name=no])])
+
+if test x"[$]pgac_cache_name" = x"yes"; then
+  AC_DEFINE_UNQUOTED(pgac_define_name)
+  $2
+else
+  ifelse([$3], [], :, [$3])
+fi
+undefine([pgac_define_name])[]dnl
+undefine([pgac_cache_name])[]dnl
+])
