// Hotkeys define file (Unix (Linux) version)

// Menu IDs (fictive keys)
#define K_MENUID       0x400
#define MI_LFULL       0x401
#define MI_LBRIEF      0x402
#define MI_LCUSTOM     0x403
#define MI_LINFO       0x404
#define MI_LTREE       0x405
#define MI_LSORT       0x406
#define MI_LFILTER     0x407
#define MI_LHIDE       0x408
#define MI_LREREAD     0x409
#define MI_LMOUNT      0x40A
#define MI_RFULL       0x411
#define MI_RBRIEF      0x412
#define MI_RCUSTOM     0x413
#define MI_RINFO       0x414
#define MI_RTREE       0x415
#define MI_RSORT       0x416
#define MI_RFILTER     0x417
#define MI_RHIDE       0x418
#define MI_RREREAD     0x419
#define MI_RMOUNT      0x41A

#define MI_CH1	         0x420
#define MI_CH2           0x421
#define MI_CH3           0x422
#define MI_MOUNTDISABLE  0x423
#define MI_MOUNTHIDE     0x424 

#ifdef UNIX  /* Unix Constants */

#define M_KEY		0xffff        /* Key mask */  
#define K_TERMRESIZE	0x019a
#define K_ALT		0x0200	      /* Alt shifted */

#define K_RTARROW		0x0105
#define K_LTARROW		0x0104
#define K_UPARROW		0x0103
#define K_DNARROW		0x0102
#define K_PGUP			0x0153

#define K_PGDN			0x0152
#define K_SPACE			0x0020
#define K_TAB			0x0009

//#define K_CTRL_RTARROW
//#define K_CTRL_LTARROW
#define K_CTRL_S		0x0013
#define K_CTRL_D		0x0004
#define K_CTRL_A		0x0001
#define K_CTRL_F		0x0006

// Linux: #define K_HOME   		0x0106
// Linux: #define K_END			0x0168

#define K_HOME   		0x016a
#define K_END			0x0181

//#define K_CTRL_HOME
//#define K_CTRL_END 		
#define K_BS   			0x0107
#define K_DEL   		0x014a
#define K_CTRL_G		0x0007
#define K_ESC   		0x001b
#define K_CTRL_Y		0x0019
#define K_ENTER 		0x000a
#define K_CTRL_BS  		0x0008	/* !!! */
#define K_CTRL_W		0x0017
#define K_CTRL_T		0x0014
#define K_CTRL_K		0x000b

#define K_CTRL_A		0x0001
#define K_CTRL_B		0x0002
#define K_CTRL_C		0x0003
#define K_CTRL_D		0x0004
#define K_CTRL_E		0x0005
#define K_CTRL_F		0x0006
#define K_CTRL_G		0x0007
#define K_CTRL_H		0x0008
#define K_CTRL_I		0x0009
#define K_CTRL_J		0x000a
#define K_CTRL_K		0x000b
#define K_CTRL_L		0x000c
#define K_CTRL_M		0x000d
#define K_CTRL_N		0x000e
#define K_CTRL_O		0x000f
#define K_CTRL_P		0x0010
#define K_CTRL_Q		0x0011
#define K_CTRL_R		0x0012
#define K_CTRL_S		0x0013
#define K_CTRL_T		0x0014
#define K_CTRL_U		0x0015
#define K_CTRL_V		0x0016
//#define K_CTRL_W		0x0017
#define K_CTRL_X		0x0018
#define K_CTRL_Y		0x0019
#define K_CTRL_Z		0x001a

#define K_F(n)			(0x0108+n)




#else        /* DOS Constants */

#define M_KEY
#define M_SHIFT			1
#define M_CTRL			2
#define M_ALT			4
#define M_SCROLL		8
#define M_NUM			16
#define M_CAPS			32
#define M_INSERT		64
 //   Command line keys
#define K_RTARROW		0x04d00
#define K_LTARROW		0x04b00
#define K_CTRL_RTARROW		0x27400
#define K_CTRL_LTARROW		0x27300
#define K_CTRL_S		0x21f13
#define K_CTRL_D		0x22004
#define K_CTRL_A		0x21e01
#define K_CTRL_F		0x22106
#define K_HOME   		0x04700
#define K_END			0x04f00
#define K_CTRL_HOME		0x27700
#define K_CTRL_END 		0x27500
#define K_BS   			0x00e08
#define K_DEL   		0x05300
#define K_CTRL_G		0x22207
#define K_ESC   		0x0011b
#define K_CTRL_Y		0x21519
#define K_ENTER 		0x01c0d
#define K_CTRL_BS  		0x20e7f
#define K_CTRL_W		0x21117
#define K_CTRL_T		0x21414
#define K_CTRL_K		0x2250b

#endif /* ifdef UNIX */
