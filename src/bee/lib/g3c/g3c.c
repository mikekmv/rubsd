/* $oganer: g3c.c,v 1.1 2005/07/17 16:28:32 shadow Exp $ */
/* %oganer: g3c.c,v 1.22 2005/07/09 17:26:42 shadow Exp % (from ICMS)*/

/*
 * Copyright (c) 2002, 2003, 2004 Ilya Kovalenko <shadow@oganer.net>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/ioctl.h>
#include <sys/param.h>
#include <sys/types.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include <unistd.h>
#include <sysexits.h>
#include <syslog.h>

#include "bee.h"
#include "g3c.h"
#include "misc.h"

/* Internal functions */
static g3c_section * g3c_newsect(g3c_section * parent, int line);
static int           g3c_newparam(g3c_section * parent, char * name,
                                  void * value, int type, int line);
static int    g3c_strcmp(char * one, char * two);
static void * strmove(void * dst, void * src, int len);
static int    g3c_castvalue(void * value, int from, int to, void * buf);
static void * g3c_mkvalcopy(void * value, int type);
static void   g3ci_free(g3c_section * sect);
static int    g3ci_rdefsect(g3c_pos * pos);
static int    g3ci_gfmarklevel(int lev);
static int    g3ci_gfflush(g3c_file * file);
static int    safe_close(int);

/********************************************/ 
/* Reset (or initialize) position structure */
/* if ptr isnt zero - initialize structure  */
/********************************************/

int g3c_reset(g3c_pos * pos, g3c_section * config)
{
   if (config != NULL) pos->config = config;
   else if (pos->config == NULL) return (-1);

   pos->section = pos->config;
   pos->index   = 0;
   pos->pindex  = 0;
   pos->level   = 0;
   return 0;
}

/*********************************/
/* Switch to first child section */
/*********************************/

void   g3c_first (g3c_pos * pos)
{  pos->index = 0;
}

void   g3c_firstparam(g3c_pos * pos)
{  pos->pindex = 0;
}

/********************************/
/* Switch to next child section */
/********************************/

int    g3c_next  (g3c_pos * pos)
{
   if (pos->index >= pos->section->sc) return (-1);
   if (pos->level > DEPTH) return (-1);

   pos->up_section[pos->level] = pos->section;   /* store section pointer */
   pos->up_index[pos->level]   = pos->index + 1; /* store (index+1) */

   pos->section = pos->section->sections+pos->index; /* down-switch section */
   pos->level++;                                     /* increase depth */
   pos->index   = 0;                                 /* zero index */
   pos->pindex  = 0;

   return 0;
}

/****************************/
/* Return to parent section */
/****************************/

int    g3c_uplevel (g3c_pos * pos)
{
   if (pos->level < 1) return (-1);

   pos->level--;
   pos->section = pos->up_section[pos->level];   /* restore section ptr */
   pos->index   = pos->up_index[pos->level];     /* restore index */
   pos->pindex  = 0;

   return 0;
}

/**************************************/
/* Switch to next named child section */
/**************************************/

int    g3c_nextn (g3c_pos * pos, char * name)
{  int rc;

   do
   {  rc = g3c_next(pos);
      if (rc < 0) break;

      rc = g3c_strcmp((char*)g3c_getvalue(pos, "type", PT_STRING), name);
      if (rc != 0) g3c_uplevel(pos);
   } while (rc != 0);

   return rc;
}

/***************************************************/
/* Get named parameter value (no allocations made) */
/***************************************************/

