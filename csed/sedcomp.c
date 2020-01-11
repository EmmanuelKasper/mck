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

/*
 * sedcomp.c -- stream editor main and compilation phase
 *
 * The stream editor compiles its command input  (from files or -e options) 
 * into an internal form using compile() then executes the compiled form using
 * execute(). Main() just initializes data structures, interprets command
 * line options, and calls compile() and execute() in appropriate sequence.
 * The data structure produced by compile() is an list of compiled-command
 * structures (type sedcmd). In the special case that the command is a label 
 * the struct will hold a ptr into the labels list labels during most of the 
 * compile, until resolve() resolves references at the end. The operation of 
 * execute() is described in its source module.
 *
 * Memory management: there are three general buffers: linebuf, genbuf, and
 * holdbuf; each buffer can grow when using LINEGROW, GENGROW and HOLDGROW
 * macros. For this reason, some functions designed to add text to one of the
 * 3 general buffer will only work on pointers pointing inside that buffer.
 *
 * === Initially written for the GNU operating system by Eric S. Raymond ===
 */

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
 
#include "sed.h"

#if DEBUG
int debug = 0;
void dbg(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
}
#endif


/* main data areas */
sedcmd *cmds = NULL;            /* hold compiled commands */

/* miscellaneous shared variables */
int nflag;                      /* -n option flag */
sedcmd *pending = NULL;         /* next command to be executed */
uchar bits[] = { 1, 2, 4, 8, 16, 32, 64, 128 };

/***** module common stuff *****/
#define WFILES          10      /* max # w output files that can be compiled */
#define MAXDEPTH        20      /* maximum {}-nesting level */

#define SKIPWS(pc) while((*pc==' ')||(*pc=='\t')||*pc=='\f'||*pc=='\v')pc++

/*
 * labels
 */

typedef struct label {          /* represent a command label */
  char *name;                   /* the label name */
  sedcmd *list;                 /* it's on the label search list */
  sedcmd *address;              /* pointer to the cmd it labels */
  struct label *next;
} label;

/* label handling */
static label *labels = NULL;   /* here's the label table */

/* compilation state */
static uchar *cp;               /* current script character */
static sedcmd *cmdp;            /* current compiled-cmd ptr */
static int bdepth = 0;          /* current {}-nesting level */
static int bcount = 0;          /* # tagged patterns in current RE */

/*
 * private prototypes
 */

static void cmderror(const char *fmt, ...);
static int cmdline(void);
static void usage(int fatal);
static void compile(void);
static void resolve(void);       
static uchar fixquote(uchar **);
static int address(sedcmd *cmdp, int pass);
static int cmdcomp(register uchar cchar);
/* get the argument of a\, c\, i\ */
static uchar * getargtext(void);
static label * search(void);
static uchar * recomp(uchar redelim);
static uchar * rhscomp(uchar delim);
static uchar * ycomp(void);

/*
 * error reporting
 */

void fatal(const char *fmt, ...)
{
  va_list ap;
  fprintf(stderr, "sed: ");
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fprintf(stderr, "\n");
  exit(EXIT_FAILURE);
}


/*
 * memory
 */

void *xmalloc(int n)
{
  char *p = malloc(n);
  if (NULL == p) fatal("out of memory");
#if DEBUG
  /* detect uninitialized memory */
  memset(p, 0xFF, n);
#endif
  return p;
}

uchar *xstrdup(const uchar *s)
{
  int n = strlen((const char *)s)+1;
  uchar *p = xmalloc(n);
  memmove(p, s, n);
  return p;
}

uchar *xdup(const uchar *s, const uchar *t)
{
  int n = t - s;
  uchar *p = xmalloc(n);
  memmove(p, s, n);
  return p;
}

/*
 * buffers
 */

uchar *linebuf, *genbuf, *holdbuf;
int linesz, gensz, holdsz;

#if DEBUG
#define INIT_AMOUNT 10
#else
#define INIT_AMOUNT 1024
#endif
void init_buffers(void)
{
  linebuf = xmalloc(linesz = INIT_AMOUNT);
  genbuf  = xmalloc(gensz  = INIT_AMOUNT);
  holdbuf = xmalloc(holdsz = INIT_AMOUNT);
}

#if DEBUG
#define GROW_AMOUNT 10
#else
#define GROW_AMOUNT 1024
#endif
uchar * grow(uchar *q, uchar **buf, int *sz, int amount) 
{
  int n = (amount < GROW_AMOUNT) ? GROW_AMOUNT : amount;
  uchar *p;
  
#if DEBUG
  if (q < *buf || q > *buf+*sz) { fprintf(stderr, "oops.\n"); exit(EXIT_FAILURE); }
#endif
  
  D(("before grow: \"%s\"\n", *buf));
  
  p = realloc(*buf, *sz + n);
  if (p == NULL && n > amount) {
    /* try with the amount only */
    n = amount;
    p = realloc(*buf, *sz + n);
  }
  if (p == NULL) fatal("no memory: %s", strerror(errno));
  q = (q - *buf) + p;
  *buf = p;
  *sz += n;
 D(("after grow: \"%s\"\n", *buf));
  return q;
}





