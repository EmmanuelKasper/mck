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
 
/* sedexec.c -- execute compiled form of stream editor commands 
 *
 * The single entry point of this module is the function execute().  It may 
 * take a string argument (the name of a file to be used as text) or the 
 * argument NULL which tells it to filter standard input.  It executes the 
 * compiled commands in cmds on each line in turn.  The function command() 
 * does most of the work. Match() and advance() are used for matching text 
 * against precompiled regular expressions and dosub() does right-hand-side
 * substitution.  Getline() does text input; readout() performs output needed
 * by r and a\ commands.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>

#include "sed.h"

/*
 * private prototypes
 */

static int getline(register uchar *buf);
static int selected(sedcmd * ipc);
static void command(sedcmd * ipc);
static void readout(void);
static int match(uchar *expbuf, int gf);
static int advance(register uchar *lp, register uchar *ep);
static void dosub(uchar *rhsbuf);
static void listto(register uchar *p1, FILE * fp);
static int substitute(sedcmd * ipc);


/*
 * private defines
 */

#define TRUE                    1
#define FALSE                   0
#define ISWCHAR(c) (isalnum(c)||c=='_')
#define SAME(a, b, count) (!memcmp((a), (b), (count)))

static long lnum = 0L;          /* current source line number */

/* append buffer maintenance */

struct append {
  char *p;
  int isfname;
  struct append *next;
} *appends = NULL, *appfreelist = NULL;

static void append(uchar *p, int isfname);


/* genbuf and its pointers and misc pointers*/
static uchar *gp;               /* used as stack pointer for \(...\)* */

/* buffer lengths */
static int linelen=0, holdlen=0;

/* command-logic flags */
static int lastline;            /* last line flag */
static int jump;                /* jump set */
static int delete = FALSE;      /* delete command flag */
static int cdswitch = FALSE;    /* in midst of D command */

/* tagged-pattern tracking */
static int bracend[MAXTAGS + 1];      /* tagged pattern start pointers */
static int brastart[MAXTAGS + 1];     /* tagged pattern end pointers */

static uchar *lastre = NULL;    /* last RE pointer */
static int lasttags;            /* number of \(...\) in the last RE */


/* execute the compiled commands in cmds[] on a file 
 * file - name of text source file to be edited 
 */
void execute(char *file)
{                               
  register sedcmd *ipc;         /* ptr to current command */
  if (file && !freopen(file, "rt", stdin))
    fatal("cannot open %s: %s", file, strerror(errno));
  /* here's the main command-execution loop */
  while (pending || cdswitch || getline(linebuf)) {   /* v1.5 */
    ipc = pending ? pending : cmds;
    delete = FALSE;
    cdswitch = FALSE;
    while (!delete && ipc != NULL && ipc->command) {
      if (pending || selected(ipc)) {
        command(ipc);
      }
      if (jump) {
        jump = FALSE;
        ipc = ipc->u.link;
      } else {
        ipc = ipc->next;
      }
    }
    if (pending)
      break;
    /* we've now done all modification commands on the line */
    if (!nflag && !delete)
      puts((char *)linebuf);
    /* if we've been set up for append, emit the text from it */
    if (appends != NULL)
      readout();
  }
}

/* is current command selected */
static int selected(sedcmd *ipc)
{                               
  int ans = 1;
  int f = ipc->flags;

  if (f & FLAG_IN) {
    if (f & FLAG_L2) {
      if (ipc->addr[1].num != -1 && lnum >= ipc->addr[1].num) {
        ipc->flags &= ~FLAG_IN;
      }
    } else {
      if (match(ipc->addr[1].re, 0)) {
        ipc->flags &= ~FLAG_IN;
      }
    }
  } else if (f & (FLAG_L1 | FLAG_RE1)) {
    if (f & FLAG_L1) {
      if (ipc->addr[0].num == -1) {
        return (f & FLAG_NOT) ? !lastline : lastline;
      } 
      ans = (lnum == ipc->addr[0].num);
    } else {
      ans = match(ipc->addr[0].re, 0);
    }
    if (ans && 
      ((f & FLAG_RE2) || 
        ((f & FLAG_L2) && 
          ! (ipc->addr[1].num != -1 && lnum >= ipc->addr[1].num)
      ))) {
      ipc->flags |= FLAG_IN;
    }
  } 
  return (f & FLAG_NOT) ? !ans : ans;
}