void * g3c_getvalue (g3c_pos * pos, char * name, int type)
{  g3c_section * s    = pos->section;
   int           rc   = (-1);
   void        * pval = NULL;     /* ptr to value origin */
   int           i;

   for (i=0; i < s->pc; i++)
   {  if ((rc = g3c_strcmp(s->params[i].name, name)) == 0) break;
   }
   if (rc != 0) return NULL;
   pval = s->params[i].value;
   if (s->params[i].type == type) return pval;

   g3c_castvalue(pval, s->params[i].type, type, pos->valbuf);

   return pos->valbuf;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * *\ 
 *  Set parameter value (add parameter if not exist) *
\* * * * * * * * * * * * * * * * * * * * * * * * * * */

int g3c_setvalue(g3c_pos * pos, char * name, void * val, int type)
{  g3c_param    * param;
   void         * tmp;
   int            rc;

   if (pos == NULL || name == NULL || val == NULL) return (-1);
   param = g3c_getparam(pos, name);
   if (param == NULL)  /* if not exist - create */
   {  rc = g3c_newparam(pos->section, name, val, type, (-2));
/* 
 * line = -2 to prevent fdef flag setting (set if line = -1) 
 *           & indicate that parameter is lineless
 */
      if (rc < 0) return (-1);
      return 0;
   }
   if (type != param->type)
   {  rc = g3c_castvalue(val, type, param->type, pos->valbuf);
      if (rc <= 0) return (-1);
      val = &(pos->valbuf);
   }
   tmp = g3c_mkvalcopy(val, param->type);
   if (tmp == NULL) return (-1);

   FREE(param->value);
   param->value = tmp;
   param->fdef  = 0;
   if (pos->section->fdef) g3ci_rdefsect(pos);

   return 0;
}

/* Internal: reset default flag on section & it's parent branch */

int g3ci_rdefsect(g3c_pos * pos)
{  int i;

   if (pos->section->fdef)
   {  pos->section->fdef = 0;
      for (i = pos->level - 1; i >= 0; i--)
      {  if (pos->up_section[i]->fdef == 0) break;
         pos->up_section[i]->fdef = 0;
      }
   }
   return 0;
}

/* * * * * * * * * * * * * * *\
 *   Delete named parameter  *
\* * * * * * * * * * * * * * */

int g3c_delparam(g3c_pos * pos, char * name)
{  int         rc;
   g3c_param * param;
   
   param = g3c_getparam(pos, name);
   if (param == NULL) return (-1);
   FREE(param->name);
   FREE(param->value);
   rc = da_rm(&(pos->section->pc), (void**)&(pos->section->params), 
          sizeof(g3c_param), pos->pindex, NULL);
   return rc;   
}

/* * * * * * * * * * * * * * * * * *\
 *  Create & enter new subsection  *
\* * * * * * * * * * * * * * * * * */

int g3c_opensect(g3c_pos * pos, char * name)
{  g3c_section * sect;
   int           rc;

   if (pos->level > DEPTH) return (-1);

   sect = g3c_newsect(pos->section, (-2));  /* (-2) prevents fdef flag set */
   if (sect == NULL) return (-1);  
   rc = g3c_newparam(sect, "type", name, PT_STRING, (-2));
   if (rc < 0)
   {  g3ci_free(sect);
      da_rm(&(pos->section->sc), (void **)&(pos->section->sections),
            sizeof(g3c_section), pos->section->sc - 1, NULL);
      return (-1);
   }
   /* ensure not default branch */
   g3ci_rdefsect(pos);    
   
   /* switch to section (code duplication) */

   pos->up_section[pos->level] = pos->section;     /* store section pointer */
   pos->up_index[pos->level]   = pos->section->sc; /* store index (to end) */

   pos->section = sect;                         /* down-switch section */
   pos->level++;                                /* increase depth */
   pos->index   = 0;                            /* zero index */
   pos->pindex  = 0;                            /* zero parameter index */

   return 0;
}

/* * * * * * * * * * * *\  
 *  Delete subsection  *
\* * * * * * * * * * * */

int g3c_delsect (g3c_pos * pos, char * name)
{  int rc;

   rc = g3c_nextn(pos, name);
   if (rc < 0) return rc;
   g3c_uplevel(pos);
   pos->index--;
   g3ci_free(&(pos->section->sections[pos->index]));
   rc = da_rm(&(pos->section->sc), (void**)&(pos->section->sections),
         sizeof(g3c_section), pos->index, NULL);
   if (rc < 0) return (-1);

   return 0;   
} 

/*  Internal: make (allocate) value copy */
void * g3c_mkvalcopy (void * val, int type)
{  void * pret;

   if (val == NULL) return NULL;  
   
   switch (type)
   {  case PT_INTEGER:
        pret = calloc(1, sizeof(int));
        if (pret)
        {  *((int*)pret) = *((int*)val);
           return pret;
        }
        else syslog(LOG_ERR, "g3c_mkvalcopy(calloc): %m");
        break;
      case PT_LONGLONG:
        pret = calloc(1, sizeof(int64_t));
        if (pret)
        {  *((int64_t*)pret) = *((int64_t*)val);
           return pret;
        }
        else syslog(LOG_ERR, "g3c_mkvalcopy(calloc): %m");
        break;
      case PT_STRING:
        pret = calloc(1, strlen((char*)val) + 1);
        if (pret)
        {  strcpy(pret, val);
           return pret;
        }
        else syslog(LOG_ERR, "g3c_mkvalcopy(calloc): %m");
        break;
   }
   return NULL;
}


/**********************************/
/* Get pointer to named parameter */
/**********************************/

g3c_param * g3c_getparam(g3c_pos * pos, char * name)
{  g3c_section * s  = pos->section;
   int           rc = (-1);
   int           i;

   for (i=0; i < s->pc; i++)
   {  if ((rc = g3c_strcmp(s->params[i].name, name)) == 0) break;
   }
   if (rc != 0) return NULL;
   pos->pindex = i;

   return s->params + i;
}

/***************************************/
/* Cast value from one type to another */
/***************************************/

/* Returns string lenght including terminator ! (not bug - feature :) */

int g3c_castvalue(void * value, int from, int to, void * buffer)
{
   int len;

   switch (to)
   {  case PT_STRING:
         sprintf((char*)buffer, "%d", *((int*)value) );
	 len = strlen((char*)buffer) + 1;
         break;
      case PT_INTEGER:
         *((int*)buffer) = strtol((char*)value, NULL, 0);
	 len = sizeof(int);
         break;
      case PT_LONGLONG:
         *((int64_t*)buffer) = strtoll((char*)value, NULL, 0);
	 len = sizeof(int64_t);
         break;
      default:
         return 0;
   }

   return (len);
}

/*******************************************/
/* Get value with allocation memory for it */
/*******************************************/

void * g3c_allocvalue (g3c_pos * pos, char * name, int type)
{  void * pval;
   void * pret;

   pval = g3c_getvalue(pos, name, type);
   if (pval == NULL) return NULL;  
   
   switch (type)
   {  case PT_INTEGER:
        pret = calloc(1, sizeof(int));
        if (pret != NULL)
        {  *((int*)pret) = *((int*)pval);
           return pret;
        }
        break;
      case PT_LONGLONG:
        pret = calloc(1, sizeof(int64_t));
        if (pret != NULL)
        {  *((int64_t*)pret) = *((int64_t*)pval);
           return pret;
        }
        break;
      case PT_STRING:
        pret = calloc(1, strlen(pval)+1);
        if (pret != NULL)
        {  strcpy(pret, pval);
           return pret;
        }
        break;
   }
   return NULL;
}

char *
g3c_string(g3c_pos *pos, char *param)
{
	char *str;

	str = (char *)g3c_allocvalue(pos, param, PT_STRING);

	return (str);
}

int *
g3c_integer(g3c_pos *pos, char *param)
{
	int * num;

	num = (int *)g3c_getvalue(pos, param, PT_INTEGER);

	return (num);
}

int64_t *
g3c_longlong(g3c_pos *pos, char *param)
{
	int64_t *num;

	num = (int64_t *)g3c_getvalue(pos, param, PT_LONGLONG);

	return (num);
}

/********************************************************/
/* Compare strings & return 0 on success (-1) not equal */
/********************************************************/

int g3c_strcmp(char * one, char * two)
{   return strcmp(one, two) ? (-1) : 0;
}


/* * * * * * * * * * * * * * * *\
 *  Interrupt-protected close  *
\* * * * * * * * * * * * * * * */

int safe_close(int fd)
{  int rc;

   do
   {  rc = close(fd);
   } while(rc < 0 && errno == EINTR);

   return rc;
}

/* * * * * * * * * * * * * * * *\
 *  Initialize file structure  *
\* * * * * * * * * * * * * * * */

int    g3c_file_init    (g3c_file * file, const char * filespec)
{
   if (file == NULL)
   {  syslog(LOG_ERR, "g3c_file_init(): NULL pointer");
      return (-1);
   }

   memset(file, 0, sizeof(*file));

   if (filespec != NULL)
   {  file->filespec = strnalloc(filespec, 0);
      if (file->filespec == NULL)
      {  syslog(LOG_ERR,"g3c_file_init(strnalloc(filespec)): %m");
         return (EX_OSERR);
      }
   }
   return G3C_SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * *\
 *   Load file to file structure buffer  *
\* * * * * * * * * * * * * * * * * * * * */

int g3c_file_load(g3c_file * file, const char * filespec)
{  int fd;
   int ln;
   int rc;

   if (file == NULL)
   {  syslog(LOG_ERR, "g3c_file_load(): NULL pointer");
      return (-1);
   }

   if (filespec == NULL && file->filespec == NULL)
   {  syslog(LOG_ERR, "g3c_file_load(): NULL filespec");
      return (-1);
   }

/* Unload file if loaded (reload) */
   if (file->file != NULL) g3c_file_unload(file);

/* store new name if given */
   if (filespec != NULL)
   {  FREE(file->filespec);
      file->filespec = strnalloc(filespec, 0);
      if (file->filespec == NULL)
      {  syslog(LOG_ERR,"g3c_file_load(strnalloc(filespec)): %m");
         return (EX_OSERR);
      }
   }

/* open file (readonly, exclusive lock) */
   SIGSAFE(fd, open(file->filespec, O_RDONLY | O_EXLOCK, 0));
   if (fd == -1)
   {  syslog(LOG_ERR,"g3c_file_load(open(%s)): %m", file->filespec);
      return (EX_OSERR);
   }

/* determine file size */
   rc = ioctl(fd, FIONREAD, &ln);
   if (rc < 0)
   {  syslog(LOG_ERR, "g3c_file_load(ioctl): %m");
      return (EX_OSERR);
   }   

/* Allocate buffer */
   file->file = calloc(1, ln + 1);  /* allocate one more byte */
   if (file->file == NULL)
   {  syslog(LOG_ERR,"g3c_file_load(calloc(%d)): %m", ln);
      safe_close (fd);
      return (EX_OSERR);
   }
   file->len = ln;

/* Load file */
   ln = 0;
   do
   {  SIGSAFE(rc, read(fd, file->file + ln , file->len - ln));
      if (rc < 1)
      { 
	      syslog(LOG_ERR, "g3c_file_load(read(%d bytes)) = rc: %m", 
  	      	     file->len - ln);
         FREE(file->file);
         file->len = 0;
         safe_close(fd);
         return (EX_OSERR);
      } 
      ln += rc;  
   } while (ln < file->len);

   safe_close (fd);
   return G3C_SUCCESS; 
}

/* * * * * * * * * * * * * * * * *\
 *  Save config file from buffer *
\* * * * * * * * * * * * * * * * */

int g3c_file_save(g3c_file * file, const char * filespec)
{  int fd;
   int ln;
   int rc;

/* check structure ptr */
   if (file == NULL)
   {  syslog(LOG_ERR, "g3c_file_save(): NULL pointer");
      return (-1);
   }

/* if name is nor given - save with original name */
   if (filespec == NULL) filespec = file->filespec;

/* check filespec */
   if (filespec == NULL)
   {  syslog(LOG_ERR, "g3c_file_save(): NULL filespec");
      return (-1);
   }

/* check file existing on structure */
   if (file->file == NULL || file->len < 1)
   {  syslog(LOG_ERR, "g3c_file_save(): warning: freeing file with unknown size");
      FREE(file->file);
      file->len = 0;
   }
   if (file->file == NULL)
      syslog(LOG_ERR, "g3c_file_save(): Warning: saving empty file");

/* Open file (write, create, exlock, truncate) */
   SIGSAFE(fd, open(filespec, O_WRONLY | O_CREAT | O_EXLOCK | O_TRUNC, 0544));
   if (fd < 0) 
   {  syslog(LOG_ERR, "g3c_file_save(open): %m");
      return (EX_OSERR);
   }

/* Writing file */
   if (file->len > 0) 
   {  ln = 0;
      do
      { SIGSAFE(rc, write(fd, file->file + ln, file->len - ln));
         if (rc < 1) 
         {  
            syslog(LOG_ERR, "g3c_file_save(write(%d bytes)) = %d: %m", 
                         file->len - ln, rc);  
            safe_close(fd);
            unlink(filespec);         /* ! unlinking opened file */
            return (EX_OSERR);
         }

         ln += rc;
      }  while (ln < file->len);
   }

   safe_close(fd);

   return G3C_SUCCESS;
}

/* * * * * * * * * * * * * * *\
 *  Unload file from buffer  *
\* * * * * * * * * * * * * * */

int g3c_file_unload(g3c_file * file)
{  
   
/* check structure ptr */
   if (file == NULL)
   {  syslog(LOG_ERR, "g3c_file_unload(): NULL pointer");
      return (-1);
   }

   FREE(file->file);
   file->len = 0;

/* Does not free filespec */

   return G3C_SUCCESS;
}

/* * * * * * * * * * * * * * *\
 *   Destroy file structure  *
\* * * * * * * * * * * * * * */

int g3c_file_free(g3c_file * file)
{  int rc;

   rc = g3c_file_unload(file);
   if (rc < 0) return rc;

   FREE(file->filespec);

   return G3C_SUCCESS;
}



/* * * * * * * * * * * * * * * * * * * * * * *\
 *  Generate file from configuration image   *
\* * * * * * * * * * * * * * * * * * * * * * */

char   genbuf[256];  /* File line buffer */
int    gblen = 0;    /* string lenght (aka end position) */

int g3c_genfile(g3c_file * file, g3c_section * cfg)
{  
   if (file == NULL || cfg == NULL) return (-1);
/* Free last file (if loaded) */
   if (file->file != NULL) FREE(file->file);
   file->len = 0;
/* Initialize line buffer */
   genbuf[0] = '\0';
   gblen     = 0; 
/* UnParse main section */
   return g3c_gfsect(file, cfg, (-1));
}

/* Internal: printout nest level */
int g3ci_gfmarklevel(int lev)
{  int i;
   for (i=0; i<lev; i++)
      gblen += snprintf(genbuf + gblen, sizeof(genbuf) - gblen, "   ");
   return 0;
}

/* Internal: Flush genbuf to file */
int g3ci_gfflush(g3c_file * file)
{  void * tmp;

/* Add EOL to buffer */
   gblen += snprintf(genbuf + gblen, sizeof(genbuf) - gblen, "\n");
/* Realloc file */
   tmp = realloc(file->file, file->len + gblen + 1); /* one more byte (debug \0) */
   if (tmp == NULL)
   {  syslog(LOG_ERR, "g3ci_gfflush(realloc): %m");
      return (-1);
   }
   file->file = (char*)tmp;
/* Copy buffer */
   memmove(file->file + file->len, genbuf, gblen);
   file->len += gblen;                        
   file->file [file->len] = '\0';   /* add debug terminator */
/* Clear buffer */
   genbuf[0] = '\0';
   gblen     = 0;
/* Success reporting */
   return 0;
}

/* Internal: generate section on file (recursive) */
int g3c_gfsect(g3c_file * file, g3c_section * sect, int lev)
{  int i;

/* Skip section if it was added by default */
/* (default flags must be reset by add procedure) */
   if (sect->fdef != 0) return 0;

/* Open section */
   if (gblen == 0) g3ci_gfmarklevel(lev);
   if (lev >= 0) gblen += snprintf(genbuf+gblen, sizeof(genbuf)-gblen, "{  ");

/* dump section parameters */
   if (sect->params != NULL)
   {  for (i=0; i<sect->pc; i++)
      {  if (sect->params[i].fdef == 0)
         {  if (gblen == 0) g3ci_gfmarklevel(lev + 1);
            gblen += snprintf(genbuf + gblen, sizeof(genbuf) - gblen, 
                              "%s=", sect->params[i].name);
            if (sect->params[i].type == PT_STRING)
               gblen += snprintf(genbuf + gblen, sizeof(genbuf) - gblen, 
                               "%s", (char*)sect->params[i].value);
            else
               gblen += g3c_castvalue(sect->params[i].value,
                                     sect->params[i].type,
                                     PT_STRING, genbuf + gblen)
                       - 1; /* !!! because returned len includes \0 */
            if (g3ci_gfflush(file) < 0) return (-1);
         }
      } 
   }
/* dump subsections */
   if (sect->sections != NULL)
   {  for (i=0; i < sect->sc; i++)
      {  if (g3c_gfsect(file, &(sect->sections[i]), lev + 1) < 0) return (-1);
                     /* Direct recursion ! */
      }
   }
/* Close section */
   if (gblen == 0) g3ci_gfmarklevel(lev);
   if (lev >= 0) gblen += snprintf(genbuf + gblen, sizeof(genbuf) - gblen, "}");

   if (g3ci_gfflush(file) < 0) return (-1);
   return 0;
}

/*****************************************************/
/* Parse file to config (toplevel section) structure */
/*****************************************************/
int g3c_parse(g3c_file * file, g3c_section ** cfg, g3c_rulesec * rul)
{  int               i;                 /* main index if file */
   int               s;                 /* *work* */
   char            * text = file->file; /* file ptr */
   int               line      = 1;     /* line for error messages */
   int               comment   = 0;     /* in comment flag */
   int               mlcomment = 0;     /* in multiline comment flag */
   int               mlbreak   = 0;     /* in multiline break status */
   int               escape    = 0;     /* escape flag () */
   int               quotes    = 0;     /* in quotes flag */
   char              pname[NAME_LEN+1]; /* temporal name storage */
   char              pval[VALUE_LEN+1]; /* temporal value origin storage */
   g3c_section     * sec;               /* current subsection ptr */
   g3c_section     * tmp;               /* allocate temp */
   g3c_section     * up_sec[DEPTH];     /* section ptrs stack (LIFO) */
   int               up_levels = 0;     /* current stack size/depth level */
   char              ptok[TOKEN_LEN+1]; /* Token buffer */
   int               tlen      = 0;     /* Token lenght */
   int               tlim = NAME_LEN;   /* Token limitation */
   int               splen     = 0;     /* Token afterspace counter */

   pname[0] = '\0';
   pval[0]  = '\0';
   ptok[0]  = '\0';

   if (file == NULL || cfg == NULL) return (EX_OSERR);
  
   tmp = g3c_newsect(NULL, 0);
   if (tmp == NULL)  return (EX_OSERR);
   if (*cfg != NULL) g3c_free(*cfg);
   *cfg = tmp;
   sec  = tmp;
   if (g3c_newparam(sec, "type", "main", PT_STRING, (-1)) != G3C_SUCCESS)
   {  syslog(LOG_ERR, "g3c_parse(): can't add type");
      return (EX_OSERR);
   }

   for (i=0; i < file->len; i++)
   {  
      if (text[i] == '\n') line++;

      if (comment)  
      {  if (text[i] == '\xd' || text[i] == '\xa') comment = 0;
         continue;
      }

      if (mlcomment)
      {  if (text[i] == '\xd' || text[i] == '\xa') mlbreak = 1;
         else
         {  if (text[i] == '#' && mlbreak != 0) 
            {  mlbreak++;
               if (mlbreak > 2)
               {  mlcomment = 0;
                  comment   = 1;
                  mlbreak   = 0;
               }
            }
            else mlbreak = 0;   
         }
         continue;
      } 

      if (escape)
      {  escape = 0;
         if (tlen + splen < tlim)
         {  for (s=0; s < splen; s++) ptok[tlen++] = ' ';
            splen = 0;
            ptok[tlen++] = text[i];
            ptok[tlen]   = '\0';
         }
         continue;
      }
      if (! quotes)                /* if NO QUOTES */
      {  switch (text[i])
         {  case ' ':
            case '\t':
               if (tlen!=0 && tlen + splen < tlim)
               {  splen++;
               }
               break;  
            case '\"':
               quotes = 1;
               break;
            case '=':
               if (pname[0] != '\0' || tlen == 0)
               {  syslog(LOG_ERR,"%s: %d: error - Unexpected \"=\"",
                         file->filespec, line);
                  return EX_CONFIG;
               }
               strmove(pname, ptok, tlen);
               ptok[0] = '\0';
               tlen    = 0;
               splen   = 0;
               tlim    = VALUE_LEN;
               break; 
            case '\n':
            case '\r':
               if (tlen == 0 && pname[0] == '\0') break;
               if (tlen != 0 && pname[0] != '\0')
               {  strmove(pval, ptok, tlen);
                  g3c_newparam(sec, pname, pval, PT_STRING, line);
                  pval[0]  = '\0';
                  pname[0] = '\0';
                  ptok[0]  = '\0';
                  tlen     = 0;
                  splen    = 0;
                  tlim     = NAME_LEN;
               }
               else
               {  syslog(LOG_ERR,"%s: %d: error - Unexpected EOL",
                         file->filespec, line);
                  return EX_CONFIG;
               }
               break;
            case '{':
               if ((tlen == NULL) != (pname[0] == '\0'))
               {  syslog(LOG_ERR,"%s: %d: error - Unexpected \"{\"",
                         file->filespec, line);
                  return EX_CONFIG;   /* ON ERROR */
               }
               if (tlen != 0)     /* Store last parameter if present */
               {  strmove(pval, ptok, tlen);
                  g3c_newparam(sec, pname, pval, PT_STRING, line);
                  pval[0]  = '\0';
                  pname[0] = '\0';
                  ptok[0]  = '\0';
                  tlen     = 0;
                  splen    = 0;
                  tlim     = NAME_LEN;
               }
               if ((tmp = g3c_newsect(sec, line)) != NULL)
               {  if (up_levels < DEPTH)
                  {  up_sec[up_levels] = sec;
                     up_levels++;
                     sec = (g3c_section*)tmp;  
                  }
                  else
                  {  syslog(LOG_ERR,"%s: %d: error - Depth limit ! "
                            "(%d levels)", file->filespec, line, DEPTH);
                     FREE(tmp);
                     return EX_CONFIG;
                  }
               }
               break;
            case '}':
               if ((tlen == 0) != (pname[0] == '\0'))
               {  syslog(LOG_ERR,"%s: %d: error - Unexpected \"}\"",
                         file->filespec, line);
                  return EX_CONFIG;   /* ON ERROR */
               }
               if (tlen != 0)    /* store last parameter if present */
               {  strmove(pval, ptok, tlen);
                  g3c_newparam(sec, pname, pval, PT_STRING, line);
                  pval[0]  = '\0';
                  pname[0] = '\0';
                  ptok[0]  = '\0';
                  tlen     = 0;
                  splen    = 0;
                  tlim     = NAME_LEN;
               }
               if (up_levels > 0)
               {  
                  if (g3c_rulecheck(sec, rul, file->filespec) != G3C_SUCCESS)
                     return EX_CONFIG;
                  up_levels--;
                  sec = up_sec[up_levels];
               }
               else
               {  syslog(LOG_ERR,"%s: %d: error - Extra \"}\"",
                          file->filespec, line);
                  return EX_CONFIG;
               }
               break;
            case '#':
               if ((tlen == 0) != (pname[0] == '\0'))
               {  syslog(LOG_ERR,"%s: %d: error  - Unexpected \"#\"",
                         file->filespec, line);
                  return EX_CONFIG;   /* ON ERROR */
               }
               if (tlen != 0)    /* store last parameter if present */
               {  strmove(pval, ptok, tlen);
                  g3c_newparam(sec, pname, pval, PT_STRING, line);
                  pval[0]  = '\0';
                  pname[0] = '\0';
                  ptok[0]  = '\0';
                  tlen     = 0;
                  splen    = 0;
                  tlim     = NAME_LEN;
               }
               if (text[i + 1] == '#') /* hack: possible overread (fail-safe) */
               {  mlcomment = 1;
                  mlbreak   = 0;
               }
               else comment = 1;
               break;
            case '\\':
               escape = 1;
               break;
            default:
               if (tlen+splen < tlim)
               {  for (s=0; s < splen; s++) ptok[tlen++] = ' ';
                  splen        = 0;
                  ptok[tlen++] = text[i];
                  ptok[tlen]   = '\0';
               }
         }
         continue;
      } /* if not quotes */
      else
      {  if (text[i] != '\"')
         {  if (tlen + splen < tlim)
            {  for (s=0; s < splen; s++) ptok[tlen++] = ' ';
               splen        = 0;
               ptok[tlen++] = text[i];
               ptok[tlen]   = '\0';
            }
         }
         else quotes = 0;
         continue;
      }   
   } 
   if (up_levels != 0 || tlen != 0)
   {  syslog(LOG_ERR,"%s: %d: error - Unexpected EOF", file->filespec, line);
      return EX_CONFIG;
   }

   return 0;
   return g3c_rulecheck(sec, rul, file->filespec);
}

/*******************************************/
/* Check new section with given rules list */
/*******************************************/

int g3c_rulecheck (g3c_section * sec, g3c_rulesec * rules, char * filespec)
{  
   g3c_pos          intpos;
   g3c_pos        * pos = &intpos;

   g3c_rulesec    * secrule;
   g3c_ruleparam  * prmrule;
   g3c_param      * pparam;
   int              valsz;
   void           * tmp;
   int              s, i, j, chk;
   int              rc;
   g3c_section    * subsect;

   g3c_reset(pos, sec);

   tmp = g3c_getvalue(pos, "type", PT_STRING);
   if (tmp == NULL)
   {  syslog(LOG_ERR, "%s: %d: error: Mistyped section", filespec,
                                sec->line);
      return EX_CONFIG;
   }

/*   printf("section %s\n", (char*)tmp); */

   if (rules == NULL) return G3C_SUCCESS;          /* no rules at all */

   s = 0;
   while (rules[s].type != NULL)
   {  
      if (g3c_strcmp((char*)tmp, rules[s].type) == G3C_SUCCESS)  break;
      else s++;
   }
   if (rules[s].type == NULL)
   {   
      return G3C_SUCCESS; /* no rules 4 section */
   }
   secrule = rules + s;

   for (i=0; i < secrule->pc; i++)
   {  prmrule = secrule->params + i;
      if (prmrule->name != NULL)
      {  pparam = g3c_getparam(pos, prmrule->name);
         if (pparam == NULL)
         {  if (prmrule->required)
            {  syslog(LOG_ERR, "%s: %d: error: \"%s\" requred in"
                               " \"%s\" section",
                               filespec,
                               sec->line,
                               prmrule->name,
                               secrule->type);
               return EX_CONFIG;
            }
            if (prmrule->defvalue != NULL)
            {  if ((rc = g3c_newparam(sec, prmrule->name, prmrule->defvalue,
                                       prmrule->type, -1)) != G3C_SUCCESS)
               {  syslog(LOG_ERR, "g3c_rulecheck(): can't add parameter");
                  return rc;  
               }
            } 
         }
         else
         {  if (prmrule->type != pparam->type)
            {  valsz = g3c_castvalue(pparam->value, 
                  pparam->type, prmrule->type, pos->valbuf);
               tmp = realloc(pparam->value, valsz);
               if (tmp != NULL) 
               {  pparam->value = tmp;
                  memmove(pparam->value, pos->valbuf, valsz);
                  pparam->type = prmrule->type;
               }
               else
               {  syslog(LOG_ERR, "g3c_rulecheck(realloc(%d)): %m", valsz);
                  return EX_OSERR;
               }               
            }
            switch (prmrule->type)
            {
            case PT_INTEGER:
               if ( *((int*)pparam->value) > prmrule->vmax)
               {  __extension__ syslog(LOG_ERR, "%s: %d: error: value \"%s\" > %lld in"
                               " \"%s\" section at %d",
                               filespec,
                               pparam->line,
                               prmrule->name,
                               prmrule->vmax,
                               secrule->type,
                               sec->line);
                  return EX_CONFIG;
               }
               if ( *((int*)pparam->value) < prmrule->vmin)
               { __extension__ syslog(LOG_ERR, "%s: %d: error: \"%s\" < %lld in"
                               " \"%s\" section at %d",
                               filespec,
                               pparam->line,
                               prmrule->name,
                               prmrule->vmin,
                               secrule->type,
                               sec->line);
                  return EX_CONFIG;
               }
               /* calling check function */
               if (prmrule->check != NULL && 
                   prmrule->check(pparam->value) != G3C_SUCCESS)
               {  syslog(LOG_ERR, "check function failed for paramerer: %s",
                          prmrule->name);
                  return(EX_CONFIG);
               }
               break;
            case PT_LONGLONG:
               if ( *((int64_t*)pparam->value) > prmrule->vmax)
               { __extension__ syslog(LOG_ERR, "%s: %d: error: value \"%s\" > %lld in"
                               " \"%s\" section at %d",
                               filespec,
                               pparam->line,
                               prmrule->name,
                               prmrule->vmax,
                               secrule->type,
                               sec->line);
                  return EX_CONFIG;
               }
               if ( *((int64_t*)pparam->value) < prmrule->vmin)
               { __extension__ syslog(LOG_ERR, "%s: %d: error: \"%s\" < %lld in"
                               " \"%s\" section at %d",
                               filespec,
                               pparam->line,
                               prmrule->name,
                               prmrule->vmin,
                               secrule->type,
                               sec->line);
                  return EX_CONFIG;
               }
               /* calling check function */
               if (prmrule->check != NULL &&
                    prmrule->check(pparam->value) != G3C_SUCCESS)
               {  syslog(LOG_ERR, "check function failed for paramerer: %s", prmrule->name);
                  return(EX_CONFIG);
               }
               break;
            case PT_STRING:
               if ( strlen((char*)pparam->value) > prmrule->vmax)
               { __extension__ syslog(LOG_ERR, "%s: %d: warning: \"%s\" > %lld chars in"
                               " \"%s\" section at %d: cutting off",
                               filespec,
                               pparam->line,
                               prmrule->name,
                               prmrule->vmax,
                               secrule->type,
                               sec->line);
                  ((char*)pparam->value)[prmrule->vmax]='\0'; 
               }
               /* check permitted values */
               if (prmrule->pvl != NULL) 
               {  
               	  chk = 0;
               	  for (j = 0; 
                       prmrule->pvl[j] != NULL && 
                       (chk = strcmp(prmrule->pvl[j], (char*)pparam->value));
                       ++j);
               	  if (chk)
               	  {
               	     syslog(LOG_ERR, "invalid parameter value \"%s\" for parameter \"%s\"",
               	            (char *)pparam->value, pparam->name);
               	     return (EX_CONFIG);
               	  }
               }
               /* calling check function */
               if (prmrule->check != NULL &&
                    prmrule->check(pparam->value) != G3C_SUCCESS)
               {  syslog(LOG_ERR, "check function failed for paramerer: %s", prmrule->name);
                  return(EX_CONFIG);
               }
               break;
            default:
               syslog(LOG_ERR, "g3c_rulecheck(): Strange error ...");
               return EX_OSERR;
            } /* switch(type) */ 
         } /* else (i.e. if pparam != NULL) */
      } /* if rulename != NULL*/  
   } /* for (params) */

   for (i=0; i < secrule->sc; i++)
   {  if (secrule->sections[i].type != NULL)
      {  g3c_first(pos);
         g3c_first(pos);
         rc = g3c_nextn(pos, secrule->sections[i].type);
         g3c_uplevel(pos);
         if (rc != G3C_SUCCESS)
         {  if (secrule->sections[i].required)
            {  syslog(LOG_ERR, "%s: --: error: \"%s\" subsection requred in"
                            " \"%s\" section at %d",
                            filespec,
                            secrule->sections[i].type,
                            secrule->type,
                            sec->line);
               return EX_CONFIG;
            }
            if (secrule->sections[i].def)
            {  subsect = g3c_newsect(sec, -1);
               if (subsect == NULL)
               {  syslog(LOG_ERR, "g3c_rulecheck(): can't create section");
                  return EX_OSERR;
               }
               rc = g3c_newparam(subsect, "type", secrule->sections[i].type,
                                PT_STRING, -1);
               if (rc != G3C_SUCCESS)
               {  syslog(LOG_ERR, "g3c_rulecheck(): can't create type parameter");
                  return EX_OSERR;
               }
               rc = g3c_rulecheck(subsect, secrule->sections + i, filespec);  /* direct recurse !!! */
            }
         }  /* if section not present */
      }  /* if ruletype != NULL */
   } /* for (sections) */ 

   return G3C_SUCCESS;      
}

/*************************************/
/* Create new (sub)section (untyped) */
/*************************************/

g3c_section * g3c_newsect (g3c_section * parent, int line)
{  void * tmp;
   void * ret;
   int    ind;
   
   if (parent == NULL)
   {  ret = calloc(1, sizeof(g3c_section));
      if (ret == NULL)
      {  syslog(LOG_ERR, "g3c_newsect(calloc(%u)): %m",
                sizeof(g3c_section));
      }
      return (g3c_section*) ret;
   } 
   else
   {  ind = parent->sc++;
      tmp = realloc(parent->sections, sizeof(g3c_section)*(parent->sc));
      if (tmp == NULL)
      {  syslog(LOG_ERR, "g3c_newsect(realloc(%u)): %m",
                sizeof(g3c_section)*(parent->sc));
         parent->sc--;
         return NULL;
      }
      parent->sections = (g3c_section*)tmp;

      parent->sections[ind].sc       = 0;
      parent->sections[ind].pc       = 0;
      parent->sections[ind].sections = NULL;
      parent->sections[ind].params   = NULL;
      parent->sections[ind].line     = line;
      parent->sections[ind].fdef     = (line == -1);
      return parent->sections + ind;
   }
}

/***********************************************/
/* Create (add) new parameter to given section */
/***********************************************/

int g3c_newparam (g3c_section * parent, char * name, void * value, 
                    int type, int line)
{  void * tmp;
   int    ind;
   
   if (parent == NULL || name == NULL || value == NULL || type > PT_LAST) 
      return (-1);

   ind = parent->pc++;
   tmp = realloc(parent->params, sizeof(g3c_param)*parent->pc);
   if (tmp == NULL)
   {  syslog(LOG_ERR, "g3c_newparam(realloc(%u)): %m",
             sizeof(g3c_param)*(parent->pc));
      parent->pc--;
      return EX_OSERR;
   }
   parent->params            = (g3c_param*)tmp;
   parent->params[ind].type  = type;
   parent->params[ind].line  = line;
   parent->params[ind].fdef  = (line == -1);
   parent->params[ind].name  = NULL;
   parent->params[ind].value = NULL;

   if ((parent->params[ind].name = (char*)calloc(1, strlen(name) + 1)) == NULL)
   {  syslog(LOG_ERR, "g3c_newparam(calloc(%u)): %m",
             sizeof(g3c_param)*(parent->pc));
      parent->pc--;
      return EX_OSERR;
   }
   strcpy(parent->params[ind].name, name);
   switch (type)
   {  case PT_STRING: 
        if ((parent->params[ind].value = calloc(1, strlen((char*)value) + 1)) 
              != NULL) strcpy((char*)parent->params[ind].value, (char*)value);
        break;
      case PT_INTEGER:
        if ((parent->params[ind].value = calloc(1, sizeof(int))) != NULL)
           *((int*)parent->params[ind].value) = *((int*)value);
        break;
      case PT_LONGLONG:
        if ((parent->params[ind].value = calloc(1, sizeof(int64_t))) != NULL)
           *((int64_t*)parent->params[ind].value) = *((int64_t*)value);
        break;
      default:
        parent->params[ind].value = NULL;
   }

   if (parent->params[ind].value == NULL)
   {  syslog(LOG_ERR, "g3c_newparam(calloc(..)): %m");
      FREE(parent->params[ind].name);
      parent->pc--;
      return EX_OSERR;
   }

   return G3C_SUCCESS;
}

/*********************************************/
/* Internal function - free section contens, */ 
/* but do not free structure !!!             */
/*********************************************/

void g3ci_free (g3c_section * sec)
{  int i;

   if (sec != NULL)
   {  if (sec->params != NULL)
      {  for (i=0; i < sec->pc; i++)
         {  FREE(sec->params[i].name);
            FREE(sec->params[i].value);
         }
         FREE(sec->params);
      }
      if (sec->sections != NULL)
      {  for(i=0; i < sec->sc; i++) 
           g3ci_free(sec->sections + i);           /* direct recurse ! */
         FREE(sec->sections);
      }
   }
}

/************************************************************/
/* Free section (with subsections) & free section structure */
/************************************************************/

void g3c_free (g3c_section * sec)
{  
   if (sec != NULL)
   {  g3ci_free(sec);
      FREE(sec);
   }
}

/***********************************************/
/* Move (copy) given no of symbols into string */
/***********************************************/

void * strmove(void * dst, void * src, int len)
{
   ((char*)dst)[len] = '\0';
   return memmove(dst, src, len);
}
