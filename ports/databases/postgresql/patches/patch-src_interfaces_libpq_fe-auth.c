$RuOBSD$

--- src/interfaces/libpq/fe-auth.c.orig	Mon Nov  5 20:46:37 2001
+++ src/interfaces/libpq/fe-auth.c	Thu May  2 05:16:58 2002
@@ -403,9 +403,18 @@
 	{
 		if (retval == KRB5_SENDAUTH_REJECTED && err_ret)
 		{
+#if defined(HAVE_KRB5_ERROR_TEXT_DATA)
 			snprintf(PQerrormsg, PQERRORMSG_LENGTH,
 			  libpq_gettext("Kerberos 5 authentication rejected: %*s\n"),
 					 err_ret->text.length, err_ret->text.data);
+#elif defined(HAVE_KRB5_ERROR_E_DATA)
+			snprintf(PQerrormsg, PQERRORMSG_LENGTH,
+			  libpq_gettext("Kerberos 5 authentication rejected: %*s\n"),
+					 err_ret->e_data->length,
+					 (const char *)err_ret->e_data->data);
+#else
+#error "bogus configuration"
+#endif
 		}
 		else
 		{
@@ -683,7 +692,7 @@
 char *
 fe_getauthname(char *PQerrormsg)
 {
-	char	   *name = (char *) NULL;
+	const char *name = (char *) NULL;
 	char	   *authn = (char *) NULL;
 	MsgType		authsvc;
 