uchar *lastloc;

/* match RE at ep against linebuf; if gf set, copy linebuf from genbuf 
 * expbuf - 
 * gf - set on recall 
 */
static int match(uchar *ep, int gf)
{                               
  uchar c;
  uchar *p1 = linebuf;
  
  if (gf) p1 += bracend[0];
  gp = genbuf;
  if (*ep == ALAST) {
    if (lastre == NULL) {
      fatal("%s: no last RE", ep+1);
    }
    lastloc = ep+1;
    ep = lastre;
    lasttags = 0;               /* it will be computed at runtime */
  } else {
    lastre = ep;
    lasttags = 10;              /* already checked at compile time */
  }
  if (*ep++ == AANCH) {         /* it is an anchored RE */
    if (gf) return (FALSE);     /* no repeats on anchored match */
    brastart[0] = 0;
    if (*ep == CCHR && ep[1] != *p1)    /* 1st char is wrong */
      return (FALSE);           /* so fail */
    return (advance(p1, ep));
  }
  /* else try to match rest */
  /* quick check for 1st character if it's literal */
  if (*ep == CCHR) {
    c = ep[1];              /* get search character */
    ep += 2;
    do {
      if (*p1 == c) {
        brastart[0] = p1 - linebuf;
        if (advance(p1 + 1, ep)) {   /* match the rest */
          return 1;
        }
      }
    } while (*p1++);
    return (FALSE);
  }
  /* else try for unanchored match of the pattern */
  do {
    brastart[0] = p1 - linebuf;
    if (advance(p1, ep))
      return 1;
  } while (*p1++);
  /* if got here, didn't match either way */
  return (FALSE);
}

/* attempt to advance match pointer by one pattern element 
 * lp - source
 * ep - RE
 */
