$RuOBSD$

--- src/backend/libpq/auth.c.orig	Thu May  2 02:44:44 2002
+++ src/backend/libpq/auth.c	Thu May  2 03:01:38 2002
@@ -229,7 +229,7 @@
 				 " Kerberos error %d\n", retval);
 		com_err("postgres", retval,
 				"while getting server principal for service %s",
-				pg_krb_server_keyfile);
+				PG_KRB_SRVNAM);
 		krb5_kt_close(pg_krb5_context, pg_krb5_keytab);
 		krb5_free_context(pg_krb5_context);
 		return STATUS_ERROR;
@@ -283,8 +283,15 @@
 	 *
 	 * I have no idea why this is considered necessary.
 	 */
+#if defined(HAVE_KRB5_TICKET_ENC_PART2)
 	retval = krb5_unparse_name(pg_krb5_context,
 							   ticket->enc_part2->client, &kusername);
+#elif defined(HAVE_KRB5_TICKET_CLIENT)
+	retval = krb5_unparse_name(pg_krb5_context,
+							   ticket->client, &kusername);
+#else
+#error "bogus configuration"
+#endif
 	if (retval)
 	{
 		snprintf(PQerrormsg, PQERRORMSG_LENGTH,
