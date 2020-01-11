/* Cheap sed, an implementation of the sed utility
   Copyright (C) 1988, 89, 90, 91 Eric S Raymond
   Copyright (C) 1991 Howard L. Helman and David Kirschbaum
   Copyright (C) 2003 Laurent Vogel
  
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
  
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

/* sed.h -- types and constants for the stream editor */

#ifndef SED_H
#define SED_H

typedef unsigned char uchar;

/* data area sizes used by both modules */
#define MAXTAGS         9       /* tagged patterns are \1 to \9 */

/* constants for compiled-command representation */
#define NOCMD   0x00            /* no command                               */
#define EQCMD   0x01            /* = -- print current line number           */
#define ACMD    0x02            /* a -- append text after current line      */
#define BCMD    0x03            /* b -- branch to label                     */
#define CCMD    0x04            /* c -- change current line                 */
#define DCMD    0x05            /* d -- delete all of pattern space         */
#define CDCMD   0x06            /* D -- delete first line of pattern space  */
#define GCMD    0x07            /* g -- copy hold space to pattern space    */
#define CGCMD   0x08            /* G -- append hold space to pattern space  */
#define HCMD    0x09            /* h -- copy pattern space to hold space    */
#define CHCMD   0x0A            /* H -- append hold space to pattern space  */
#define ICMD    0x0B            /* i -- insert text before current line     */
#define LCMD    0x0C            /* l -- print pattern space in escaped form */
#define NCMD    0x0D            /* n -- get next line into pattern space    */
#define CNCMD   0x0E            /* N -- append next line to pattern space   */
#define PCMD    0x0F            /* p -- print pattern space to output       */
#define CPCMD   0x10            /* P -- print first line of pattern space   */
#define QCMD    0x11            /* q -- exit the stream editor              */
#define RCMD    0x12            /* r -- read in a file after current line   */
#define SCMD    0x13            /* s -- regular-expression substitute       */
#define TCMD    0x14            /* t -- branch on last successful s///      */
#define WCMD    0x15            /* w -- write pattern space to file         */
#define XCMD    0x16            /* x -- exhange pattern and hold spaces     */
#define YCMD    0x17            /* y -- transliterate text                  */

typedef struct cmd_t {          /* compiled-command representation */
  union {
    uchar *re;
    long num;
  } addr[2];                    /* addresses for command */
  union {
    uchar *lhs;                 /* s command lhs, or text argument */
    struct cmd_t *link;         /* branch address */
  } u;
  uchar command;                /* command code */
  uchar flags;
  uchar *rhs;                   /* s command replacement string */
  FILE *fout;                   /* associated output file descriptor */
  int nthone;                   /* if !0 only nth replace */
  struct cmd_t *next;
} sedcmd;                       /* use this name for declarations */

#define FLAG_L1   1     /* first address is a line number or $ */
#define FLAG_L2   2     /* second address is a line number or $ */
#define FLAG_RE1  4     /* first address is a RE */
#define FLAG_RE2  8     /* second address is a RE */
#define FLAG_IN  0x10   /* in the middle of an address range? */
#define FLAG_G   0x20   /* was g postfix specified? */
#define FLAG_P   0x40   /* was p postfix specified? */
#define FLAG_NOT 0x80   /* was negation specified? */

/* address and regular expression compiled-form markers */
#define STAR    1               /* '*', ORed into single RE marker */
#define CCHR    2               /* given character to be matched follows */
#define CNCHR   4               /* [^x] for single char 'x' */
#define CDOT    6               /* dot (any character) marker */
#define CCL     8               /* character class follows */
#define CBEG    10              /* match line beginning */
#define CEND    12              /* match line end */
#define CBRA    14              /* \( tagged pattern start marker */
#define CKET    16              /* \) tagged pattern end marker */
#define CMKET   18              /* \)* multiple tagged pattern end marker */
#define CBACK   20              /* backslash-digit pair marker */
#define CEOF    22              /* end-of-field mark end of RE */
#define CBOW    24              /* begining of word test */
#define CEOW    26              /* end of word test */
#define MTYPE   32              /* \{...\}, ORed into single RE marker */

/* Regexp types (the first byte of compiled regexp) */
#define AREGE   0               /* a non-anchored regexp /.../ */
#define AANCH   1               /* an anchored regexp /^.../ */
#define ALAST   2               /* //, use last used RE */

/*
 * utilities
 */

void fatal(const char *fmt, ...);
void *xmalloc(int n);
uchar *xstrdup(const uchar *s);
uchar *xdup(const uchar *s, const uchar *end);

/*
 * buffers
 *
 * sed uses three buffers (size in brackets):
 * - linebuf (linesz): script lines, pattern space
 * - genbuf (gensz): temporary working buffer
 * - holdbuf (holdsz): hold buffer
 */

extern uchar *linebuf, *genbuf, *holdbuf;
extern int linesz, gensz, holdsz;

void init_buffers(void);
uchar * grow(uchar *q, uchar **buf, int *sz, int amount);

#define LINEGROW(p,n) \
  ((((p)-linebuf)+(n)>=linesz)?(p)=grow((p),&linebuf,&linesz,(n)):(p))
#define GENGROW(p,n) \
  ((((p)-genbuf)+(n)>=gensz)?(p)=grow((p),&genbuf,&gensz,(n)):(p))
#define HOLDGROW(p,n) \
  ((((p)-holdbuf)+(n)>=holdsz)?(p)=grow((p),&holdbuf,&holdsz,(n)):(p))


/*
 * shared data
 */
 
 
/* main data areas */
extern sedcmd *cmds;            /* the list of compiled commands */

/* miscellaneous shared variables */
extern int nflag;               /* -n option flag */
extern int eargc;               /* scratch copy of argument count */
extern sedcmd *pending;         /* ptr to command waiting to be executed */
extern uchar bits[];            /* the bits table */

#if DEBUG
extern int debug;
void dbg(const char *fmt, ...);
#define D(a) if(debug) dbg a
#define DD(a) if(debug >= 2) dbg a
#else
#define D(a)
#define DD(a)
#endif

/*
 * function prototypes
 */

void execute(char *file); 

#endif /* SED_H */

/* sed.h ends here */