static int advance(uchar *lp, uchar *ep)
{                               
  uchar c;                      /* scratch character holder */
  int i1, i2;                   /* scratch integer holders */
  uchar *bbeg;
  int ct; 

DD(("advance(lp = +%d)\n", (int)(lp - linebuf)));

  while ((c = *ep++) != 0) {    /* while the end of RE is not reached */
    
DD(("  c=%d, lp=+%d, *lp='%c'\n", c, (int)(lp - linebuf), *lp));
    i1 = 0;
    i2 = -1;
    if (c & MTYPE) {
      i1 = *ep++; if (i1 & 0x80) { i1 &= 0x7f; i1 <<= 8 ; i1 += *ep++; }
      i2 = *ep++; if (i2 & 0x80) { i2 &= 0x7f; i2 <<= 8 ; i2 += *ep++; }
      c &= ~MTYPE;
      if (i2 == 0) i2--;
    }
    switch (c) {
    case CCHR:                  /* literal character */
      if (*ep++ == *lp++)
        break;                  /* if chars are equal */
      return (FALSE);           /* else return false */
    case CNCHR:
      if (*lp && *lp != *ep++) {
        lp++;
        break;                  /* if chars are different */
      }
      return (FALSE);           /* else return false */
    case CBOW:                  /* at the begining of a word */
      if (ISWCHAR(*lp) && (lp == linebuf || !ISWCHAR(lp[-1])))
        break;                  /* at word start */
      return (FALSE);
    case CEOW:                  /* at the end of a word */
      if (!ISWCHAR(*lp) && lp > linebuf && ISWCHAR(lp[-1]))
        break;
      return (FALSE);
    case CDOT:                  /* anything but eol */
      if (*lp++)
        break;                  /* not first NUL is at EOL */
      return (FALSE);           /* else return false */
    case CEND:                  /* end-of-line */
      if (*lp == 0)
        break;                  /* found that  NUL? */
      return (FALSE);           /* else return false */
    case CCL:                   /* a set */
      c = *lp++;
      if (ep[c >> 3] & bits[c & 07]) {        /* is char in set? */
        ep += 32;               /* then skip rest of bitmask */
        break;
      }                         /* and keep going */
      return (FALSE);           /* else return false */
    case CBRA:                  /* start of tagged pattern */
      c = *ep++;
      if (c > lasttags) lasttags = c;
      brastart[c] = lp - linebuf;         /* mark it */
      break;                    /* and go */
    case CKET:                  /* end of tagged pattern */
      bracend[*ep++] = lp - linebuf;      /* mark it */
      break;                    /* and go */
    case CBRA | STAR:           /* start of tagged pattern */
      { 
        uchar *curgp = gp;
        int offset = *ep++;     /* get the offset to after the KET */
        if(offset > 127) { 
          offset = ((offset & 0x7F) << 8) | *ep++; 
        }
        c = *ep++;              /* the bracket number */
        if (c > lasttags) lasttags = c;
        while (i2 && advance(lp, ep)) {
          /* CKET|STAR sets bracend but does not alter brastart */
          ct = (linebuf + bracend[c]) - lp;
D(("ct=%d, lp=%d\n", ct, lp-linebuf));
          if (ct == 0) {
            /* skip the rest of the match */
            if (i1 > 0) i1 = 0;
            i2 = 0;
            break;
          }
          /* push the length of the sub-expression on the genbuf stack */
          *gp++ = ct;
          if (ct > 127) *gp++ = (ct >> 8) | 0x80; 
          lp += ct; i2--; i1--;
        }
D(("i1=%d, i2=%d, lp=%d\n", i1, i2, lp-linebuf));
        if (i1 > 0) {
          gp = curgp;           /* restore the genbuf stack */
          return FALSE;         /* not enough matched */
        }
        ep += offset;
        while (i1++ < 0) {
          ct = *--gp;
          if (ct > 127) ct = ((ct & 0x7F) << 8) | *--gp;
          bracend[c] = lp - linebuf;
          brastart[c] = (lp - linebuf) - ct;
D(("lp=%d, ct=%d\n", lp - linebuf, ct)); 
          if (advance(lp, ep)) {
            gp = curgp;
            return (TRUE);
          }
          lp -= ct;
        }
        /* zero matches also */
        ct = *--gp;
        if (ct > 127) ct = ((ct & 0x7F) << 8) | *--gp;
        bracend[c] = lp - linebuf;
        brastart[c] = (lp - linebuf) - ct;
D(("lp=%d, ct=%d\n", lp - linebuf, ct)); 
        gp = curgp;
      }
      break;                    /* and go */
    case CMKET:           /* end of tagged pattern */
      bracend[*ep++] = lp - linebuf;      /* mark it */
      return TRUE;
    case CBACK:
      bbeg = linebuf + brastart[*ep];
      ct = (linebuf + bracend[*ep++]) - bbeg;
      if (ct == 0) break;       /* zero length matches */   
      if (SAME(bbeg, lp, ct)) {
        lp += ct;
        break;
      }
      return (FALSE);
    case CBACK | STAR:
      bbeg = linebuf + brastart[*ep];
      ct = (linebuf + bracend[*ep++]) - bbeg;
      if (ct == 0) break;       /* zero length matches */   
      while (SAME(bbeg, lp, ct) && i2) {
        lp += ct; i2--; i1--;
      }
      if (i1 > 0) return FALSE; /* not enough matched */
      while (i1++ < 0) {
        if (advance(lp, ep))
          return (TRUE);
        lp -= ct;
      }
      break;
    case CDOT | STAR:           /* match .* */
      while (*lp && i2) {       /* match anything */
        lp++; i2--; i1--;       
      }
      goto star;                /* now look for followers */
    case CCHR | STAR:           /* match <literal char>* */
      c = *ep++;
      while (*lp == c && i2) {  /* match many of that char */
        lp++; i2--; i1--;       
      }
      goto star;
    case CNCHR | STAR:          /* match [^x]* */
      c = *ep++;
      while (*lp && *lp != c && i2) { 
        lp++; i2--; i1--;       
      }
      goto star;
    case CCL | STAR:            /* match [...]* */
    
#if DEBUG
      if (debug >= 2) {
        int i;
        fprintf(stderr, "[");
        for (i = 0; i < 32 ; i++) {
          fprintf(stderr, "%02x", ep[i]);
        }
        fprintf(stderr, "]\\{%d,%d\\}\n", i1, i2);
      }
#endif
        
        
      while (c = *lp, ep[c >> 3] & bits[c & 07] && i2) { 
        lp++; i2--; i1--;       
      }
      ep += 32;                 /* skip past the set */
      goto star;                /* match followers */
    star:                       /* the recursion part of a * or + match */
      if (i1 > 0) return FALSE; /* not enough matched */

      /* look for the first char of the follower, in some easy cases */
      if (*ep == CCHR) {
        ep++;
        c = *ep++;
        while (i1++ < 0) {
          if (*lp-- == c && advance(lp+2, ep))
            return (TRUE);
        }
        ep -= 2;
        break;
      }
      if (*ep == CBACK && brastart[ep[1]] < bracend[ep[1]]) {
        c = linebuf[brastart[ep[1]]];
        while (i1++ < 0) {
          if (*lp != c) {
            lp--;
            continue;
          }
          if (advance(lp--, ep))
            return (TRUE);
        }
        break;
      }
      while (i1++ < 0) {
        if (advance(lp--, ep))
          return (TRUE);
      }
      break;
    default:
      fatal("internal error: RE bad code %x", *--ep);
    }
  }
  bracend[0] = lp - linebuf;              /* set second loc */
  return (TRUE);                /* wow we matched it */
}                               

