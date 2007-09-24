/* $oganer: g3c.h,v 1.1 2005/07/17 16:28:31 shadow Exp $ */

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

#ifndef _G3C_H
#define _G3C_H

#define PT_STRING	(0)
#define PT_INTEGER	(1)
#define PT_LONGLONG	(2)
#define PT_LAST		PT_LONGLONG

#define VAL_NOMIN	(__extension__ LLONG_MIN)
#define VAL_NOMAX	(__extension__ LLONG_MAX)

#define DEPTH		(8)	/* maximal subsection depth */
#define NAME_LEN	(40)	/* max param name len */
#define VALUE_LEN	(128)	/* max value len */
#define TOKEN_LEN	MAX(NAME_LEN, VALUE_LEN)

#define FREE(p)	\
	if ((p) != NULL) {	\
		free(p);	\
		(p)=NULL;	\
	}			\

#define G3C_SUCCESS		(0)

#define PRM_NOTREQ		(0)
#define PRM_REQ			(1)
#define SECTION_NOTREQ		(0)
#define	SECTION_REQ		(1)

typedef struct g3c_param	g3c_param;
typedef struct g3c_pos		g3c_pos;
typedef struct g3c_rulesec	g3c_rulesec;
typedef struct g3c_ruleparam	g3c_ruleparam;
typedef struct g3c_file		g3c_file;
typedef struct g3c_section	g3c_section;

/* (sub-)Section structure */
struct g3c_section
{
	int		 sc;		/* Subsection count */
	int		 pc;		/* Parameters count */
	g3c_param	*params;	/* !! Array of parameter structs */
	g3c_section	*sections;	/* !! Array of subsection structs */
	int		 fdef;		/* "inserted by default" flag (dont save) */
	int		 line;		/* starting line */
};

/* Parameter structure */
struct g3c_param
{
	char		*name;		/* name of parameter */
	int		 type;		/* parameter type */
	void		*value;		/* pointer to value */	
	int		 fdef;		/* "inserted by default" flag */
	int		 line;		/* line in config (for error messages) */

};

/* Query position structute */
struct g3c_pos
{
	g3c_section	*config;	/* pointer to parsed configuration */
	int		 index;		/* index of current subsection */
	int		 pindex;	/* index of current parameter */
	g3c_section	*section;	/* pointer to current section */
	int		 level;		/* subsection depth (stack size) */
	int		 up_index[DEPTH];	/* stack of indexes */
	g3c_section	*up_section[DEPTH];	/* stack of section pointers */
	char		 valbuf[VALUE_LEN];	/* value buffer */
};

/* Rule for section */
struct g3c_rulesec
{
	int		 sc;		/* Subsection count */
	int		 pc;		/* Parameters count */
	char		*type;		/* Section type string */
	g3c_rulesec	*sections;	/* Array of subsection rules */
	g3c_ruleparam	*params;	/* Array of rules for parameters */
	int		 def;		/* "add by default" flag */
	int		 required;	/* "requred" flag (error if not exist) */
};

/* Rule for parameter */
struct g3c_ruleparam
{
	char		*name;		/* parameter name */
	int		 type;		/* type of value */
	int		 required;	/* "requred" flag (error if not exist) */
	void		*defvalue;	/* default value */
	int64_t		 vmin;		/* minimal value */
	int64_t		 vmax;		/* maximal value (or lenght) */
	char	 	**pvl;		/* permitted values list (g3c internal checking, PT_STRING only) */
	int		(*check)(void *);	/* value check function (external checikng, all types)*/
};

struct g3c_file
{
	int		 len;
	char		*file;
	char		*filespec;
};

__BEGIN_DECLS

/* File layer functions */
int    g3c_file_init  (g3c_file * file, const char * filespec);
int    g3c_file_free  (g3c_file * file);

int    g3c_file_load  (g3c_file * file, const char * filespec);
int    g3c_file_save  (g3c_file * file, const char * filespec);
int    g3c_file_unload(g3c_file * file);

/* Image (parse/generate) layer functions */
int    g3c_parse    (g3c_file * file, g3c_section ** cfg, g3c_rulesec * rul);
int    g3c_genfile  (g3c_file *, g3c_section *);    
int    g3c_gfsect   (g3c_file *, g3c_section *, int);    
int    g3c_rulecheck(g3c_section *, g3c_rulesec *, char *);
void   g3c_free     (g3c_section *);

/* Work (read) layer functions */
/* Reset position to root */
int    g3c_reset(g3c_pos *, g3c_section *);
/* Get parameter value (no allocations) */
void * g3c_getvalue(g3c_pos *, char *, int);
/* Get parameter value (with allocating memory) */
void * g3c_allocvalue(g3c_pos *, char *, int);
/* get string */
char * g3c_string(g3c_pos *, char *);
/* get integer ptr */
int * g3c_integer(g3c_pos *, char *);
/* get longlong ptr */
int64_t * g3c_longlong(g3c_pos *, char *);
/* Reset parameter number */
void   g3c_firstparam(g3c_pos *);
/* Get parameter structure pointer */
g3c_param * g3c_getparam(g3c_pos *, char *);
/* Reset subsection number (do not enter to) */
void   g3c_first(g3c_pos *);
/* Get (enter to) next subsection */
int    g3c_next(g3c_pos *);
/* Get (enter to) next subsection with given vame (search) */
int    g3c_nextn(g3c_pos *, char *);
/* Return back from subsection */
int    g3c_uplevel(g3c_pos *);

/* Work (write) layer functions */
/*
   TODO: 
     change parameter value (ensure not in default branch)
     add parameter & set    ( --//-- )
     add section enter it   (ensure not in default branch)
     delete parameter       (if not default)
     delete section         (if not default)
*/
/* set new value (convert) or create parameter & set value */
int g3c_setvalue(g3c_pos *, char *, void *, int);
/* delete named parameter (if not default) */
int g3c_delparam(g3c_pos *, char *);
/* create & enter new subsection (typed) */
int g3c_opensect(g3c_pos *, char *);
/* delete typed section */
int g3c_delsect(g3c_pos *, char *);

__END_DECLS

#endif			/* _G3C_H */
