/*	$RuOBSD$	*/

#include "module.h"


char *
module_entry(void)
{
	static char *id = MODULE_NAME " ($RuOBSD$)";

	return (id);
}