/* substitute 
 * ipc - ptr to s command struct to do
 */
static int substitute(sedcmd *ipc)
{
  int lastend;               /* end of previously matched area */ 
  int repcnt;                   /* count sucessful matches */
  int fcnt;                     /* requested match number, or zero */
  int advance = 0;              /* one if stepping after a null match */

  if (!match(ipc->u.lhs, 0)) {  /* if no match, command fails */
    return (FALSE);             
  }
  fcnt = ipc->nthone;
  if (bracend[0] == brastart[0]) {  /* null-lengthed match */
    advance = 1;                /* next match will start one char right */
  }
  if (fcnt <= 1) {
    dosub(ipc->rhs);
    if (! (ipc->flags & FLAG_G)) {
      return 1;
    }
  }
  /* now either fcnt > 1, or it is a global subst */
  repcnt = 1;
  lastend = bracend[0];

  while (lastend + advance <= linelen) { /* cycle through possibles */
    bracend[0] += advance;
    advance = 0;
    if (!match(ipc->u.lhs, 1)) { /* no other found */
      break;
    }
    if (bracend[0] == brastart[0]) {  /* null-lengthed match */
      if (brastart[0] == lastend) { 
        bracend[0]++;           /* advance immediately one char right */
        continue;
      } else {
        advance = 1;            /* next match will start one char right */
      }
    }
    if (!fcnt || ++repcnt == fcnt) {
      dosub(ipc->rhs);
    }
    if (fcnt == repcnt) break;
    lastend = bracend[0];
  }
  return !fcnt || fcnt == repcnt;
} 

/* uses linebuf, genbuf, spend */
/* generate substituted right-hand side (of s command) */
static void dosub(uchar *rhsbuf)
{
  register uchar *lp, *sp, *rp;
  /* anything before location 1 will not be considered. */
  sp = genbuf;
  rp = rhsbuf;
  for (;;) {
    int c = *rp++;
    if (c == 0) {
      c = *rp++;
      if(c == 0) break;
      c -= '0';
      if (c > lasttags) {
        /* this can only happen when using the last RE, since other
         * cases are detected at compile time 
         */
        fatal("%s: no such \\%c", lastloc, c + '0');
      }
      lp = linebuf + brastart[c];
      c = bracend[c] - brastart[c];
      GENGROW(sp, c);
      while (--c >= 0) {
        *sp++ = *lp++;
      }
    } else {
      GENGROW(sp, 1);
      *sp++ = c;
    }
  }
  /* update bracend[0] (needed for the next replace) */
  rp = linebuf + bracend[0];
  bracend[0] = brastart[0] + (sp - genbuf);
  
  lp = linebuf + brastart[0];
  /* if the replaced part is smaller than the previous part */
  if (sp - genbuf <= rp - lp) {
    /* move back the replaced part */
    *sp = 0;
    sp = genbuf;
    while (*sp) {
      *lp++ = *sp++;
    }
    /* and close the gap if any */
    if (lp != rp) {
      while ((*lp++ = *rp++) != 0);   
      linelen = (lp - 1) - linebuf;
    }
  } else {
    /* original implementation, append to work buf */
    GENGROW(sp, linelen - (rp - linebuf));
    while ((*sp++ = *rp++) != 0); 
    /* then copy all back */
    LINEGROW(lp, sp - genbuf);
    sp = genbuf;
    while ((*lp++ = *sp++) != 0); 
    linelen = (lp - 1) - linebuf;
  }
}