/*
 * command line processing
 *
 * compile() will ask for script lines using cmdline(); cmdline() 
 * in turn interprets argc/argv to return the next line of script.
 * when the entire script has been returned, cmdline() returns zero.
 * then, succesive arguments are used as input file names.
 */

int eargc;
static char **eargv;

static int cmde;           /* 1 if last line read from a -e expression */
static int cmdenum = 0;    /* number of -e expression */
static int cmdlnum;        /* line number */
static char *cmdfname;     /* -f file name */

/* embedded in commands in spots where weird run-time errors could occur */
static uchar * location(uchar *g)
{
  if (cmde) {
    GENGROW(g, (int) sizeof("-e expression #123456789, line 123456789"));
    sprintf((char *)g, "-e expression #%d, line %d", cmdenum, cmdlnum);
  } else {
    GENGROW(g, (int)(sizeof("file  line 123456789") + strlen(cmdfname)));
    sprintf((char *)g, "file %s line %d", cmdfname, cmdenum);
  }
  return g + strlen((char *)g);
}
  
static void cmderror(const char *fmt, ...)
{
  va_list ap;
  if (cmde) {
    fprintf(stderr, "sed: -e expression #%d, ", cmdenum);
  } else {
    fprintf(stderr, "sed: file %s ", cmdfname);
  }
  fprintf(stderr, "line %d: ", cmdlnum);
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  fprintf(stderr, "\n");
  exit(EXIT_FAILURE);
}

static int cmdline(void)
{
  static FILE * cmdf = NULL;   /* NULL if not in -f */
  static char * cmdl = NULL;   /* NULL if not in -e */
  static int ef_seen = 0;   /* 1 if any -e or -f argument was seen */
  static int done = 0;    /* 1 when done (after the current cmdf or cmdl) */
  uchar *p = linebuf;
  int c;
      
doit:
  cmdlnum++;
  if (NULL != cmdf) {
    for(;;) {
      do { c=getc(cmdf); } while(c == 0);
      if (c == EOF) {
        fclose(cmdf);
        cmdf = NULL;
        if (p == linebuf) break;
        *p = 0;
        return 1;
      } else if (c == '\n') {
        *p = 0;
        return 1;
      } else {
        LINEGROW(p,1);
        *p++ = c;
      }
    }
  } else if (NULL != cmdl) {
    for (;;) {
      c = *cmdl++;
      if (c == 0) {
        *p = 0;
        cmdl = NULL;
        return 1;
      } else if (c == '\n') {
        *p = 0;
        return 1;
      } else {
        LINEGROW(p,1);
        *p++ = c;
      }
    }
  } 
    
  if (done) return 0;
  
  /* process next argument */
  while (--eargc > 0) {
    char *a = *++eargv;
    if (*a == '-') {
      switch(a[1]) {
      case 'e':
        if (a[2]) {
          cmdl = a+2;
        } else if (--eargc > 0) {
          cmdl = *++eargv;
        } else {
          usage(1);
        }
        ef_seen = 1;
        cmde = 1; cmdenum++; cmdlnum=0;
        goto doit;
      case 'f':
        if (a[2]) {
          a = a+2;
        } else if (--eargc > 0) {
          a = *++eargv;
        } else {
          usage(1);
        }
        cmdf = fopen(a, "r");
        if (cmdf == NULL) 
          fatal("cannot open %s: %s", a, strerror(errno));
        cmde = 0; cmdfname = a; cmdlnum=0;
        ef_seen = 1;
        goto doit;
      case 'n':
        nflag++;
        if (a[2]) usage(1);
        break;
#if DEBUG
      case 'd':
        debug++;
        if (a[2]) usage(1);
        break;
#endif
      default:
        usage(1);
      }
    } else {
      done = 1;
      if (ef_seen) {
        eargc++;
        eargv--;
        return 0;
      } else {
        cmdl = a;
        cmde = 1; cmdenum = 1; cmdlnum=0;
        goto doit;
      }
    }
  }
  done = 1;
  return 0;
}


/*
 * usage
 */

static void usage(int fatal)
{
  fprintf(stderr, 
"usage: sed [<option>]... [<script>] [<file>]...\n"
"Sed runs commands contained in the script on input (consisting of the\n"
"concatenation of each <file>), sending output to stdout; if no <file>\n"
"name is given, input is taken from stdin.\n"
"options:\n"
"  -n         do not print by default\n"
#if DEBUG
"  -d         increase debug verbosity\n"
#endif
"  -e <text>  append the text to the script\n"
"  -f <file>  append the contents of the file to the script\n"
"The <script> argument is only present if no -e or -f options are used.\n"
  );
  exit(fatal ? EXIT_FAILURE : 0);
}


