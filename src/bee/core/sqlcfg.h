/* $Id$ */
#ifndef __CONFIG_H__
#define __CONFIG_H__

typedef struct
{  char    * DBtype;
   char    * DBhost;
   char    * DBname;
   char    * DBlogin;
   char    * DBpass;
   char    * DBscripts;
} config_t;

int cfg_load(char * file, config_t * pconf);

#endif /*__CONFIG_H__*/