/* write a visually unambiguous *p1... to fp */
static void listto(uchar *p1, FILE *fp)
{
  int c;
  const int maxlen = 72;
  int len = 0;
  while ((c = *p1++) != 0) {
    if (isprint(c) && c != '\\') {
      len++;
      if(len >= maxlen) {
        putc('\\', fp);
        putc('\n', fp);
        len = 1;
      }
      putc(c, fp);              /* pass it through */
    } else {
      switch (c) {
      case '\a': c = 'a'; goto escape;
      case '\b': c = 'b'; goto escape;
      case '\f': c = 'f'; goto escape;
      case '\n': c = 'n'; goto escape;
      case '\r': c = 'r'; goto escape;
      case '\t': c = 't'; goto escape;
      case '\v': c = 'v'; goto escape;
      case '\\': 
        escape:
        len += 2;
        if(len >= maxlen) {
          putc('\\', fp);
          putc('\n', fp);
          len = 2;
        }
        putc('\\', fp);
        putc(c, fp);
        break;
      default:
        len += 4;
        if(len >= maxlen) {
          putc('\\', fp);
          putc('\n', fp);
          len = 4;
        }
        fprintf(fp, "\\%03o", c);
      }
    }
  }
  putc('$', fp);
  putc('\n', fp);
}

/* execute compiled command pointed at by ipc */
static void command(sedcmd *ipc)
{
  static int didsub = FALSE;     /* true if last s succeeded */
  register uchar *p1, *p2;       /* temp pointers */

  switch (ipc->command) {
  case ACMD:                    /* append */
    append(ipc->u.lhs, 0);
    break;
  case CCMD:                    /* change pattern space */
    delete = TRUE;
    if (!(ipc->flags & FLAG_IN) || lastline)
      puts((char *)ipc->u.lhs);
    break;
  case DCMD:                    /* delete pattern space */
    delete++;
    break;
  case CDCMD:                   /* delete first line in pattern space */
    p1 = linebuf;
    delete++;
    while (*p1 && *p1 != '\n')
      p1++;
    if (!*p1++)
      return;
    linelen -= (p1 - linebuf);
    memmove(linebuf, p1, linelen+1);
    cdswitch = TRUE;
    break;
  case EQCMD:                   /* show current line number */
    fprintf(stdout, "%ld\n", lnum);
    break;
  case GCMD:                    /* copy hold space to pattern space */
    p1 = linebuf; 
    LINEGROW(p1, holdlen+1); 
    linelen = holdlen;
    memmove(p1, holdbuf, holdlen+1);
    break;
  case CGCMD:                   /* append hold space to pattern space */
    p1 = linebuf + linelen; 
    linelen += holdlen+1;
    LINEGROW(p1, holdlen+1); 
    *p1++ = '\n';
    memmove(p1, holdbuf, holdlen+1);
    break;
  case HCMD:                    /* copy pattern space to hold space */
    p1 = holdbuf; 
    holdlen = linelen;
    HOLDGROW(p1, linelen+1); 
    memmove(p1, linebuf, linelen+1);
    break;
  case CHCMD:                   /* append pattern space to hold space */
    p1 = holdbuf + holdlen; 
    HOLDGROW(p1, linelen+2); 
    holdlen += linelen+1;
    *p1++ = '\n';
    memmove(p1, linebuf, linelen+1);
    break;
  case ICMD:                    /* insert text */
    puts((char *)ipc->u.lhs);
    break;
  case BCMD:                    /* branch to label command */
    jump = TRUE;
    break;
  case LCMD:                    /* list text */
    listto(linebuf, stdout);
    break;
  case NCMD:                    /* read next line into pattern space */
  case CNCMD:                   /* append next line to pattern space */
    if (!pending) {
      if (ipc->command == NCMD) {
        if (!nflag)
          puts((char *)linebuf);
        linelen = 0;
      } else
        linebuf[linelen++] = '\n';
      if (appends != NULL)
        readout();
    }                           /* do any pending a,r */
    linebuf[linelen] = 0;
    if (!(getline(linebuf + linelen))) {
      pending = lastline ? NULL : ipc; 
      delete = TRUE;
    } else {
      pending = NULL;
    }
    break;
  case PCMD:                    /* print pattern space */
    puts((char *)linebuf);
    break;
  case CPCMD:                   /* print one line from pattern space */
    for (p1 = linebuf; *p1 != '\n' && *p1 != '\0';)
      putchar(*p1++);
    putchar('\n');
    break;
  case QCMD:                    /* quit the stream editor */
    if (!nflag)
      puts((char *)linebuf);            /* flush out the current line */
    if (appends != NULL)
      readout();                /* do any pending a and r commands */
    exit(0);
  case RCMD:                    /* read a file into the stream */
    append(ipc->u.lhs, 1);
    break;
  case SCMD:                    /* substitute RE */
  D(("<linelen=%d\n", linelen));
    if (substitute(ipc)) {
      didsub = TRUE;
      if (ipc->fout) {
        fprintf(ipc->fout, "%s\n", linebuf);
      }
      if (ipc->flags & FLAG_P) {
        puts((char *)linebuf);
      } 
    }
  D((" linelen=%d>\n", linelen));
    break;
  case TCMD:                    /* branch on last s successful */
    jump = didsub;
    didsub = FALSE;             /* reset after test */
    break;
  case WCMD:                    /* write pattern space to file */
    fprintf(ipc->fout, "%s\n", linebuf);
    break;
  case XCMD:                    /* exchange pattern and hold spaces */
    { 
      int n;
      p1 = linebuf; linebuf = holdbuf; holdbuf = p1;
      n = linelen; linelen = holdlen; holdlen = n;
      n = linesz; linesz = holdsz; holdsz = n;
    }
    break;
  case YCMD:
    p1 = linebuf;
    p2 = ipc->u.lhs;
    while (*p1) {
      *p1 = p2[*p1];
      p1++;
    }
    break;
  }
}