int main(int argc, char *argv[])
{                               
  eargc = argc;                 /* set local copy of argument count */
  eargv = argv;                 /* set local copy of argument list */
  if (eargc <= 1) {
    usage(1);
  }
  init_buffers();
  
  compile();
  
  if (bdepth) {                 /* we have unbalanced squigglies */
    fatal("too many {'s");            
  }
  resolve();                    /* resolve label table indirections */  
  holdbuf[0] = 0;               /* clear initial hold buffer */
  if (eargc <= 0) {
    execute((char *) NULL);     /* execute on file from stdin only */
  } else {
    while (--eargc >= 0) {      /* else execute only listed files */
      execute(*++eargv);
    }
  }
  return (0);
}                               /* everything was O.K. if we got here */

#define H       0x80            /* on if there's really code for  command */
#define LOWCMD  '8'             /* lowest char indexed in cmdmask */
#define HIGHCMD '~'             /* highest char indexed in cmdmask */

/* indirect through this to get command internal code, if it exists */
/* *INDENT-OFF* */
static unsigned char cmdmask[] = { 
  /*  8        9        :        ;        <        =        >        ? */
      0,       0,       H,       0,       0, H+EQCMD,       0,       0,      
  /*  @        A        B        C        D        E        F        G */ 
      0,       0,       0,       0,   CDCMD,       0,       0,   CGCMD,    
  /*  H        I        J        K        L        M        N        O */   
  CHCMD,       0,       0,       0,       0,       0,   CNCMD,       0, 
  /*  P        Q        R        S        T        U        V        W */      
  CPCMD,       0,       0,       0,       0,       0,       0,       0,   
  /*  X        Y        Z        [        \        ]        ^        _ */
      0,       0,       0,       0,       0,       0,       0,       0,       
  /*  `        a        b        c        d        e        f        g */
      0,  H+ACMD,  H+BCMD,  H+CCMD,    DCMD,       0,       0,    GCMD, 
  /*  h        i        j        k        l        m        n        o */   
   HCMD,  H+ICMD,       0,       0,    LCMD,       0,    NCMD,       0,   
  /*  p        q        r        s        t        u        v        w */
   PCMD,  H+QCMD,  H+RCMD,  H+SCMD,  H+TCMD,       0,       0,  H+WCMD, 
  /*  x        y        z        {        |        }        ~  */
   XCMD,  H+YCMD,       0,  H+BCMD,       0,       H,       0, 
};                              
/* *INDENT-ON* */

/* precompile sed commands
 */
static void compile(void)
{
  uchar ccode = 0;
  int first = 1;
  sedcmd *cmdlast = NULL;
  
  cmdp = cmds = xmalloc(sizeof(*cmds));

  while (cmdline()) {
    cp = linebuf;
D(("cp=\"%s\"\n", cp));
    if (first && *cp == '#') {
      first = 0;
      cp++;
#if DEBUG
      while (*cp == 'd') { cp++; debug++; }
#endif
      if (*cp == 'n') nflag++;
      continue;
    }
    first = 0;
    do {
      if (*cp == ';') cp++; 
      SKIPWS(cp);
      if (*cp == '#' || *cp == 0) break;
      
      cmdp->flags = 0;          /* initialize new command */
      
      if (address(cmdp, 0)) {     /* compile first address */
        SKIPWS(cp);
        if (*cp == ',') {         /* there's 2nd addr */
          cp++;
          if (!address(cmdp, 1))
            cmderror("garbled address");
        }
      }
      SKIPWS(cp);                 /* discard whitespace after address */
      if (*cp == '!') { 
        while(*cp == '!') cp++;
        cmdp->flags |= FLAG_NOT;
      }
      SKIPWS(cp);                 /* get cmd char, range-check it */
      if ((*cp < LOWCMD) || (*cp > HIGHCMD)
          || ((ccode = cmdmask[*cp - LOWCMD]) == 0)) {
        cmderror("unknown command `%c'", *cp);
      }
      cmdp->command = ccode & ~H; /* fill in command value */
      if (!(ccode & H)) {
        cmdp->next = xmalloc(sizeof(sedcmd));
        cmdlast = cmdp; cmdp = cmdp->next;
        cp++;
      } else if (cmdcomp(*cp++)) { 
        cmdp->next = xmalloc(sizeof(sedcmd));
        cmdlast = cmdp; cmdp = cmdp->next;
      }
      SKIPWS(cp);                 /* look for trailing stuff */
    } while (*cp == ';');
    if (*cp != 0 && *cp != '#') {
      cmderror("extra characters after command\n");
    }
  }
  
  /* get rid of last command */
  if (cmdlast == NULL) {
    cmds = NULL;
  } else {
    cmdlast->next = NULL;
  }
  /* keep last command in case there are { pointing to it */
  cmdp->next = NULL;
  cmdp->flags = 0;
  cmdp->command = NOCMD;
}

