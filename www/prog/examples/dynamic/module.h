/*	$RuOBSD$	*/

#ifndef __MODULE_H__
#define __MODULE_H__

#define MODULE_ENTRY_NAME	"module_entry"


typedef char			*module_t(void);


#ifdef MODULE
char	*module_entry(void);
#endif	/* MODULE */

#endif	/* __MODULE_H__ */