/* get next line of text to be filtered 
 * buf - where to put line (must be within linebuf)
 * returns 1 if a line was read; sets linelen to the new length of linebuf 
 * and sets lastline = 1 if the last line was read
 */
 
static int getline(uchar *buf)
{
  int c;
  uchar *p = buf;
  D(("eargc = %d\n", eargc));
  for (;;) {
    do { c = getc(stdin); } while (c == 0);
    if (c == EOF) {
      if (eargc <= 1) {
        lastline = TRUE;          /* if no more args this is it */
      }
      *p = 0; 
      linelen = p - linebuf;
      if (p == buf) return 0;
      lnum++; return 1;
    } else if (c == '\n') {
      *p = 0;
      linelen = p - linebuf;
      if (eargc <= 1) {           /* last file, check if at EOF */
        int temp = getc(stdin);
        if (temp == EOF) {
          lastline = 1;
        } else {
          ungetc(temp, stdin);
        }
      }
      lnum++; return 1;
    } else {
      LINEGROW(p, 1);
      *p++ = c;
    }
  }
}


/*
 * appends
 */


static void append(uchar *p, int isfname)
{
  struct append *a;
  if (appfreelist) {
    a = appfreelist; appfreelist = a->next;
  } else {
    a = xmalloc(sizeof *a);
  }
  a->p = (char *)p;
  a->isfname = isfname;
  a->next = appends; appends = a;
}

static void readout(void)
{
  struct append *a, *b;

  if (appends == NULL) return;
  for (a = appends; a != NULL; b = a, a = a->next) {
    if (a->isfname) {
      FILE *f = fopen(a->p, "r");
      if (f == NULL) fatal("cannot open %s: %s", a->p, strerror(errno));
      for (;;) {
        size_t count = fread(genbuf, 1, gensz, f);
        if (count <= 0) break;
        if (fwrite(genbuf, 1, count, stdout) != count) exit(EXIT_FAILURE);
      }
      fclose(f);
    } else {
      puts(a->p);
    }
  }
  b->next = appfreelist; appfreelist = appends; appends = NULL;
}


/* sedexec.c ends here */