/* compile a single command 
 * cchar - character name of command 
 */
static int cmdcomp(register uchar cchar)       
{
  static sedcmd **cmpstk[MAXDEPTH];     /* current cmd stack for {} */
  static char *fname[WFILES];   /* w file name pointers */
  static FILE *fout[WFILES];    /* w file file ptrs */
  static int nwfiles = 0;       /* count of open w files */
  int i;                        /* indexing dummy used in w */
  label *lpt;
  uchar redelim;                 /* current RE delimiter */

  switch (cchar) {
  case '{':                     /* start command group */
    cmdp->flags ^= FLAG_NOT;
    cmpstk[bdepth++] = &(cmdp->u.link);
    cmdp->next = xmalloc(sizeof *cmdp);
    cmdp = cmdp->next;
    if (*cp != '\0')
      *--cp = ';';              /* get next cmd w/o lineread */
    return (0);
  case '}':                     /* end command group */
    if (cmdp->flags & (FLAG_L1 | FLAG_RE1 | FLAG_NOT))
      cmderror("no addresses allowed for `%c'", cchar);             /* no addresses allowed */
    if (--bdepth < 0) 
      cmderror("too many }'s");             /* too many right braces */
    *cmpstk[bdepth] = cmdp;     /* set the jump address */
    return (0);
  case '=':                     /* print current source line number */
  case 'q':                     /* exit the stream editor */
    if (cmdp->flags & (FLAG_L2 | FLAG_RE2))
      cmderror("only one address allowed for `%c'", cchar);
    break;
  case ':':                     /* label declaration */
    if (cmdp->flags & (FLAG_L1 | FLAG_RE1))
      cmderror("no addresses allowed for `%c'", cchar);
    if (NULL == (lpt = search())) 
      cmderror("missing label name");
    if (lpt->address)
      cmderror("duplicate label");
    lpt->address = cmdp;        /* let it point to the (next) current command */
    return (0);
  case 'b':                     /* branch command */
  case 't':                     /* branch-on-succeed command */
    if (NULL == (lpt = search())) {
      cmdp->u.link = NULL;
    } else {
      cmdp->u.link = lpt->list;
      lpt->list = cmdp;
    }
    break;
  case 'r':                     /* read file into stream */
    if (cmdp->flags & (FLAG_L2 | FLAG_RE2))
      cmderror("only one address allowed for `%c'", cchar);
    SKIPWS(cp);
    cmdp->u.lhs = xstrdup(cp); *cp=0;
    break;
  case 'a':                     /* append text */
  case 'i':                     /* insert text */
    if (cmdp->flags & (FLAG_L2 | FLAG_RE2))
      cmderror("only one address allowed for `%c'", cchar);
  case 'c':                     /* change text */
    if ((*cp != '\\') || (cp[1] != 0)) {
      cmderror("`%c' not followed by \\ newline", cchar);
    }
    if (!cmdline()) {
      cmderror("missing argument to `%c\\'", cchar);
    }
    cmdp->u.lhs = getargtext();
    break;
  case 's':                     /* substitute regular expression */
    redelim = *cp++;            /* get delimiter from 1st ch */
    if (NULL == (cmdp->u.lhs = recomp(redelim)))
      cmderror("garbled command");
    if (NULL == (cmdp->rhs = rhscomp(redelim)))
      cmderror("garbled command");
    cmdp->nthone = 0;
    while (*cp) {
      SKIPWS(cp);
      if (*cp >= '0' && *cp <= '9') {
        if (cmdp->flags & FLAG_G || cmdp->nthone) {
          cmderror("More than one N or g flags");
        }
        i = 0;
        while (*cp >= '0' && *cp <= '9') {
          i = i * 10 + *cp++ - '0';
        }
        if (!i) {
          cmderror("bad N flag value");
        }
        cmdp->nthone = i;
      } else if (*cp == 'g') {
        cp++;
        cmdp->flags |= FLAG_G;
        if (cmdp->nthone) {
          cmderror("More than one N or g flags");
        }
      } else if (*cp == 'p') {
        cp++;
        cmdp->flags |= FLAG_P;
      } else {
        break;
      }
    }
    if (*cp == 'w') {
      cp++;                     /* and execute a w command! */
    } else {
      cmdp->fout = NULL;
      break;
    }
  case 'w':                     /* write-pattern-space command */
    SKIPWS(cp);
    if (!*cp) 
      cmderror("missing filename");
    /* look for the file name in table */
    for (i = nwfiles - 1; i >= 0; i--) {
      if (strcmp((char *)cp, fname[i]) == 0) {
        cmdp->fout = fout[i];
        return (1);
      }
    }
    /* if didn't find one, add it to table and open new out file */
    /* TODO, dynamic */
    if (nwfiles >= WFILES)
      cmderror("too many w files\n");
    fname[nwfiles] = (char *) xstrdup(cp); *cp=0;
    if (NULL == (cmdp->fout = fopen(fname[nwfiles], "wt")))   
      cmderror("cannot create %s: %s", fname[nwfiles], strerror(errno));
    fout[nwfiles++] = cmdp->fout;
    break;
  case 'y':                     /* transliterate text */
    if (NULL == (cmdp->u.lhs = ycomp()))
      cmderror("garbled command");
    break;
  }                             /* switch */
  return 1;
}                               /* succeeded in interpreting one command */


