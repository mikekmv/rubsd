// File shell
// dialogs headers file

// Dialog returns
#define ID_MEMOUT	(-1)
#define ID_CANCEL	0
#define ID_OK		1
#define ID_YES		1
#define ID_NO		2

// Dialog styles
#define DS_STATIC       1	/* static dialog memory         */
#define DS_PROLOGED	2	/* enable prologue proc() call  */
#define DS_EPILOGED     4	/* enable epilogue proc() call  */
//#define DS_MODELESS	8	/* ... in development */
#define DS_EVENTS       16	/* enable event proc() call     */
#define DS_PRIMARY	32	/* enable event proc() call 1st */
#define DS_NOTIFY	64	/* enable controls reports      */
#define DS_ARROWS	128	/* enable switch by arrows      */
#define DS_FRAMED	256	/* draw framed dialog   */

// Dialog Alignment (internal)
#define DA_DEFAULT	0
#define DA_LINE		1
#define DA_COLUMN	2
#define DA_NOCENTERL	4
#define DA_NOCENTERC	8
#define DA_HALFL	16
#define DA_HALFC	32
#define DA_PROC		64  /* Alignment by proc() */

// Dialog alignment (user)
#define DA_UP		DA_NOCENTERL | DA_DEFAULT
#define DA_DOWN		DA_NOCENTERL | DA_LINE
#define DA_LEFT		DA_NOCENTERC | DA_DEFAULT
#define DA_RIGHT	DA_NOCENTERC | DA_COLUMN
#define DA_HALFUP	DA_HALFL | DA_DEFAULT
#define DA_HALFDOWN	DA_HALFL | DA_LINE
#define DA_HALFLEFT	DA_HALFC | DA_DEFAULT
#define DA_HALFRIGHT	DA_HALFC | DA_COLUMN

// Dialog procedure actions
#define PA_PROLOGUE	1
#define PA_EPILOGUE	2
#define PA_ALIGN	3
#define PA_EVENT	4
#define PA_PRIMEVENT	5
#define PA_NOTIFY	6
#define PA_INITCTRL	7

// Control styles
#define CS_DEFAULT	0
#define CS_DISABLED	1

#define CS_HIDDEN	2
#define CS_EXTERN	4
#define CS_EXTBUF	8
#define CS_EXTLST	16

// Generic control types
#define CT_GENTYPE	0x000000ff
#define	CT_GROUP	0
#define CT_BUTTON	1
#define CT_CHECKBOX	2
#define	CT_STATIC	3
#define CT_SEPARATOR	4

// Button subtypes
#define CT_MENULIKE	0x100

// Combo types
#define CBT_MASK	3

#define CBT_EDIT	0  /* DO NOT change ! */
#define CBT_COUNT	1  /* DO NOT change ! */
#define CBT_SIMPLE	2  /* DO NOT change ! */
#define	CBT_DROPDN	3  /* DO NOT change ! */

#define CBT_NUMERIC	4
#define CBT_FLOAT	8
#define CBT_SIGNED	16
#define CBT_READONLY	32

// Control proc actions
#define CA_DRAW		1
#define CA_DRAWFOCUS	2
#define CA_EVENT	3
#define CA_SET		4
#define CA_SETTEXT	5

#define CAS_NODRAW	0x8000

// Control Notify
#define CN_FOCUS	1
#define CN_KILLFOCUS	2
#define CN_CLICKED	3

class LISTBOX
{  public:
   int		pcs;
   int		disp;
   char     **	array;
};

class COMBOX
{  public:
   int		len;     // edit
   int		pos;     // edit
   int		disp;    // edit
   char	     *  buff;    // edit
   float	step;    // count
   int		h;	 // drop size
   int		w;	 // drop size
   LISTBOX	lst;	 // list
};

class CONTROL
{   public:
    int		l;
    int		c;
    int		h;
    int		w;
    ulong	style;
    ulong	id;
    ulong	(*proc)(CONTROL * th, void * parent, int action, ulong param);
    ulong	type;
    char    *   szzText;
    int		bytes;
    int		limit;
    void     *  mem;
    int		notify;
    ulong	val;
};

class SCHEME
{   public:
    char	frame;		// frame & background
    char	title;		// title
    char	text[2];	// text & boxes (disabled, enabled)
    char	edit[2];	// edit box (disabled, enabled)
    char	button[3];	// buttons (disabled, enabled, lighted)
    char	lst[3];	// listbox (unselected, selected, lighted)
};

class DIALOGDATA
{   public:
    int 	Focus;
    int		fDraw;
    int		fDrawCur;
    int		fFList;
    LISTBOX   * FList;
    CONTROL   *	FLctrl;
};

class DIALOG
{   public:
    int		l;
    int		c;
    int		h;
    int		w;
    ulong	style;
    int		align;
    int   	scheme;
    char   *	title;
    int		(*proc)(DIALOG * th, int action, ulong param);
    int		procbytes;
    DIALOGDATA *mem;
    void    *   procmem;
    int		bytes;
    CONTROL *   ctrl;

    int		Dialog(ulong param);
    int		Dispatch();
    int		SetFocus(int n);
};

class RBSETCLASS // Class passed as param in CA_SET for radiobutton
{  public:
   short  index;
   short  first;
};

extern DIALOG		testDialog;
extern DIALOG		PullDn;
extern int		fDialogShadows;
	
ulong	GenControl      (CONTROL *th, void *parent, int action, ulong param);
ulong	RadioBoxControl (CONTROL *th, void *parent, int action, ulong param);
ulong	RadioBoxControl (CONTROL *th, void *parent, int action, ulong param);
ulong	ListBoxControl  (CONTROL *th, void *parent, int action, ulong param);
ulong	ComboBoxControl (CONTROL *th, void *parent, int action, ulong param);