static uchar fixquote(uchar **pp)
{
  uchar *p = *pp;
  uchar c = *p++;

  switch (c) {
  case 'a': c = '\a'; break;
  case 'b': c = '\b'; break;
  case 'e': c = 27; break;
  case 'f': c = '\f'; break;
  case 'n': c = '\n'; break;
  case 'r': c = '\r'; break;
  case 't': c = '\t'; break;
  case 'v': c = '\v'; break;
  case 'x':
    {
      uchar a = *p++;
      if (a >= '0' && a <= '9') { c = (a-'0')<<4; }
      else if (a >= 'a' && a <= 'f') { c = (a-'a'+10)<<4; }
      else if (a >= 'A' && a <= 'F') { c = (a-'A'+10)<<4; }
      else {
        fprintf(stderr, "bad quote \\x%c%c\n", a, *p);
        exit(1);
      }
      a = *p++;
      if (a >= '0' && a <= '9') { c += a-'0'; }
      else if (a >= 'a' && a <= 'f') { c += a-'a'+10; }
      else if (a >= 'A' && a <= 'F') { c += a-'A'+10; }
      else {
        fprintf(stderr, "bad quote \\x%c%c\n", p[-2], a);
        exit(1);
      }
    }
    break;
  }
  *pp = p;
  return c;
}
                

/* uses bcount */
/* generate replacement string for substitute command right hand side 
 * uchar **rhsp - place to compile expression to 
 * uchar delim - regular-expression end-mark to look for 
 */
static uchar * rhscomp(uchar delim)                                  
{
  uchar *fp = genbuf;
  for (;;) {
    uchar c = *cp++;
    if (c == delim) break;
    if (c == '\\') {
      c = *cp++;
      if (c >= '1' && c <= '9') {
        if (c > bcount + '0') {
          return NULL;
        } else {
          /* encode & as 0,'0'; \1 as 0,'1'; EOL is 0,0. */
          GENGROW(fp, 2);
          *fp++ = 0;
          *fp++ = c;
          continue;
        }
      } else if (c == 0) {
        if (!cmdline()) {
          return NULL;
        }
        cp = linebuf;
        c = '\n';
      } else {
        cp--;
        c = fixquote(&cp);
      }
    } else if (c == '&') {
      GENGROW(fp, 2);
      *fp++ = 0;
      *fp++ = '0';
      continue;
    } else if (c == 0) {
      return NULL;
    } 
    GENGROW(fp, 1);
    *fp++ = c;
  }
  GENGROW(fp, 2);
  *fp++ = '\0';
  *fp++ = '\0';                 /* cap the expression string */
  return xdup(genbuf, fp);
}



/* uses cp, bcount */
/* compile a regular expression to internal form 
 * redelim - RE end-marker to look for
 */
static uchar * recomp(uchar redelim)
{
  int lastep = 0;               /* index of last ep for repeat handling */
  register int c;               /* current-character pointer */
  char negclass;                /* all-but flag */
  char brnest[MAXTAGS];         /* bracket-nesting array */
  char *brnestp;                /* ptr to current bracket-nest */
  int i1 = 0, i2 = 0;           /* temps for ranges and multiples */
  uchar *a, *b;
  uchar tmp[10];
  int brafp[MAXTAGS];           /* the value of fp at the nth \( */
  uchar validbra[MAXTAGS];      /* one if \(n+1) is valid for 0<=n<=8 */
  uchar *fp = genbuf;
  
  for (i1 = 0; i1 < MAXTAGS; i1++) {
    validbra[i1] = 0;
  }
  
  if (*cp == redelim) {         /* if first char is RE endmarker */
    cp++;
    GENGROW(fp, 1);
    *fp++ = ALAST;              /* compile as last RE */
    bcount = 9;                   /* allow s//\1/ -  will test at runtime */
    fp = location(fp);          /* add location for any error report */
    return xdup(genbuf, fp);
  }
  brnestp = brnest;             /* initialize ptr to brnest array */
  bcount = 0;            /* initialize counters */

  GENGROW(fp, 1);
  if(*cp == '^') {
    *fp++ = AANCH;              /* anchored RE */
    cp++;
  } else {
    *fp++ = AREGE;              /* non-anchored RE */
  }
  
  while ((c = *cp++) != redelim) {
    switch (c) {
    case '\\':
      c = *cp++;
      if (c == '(') { /* start tagged section */
        if (bcount >= MAXTAGS) {
          return NULL;
        }
        lastep = 0;
        brafp[bcount] = fp - genbuf;
        *brnestp++ = bcount;    /* update tag stack */
        GENGROW(fp, 2);
        *fp++ = CBRA;           /* enter tag-start */
        *fp++ = bcount++ + 1;   /* bump tag count */
      } else if (c == ')') {    /* end tagged section */
        if (brnestp <= brnest)
          return NULL;             /* extra \) */
        lastep = fp - genbuf;
        GENGROW(fp, 2);
        *fp++ = CKET;           /* enter end-of-tag */
        c = *--brnestp;         /* pop tag stack */
        *fp++ = c + 1; 
        validbra[c] = 1;
      } else if (c == '{') {
        if (!lastep) goto defchar;
        /* read first number */
        i1 = 0;
        while (isdigit(*cp)) {
          i1 = i1 * 10 + *cp++ - '0';
          if (i1 > 0x7FFF) return 0;
        }
        /* read next */
        if (*cp == '\\' && cp[1] == '}') {
          cp += 2;
          if (i1 == 0) return 0;
          i2 = i1;
        } else if (*cp++ == ',') {
          if (*cp == '\\' && cp[1] == '}') {
            cp += 2;
            i2 = 0;
          } else {
            i2 = 0;
            while (isdigit(*cp)) {
              i2 = i2 * 10 + *cp++ - '0';
              if (i2 > 0x7FFF) {
                return 0;
              }
            }
            if (*cp != '\\' || cp[1] != '}' || i2 < i1 || i2 == 0) {
              return 0;
            }
            cp += 2;
          }
        }
        multiple:  
        a = tmp;
        /* use c to hold the mask ORed to *lastep */
        if (i1 || i2) {
          if (i1 <= 127) *a++ = i1; 
          else { *a++ = (i1>>8) | 0x80; *a++ = i1; }
          if (i2 <= 127) *a++ = i2; 
          else { *a++ = (i2>>8) | 0x80; *a++ = i2; }
          c = MTYPE | STAR;
        } else {
          c = STAR;
        }
        if (genbuf[lastep] == CKET) {
          genbuf[lastep] = CMKET;      /* turn the CKET to a CMKET */
          i1 = genbuf[lastep+1] - 1;   /* get the number of opening bracket */
          i2 = lastep - brafp[i1];  /* offset between \( and \) */
          if (i2 > 0x7FFF) return 0; 
          if (i2 <= 127) *a++ = i2; 
          else { *a++ = (i2>>8) | 0x80; *a++ = i2; }
          lastep = brafp[i1];   /* the CBRA will get the numbers */
        }
        genbuf[lastep] |= c;
        i1 = a - tmp;
        GENGROW(fp, i1);
        a = fp;
        b = fp += i1;
        while (a > genbuf + lastep) *--b = *--a;
        a++;
#if DEBUG
        if (debug >= 3 ) {
          fprintf(stderr, "tmp (i1=%d) = ", i1);
          for (b = tmp; b < tmp + i1 ; b++) { 
            fprintf(stderr, "%02x ", *b); 
          }
          fprintf(stderr, "\n");
        }
#endif
        b = tmp;
        while (i1--) *a++ = *b++;
        lastep = 0;
      } else if (c == '+') {    /* 1 to n repeats of previous pattern */
        if (!lastep) goto defchar;
        i1 = 1;
        i2 = 0;
        goto multiple;
      } else if (c == '?') {    /* 0 to 1 repeats of previous pattern */
        if (!lastep) goto defchar;
        i1 = 0;
        i2 = 1;
        goto multiple;
      } else if (c == '<') {    /* begining of word test - TODO, remove */
        lastep = 0;
        GENGROW(fp, 1);
        *fp++ = CBOW;
      } else if (c == '>') {    /* end of word test - TODO, remove */
        lastep = 0;
        GENGROW(fp, 1);
        *fp++ = CEOW;
      } else if (c >= '1' && c <= '9') {        /* tag use */
        c -= '1';               /* bracket number */
        if (!validbra[c])
          return 0;             /* too few */
        lastep = fp - genbuf;
        GENGROW(fp, 2);
        *fp++ = CBACK;          /* enter tag mark */
        *fp++ = c + 1;          /* and the number */
      } else if (c == '\n') {
        return 0;               /* escaped newline bad */
      } else {
        cp--;
        c = fixquote(&cp);
        goto defchar;
      }                         
      break;
    case '\0':                  /* do not allow */
    case '\n':
      return 0;                 /* no trailing pattern delimiter */
    case '.':                   /* match any char */
      lastep = fp - genbuf;
      GENGROW(fp, 1);
      *fp++ = CDOT;
      break;
    case '*':                   /* 0..n repeats of previous pattern */
      if (!lastep)
        goto defchar;
      i1 = i2 = 0;
      goto multiple;
    case '$':                   /* match only end-of-line */
      if (*cp != redelim)       /* if we're not at end of RE */
        goto defchar;           /* match a literal $ */
      GENGROW(fp, 1);
      *fp++ = CEND;             /* insert end-symbol mark */
      break;
    case '[':                   /* begin character set pattern */
      lastep = fp - genbuf;
      if ((negclass = ((c = *cp++) == '^')) != 0) {
        c = *cp++;
      }
      GENGROW(fp, 33);
      *fp++ = CCL;              /* insert class mark */
      memset(fp, 0, 32);
      /* i1 = last single char, or 0 if after a range, or -1 if beginning
       * i2 = 0, or 1 if last char is a '-' for a range expression 
       */
      i1 = -1; i2 = 0;
      do {
        if (c == '\0')
          return 0;
        /* handle [.x.], [=x=] and [:alpha:] */
        if (c == '[') {
          if (*cp == '=') {
            if (!(c = cp[1]) || cp[2] != *cp || cp[3] != ']' || i2) 
              return 0;
            fp[(c >> 3) & 0x1F] |= bits[c & 7];
            i1 = 0;
            cp += 4;
            continue;
          } else if (*cp == '.') {
            if (!(c = cp[1]) || cp[2] != *cp || cp[3] != ']') 
              return 0;
            cp += 4;
          } else if (*cp == ':') {
            if (i2) return 0;
            do {

/* I'm not sure isblank() is ANSI C, so just in case */
#define myisblank(c) ((c) == ' ' || (c) == '\t')
             
#define T(name, func) \
  if (!strncmp((char *)cp+1, #name, i2 = sizeof(#name)-1)) { \
    for (i1=1; i1<256; i1++) { \
      if (func((unsigned int)i1)) \
        fp[i1 >> 3] |= bits[i1 & 7]; \
    } \
    break; \
  }
              T(alnum, isalnum)
              T(alpha, isalpha)
              T(blank, myisblank)
              T(cntrl, iscntrl)
              T(digit, isdigit)
              T(graph, isgraph)
              T(lower, islower)
              T(print, isprint)
              T(punct, ispunct)
              T(space, isspace)
              T(upper, isupper)
              T(xdigit, isxdigit)
#undef T
              return 0;
            } while(0);
            if (cp[i2+1] != ':' || cp[i2+2] != ']') return 0;
            cp += i2+3;
            i1 = i2 = 0;
            continue;
          } else if (*cp != ']') {
            return 0;
          }
        } else if (c == '\\') {
          uchar *t = cp;
          c = fixquote(&cp);
          /* but ignore it if not a correct sequence */
          if (c == *t) {
            c = '\\'; cp = t;
          }
        } else if (c == '-' && !i2) {
          c = 0; /* ensures [.-.] doesn't trigger range */
        }
        /* handle character ranges */
        if (i2) {
          i2 = c;
          if (i1 > i2)
            return 0;
          for (; i1 <= i2; i1++)
            fp[i1 >> 3] |= bits[i1 & 7];
          i1 = i2 = 0;
          continue;
        }
        if (c == 0) {
          if (!i1 && *cp != ']') return 0;
          if (i1 != -1) { i2 = 1; continue; }
          c = '-';
        }
        
        fp[(c >> 3) & 0x1F] |= bits[c & 7];
        i1 = c;
      } while ((c = *cp++) != ']');
      if (i2) {
        c = '-';
        fp[(c >> 3) & 0x1F] |= bits[c & 7];
      }
#if DEBUG
      if (debug) {
        int i;
        fprintf(stderr, "[");
        for(i = 1; i < 256; i++) {
          if (fp[i>>3] & bits[i&7]) {
            if (i == '\\') fprintf(stderr, "\\\\");
            else if (isprint(i)) fprintf(stderr, "%c", i);
            else fprintf(stderr, "\\%03o", i);
          }
        }
        fprintf(stderr, "]\n");
      }
#endif
      /* optimize if only one character matches */
      for (c = 0, i1 = 0; i1 < 32; i1++) {
        if (fp[i1] == 0) continue;
        for (i2 = 0; i2 < 8; i2++) {
          if (fp[i1] == bits[i2]) {
            if (!c) { c = (i1 << 3) + i2; break; }
            c = 0; i1=32; break; 
          }
        }
        if (i2 == 8) { c = 0; break; }
      }
      if (c) {
        fp[-1] = negclass ? CNCHR : CCHR;
        *fp++ = c;
        break;
      }
      /* invert the bitmask if all-but was specified */
      if (negclass)
        for (i1 = 0; i1 < 32; i1++)
          fp[i1] ^= 0xFF;
      fp[0] &= 0xFE;            /* never match ASCII 0 */
      fp += 32;                 /* advance ep past set mask */
      break;
    defchar:                    /* match literal character */
    default:                    /* which is what we'd do by default */
      lastep = fp - genbuf;
      GENGROW(fp, 2);
      *fp++ = CCHR;             /* insert character mark */
      *fp++ = c;
      break;
    }
  }
  GENGROW(fp, 1);
  *fp++ = 0;                    /* end this RE */
  if (brnestp != brnest) return NULL;
#if DEBUG
  if (debug >= 2) {
    for (a = genbuf; a < fp ; a++) {
      fprintf(stderr, "%02x ", *a);
    }
    fprintf(stderr, "\n");
  }
#endif
  return xdup(genbuf, fp);
}

/* interval ranges \{n,m\} are encoded as:
 * the command ir ORed with MTYPE
 * n and m are encoded as 15-bits integers: if <= 127, the unique char is 
 * the value, else the 1st char is MSB of value with high bit set and the
 * 2nd char is LSB of value.
 */
              
/* uses cp */
/* return 1 if address found */
static int address(sedcmd *cmdp, int pass)
{
  if (*cp == '$') {             /* end-of-source address */
    cp++;
    cmdp->addr[pass].num = -1;  /* -1 means last line */
    cmdp->flags |= (FLAG_L1 << pass);
    return 1;
  } else if (*cp == '/' || *cp == '\\') {      /* start of regexp match */
    if (*cp == '\\')
      cp++;
    if (NULL == (cmdp->addr[pass].re = recomp(*cp++)))
      cmderror("garbled address");             /* compile the RE */
    cmdp->flags |= (FLAG_RE1 << pass);
    return 1;
  } else if (*cp >= '0' && *cp <= '9') {
    long lno = 0;
    cmdp->flags |= (FLAG_L1 << pass);
    while (*cp >= '0' && *cp <= '9') {  /* collect digits */
      lno = lno * 10 + *cp++ - '0';      /* compute their value */
    }
    if (lno == 0) {
      cmderror("garbled address"); 
    }
    cmdp->addr[pass].num = lno;
    return 1;
  } else {
    return 0;                      /* no legal address was found */
  }
}          

/* uses global cp */
/* get the text argument of a\, c\, i\ */
static uchar *getargtext(void)
{
  uchar *fp = genbuf;
  cp = linebuf;
  for (;;) {
    uchar c = *cp;
    if (c == '\n' || c == 0) break;
    cp++;
    if (c == '\\') {
      if (*cp == 0) {
        if (!cmdline()) {
          cmderror("trailing \\");
        }
        cp = linebuf;
        GENGROW(fp, 1);
        *fp++ = '\n';
      } else {
        GENGROW(fp, 1);
        *fp++ = fixquote(&cp);
      }
    } else {
      GENGROW(fp, 1);
      *fp++ = c;
    }
  }
  GENGROW(fp, 1);
  *fp++ = 0;
  return xdup(genbuf, fp);
}


static label *search(void)
{
  label *l;
  SKIPWS(cp);
  if (!*cp)
    return NULL; /* a valid empty label */
  /* look if the label already exists */
  for (l = labels; l != NULL && strcmp(l->name, (char *)cp); l = l->next);
  if (l == NULL) {
    /* if it does not exist, add it to the list of labels */
    l = xmalloc(sizeof *l);
    l->next = labels; labels = l;
    l->name = (char *) xstrdup(cp); 
    l->address = l->list = NULL;
  } 
  *cp=0;
  return l;
}


static void resolve(void)
{
  label *l, *lnext;
  sedcmd *f, *t;
  
  for (l = labels; l != NULL; l = lnext) {
    if (NULL == l->address) fatal("undefined label `%s'", l->name);
    f = l->list;
    if (NULL != f) {
      do {
        t = f->u.link;
        f->u.link = l->address;
      } while ((f = t) != NULL);  /* v1.4 */
    }
    free(l->name);
    lnext = l->next;
    free(l);
  }
}

/* compile a y (transliterate) command */
static uchar * ycomp(void)
{
  uchar *tp, *sp, *yp, delim = *cp++;
  int c;   
  
  yp = xmalloc(256); 
  for (c = 0; c < 256; c++) {   /* fill in self-map entries in table */
    yp[c] = c;
  }
  /* scan the 'from' section for invalid chars */
  for (sp = tp = cp; *tp != delim; tp++) {
    if (*tp == '\\') {
      tp++;
    }
    if ((*tp == '\n') || (*tp == '\0')) {
      return NULL;
    }
  }
  tp++;                         /* move to the beginning of 'to' section */
  /* now rescan the 'from' section */
  while ((c = *sp++) != delim) {
    if (c == '\\') {
      c = fixquote(&sp);
    }
    if ((yp[c] = *tp++) == '\\') {
      yp[c] = fixquote(&tp);      
    } else if ((yp[c] == delim) || (yp[c] == '\0')) {
      return NULL;
    }
  }
  if (*tp != delim) {           /* 'to', 'from' parts have unequal lengths */
    return NULL;
  }
  cp = ++tp;                    /* point compile ptr past translit */
  return yp;
}

/* sedcomp.c ends here */

