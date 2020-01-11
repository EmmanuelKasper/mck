/*
 * escape.c - escape and unescape characters
 *
 * (C) 2003 Laurent Vogel
 * GPL version 2 or later (http://www.gnu.org/copyleft/gpl.html)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
 
#define VERSION "1.0"

int maxlen = 79;

void escape(int esc, FILE *in, FILE *out)
{
  int len = 0;
#define NEED(_n) \
  do { \
    int n = _n; \
    len += n; \
    if (len >= maxlen) { \
      fprintf(out, "%c\n", esc); \
      len = n; \
    } \
  } while(0)
  
  for (;;) {
    int c = getc(in);
again:
    if (c == EOF) {
      putc(esc, out);
      putc('\n', out);
      return;
    } else if (c == '\n') {
      len = 0;
      putc('\n', out);
      /* look ahead if next char is EOF - other EOFs will be
       * preceded by escape newline 
       */
      c = getc(in);
      if (c == EOF) return;
      goto again;
    } else if (c == esc) {
      escape:
      NEED(2);
      putc(esc, out);
      putc(c, out);
      continue;
    } else if (c == '\a') {
      if (esc == 'a') goto octal;
      c = 'a'; goto escape;
    } else if (c == '\b') {
      if (esc == 'b') goto octal;
      c = 'b'; goto escape;
    } else if (c == '\f') {
      if (esc == 'f') goto octal;
      c = 'f'; goto escape;
    } else if (c == '\n') {
      if (esc == 'n') goto octal;
      c = 'n'; goto escape;
    } else if (c == '\r') {
      if (esc == 'r') goto octal;
      c = 'r'; goto escape;
    } else if (c == '\t') {
      if (esc == 't') goto octal;
      c = 't'; goto escape;
    } else if (c == '\v') {
      if (esc == 'v') goto octal;
      c = 'v'; goto escape;
    } else if (c < 32 || c > 126) {
      if (esc >= '0' && esc <= '7') {
        NEED(4);
        fprintf(out, "%cx%02x", esc, c);
      } else {
        octal:
        {
          int u = c;
          c = getc(in);
          if (c >= '0' && c <= '7') {
            NEED(4);
            fprintf(out, "%c%03o", esc, u);
          } else {
            NEED( u <= 07 ? 2 : ( u <= 077 ? 3 : 4 ) );
            fprintf(out, "%c%o", esc, u);
          }
          goto again;
        }
      }
      continue;
    }
    len++;
    if (len == maxlen) {
      int u = c;
      /* check if next char is a newline. */
      c = getc(in);
      if (c == '\n') {
        /* there's enough room */
        putc(u, out);
      } else {
        /* not enough room */
        putc(esc, out);
        putc('\n', out);
        putc(u, out);
        len = 1;
      }
      goto again;
    } else {
      putc(c, out);
    }
  }
}

void unescape(int esc, FILE *in, FILE *out)
{
  for (;;) {
    int c = getc(in);
  again:
    if (c == esc) {
      c = getc(in);
      if (c == esc) {
        ;
      } else if (c >= '0' && c <= '7') {
        int u = 0;
        int i;
        for(i = 0 ; i < 3 ; i++) {
          if (c < '0' || c > '7') break;
          u <<= 3;
          u += (c-'0');
          c = getc(in);
        }
        putc(u, out);
        goto again;
      } else if (c == 'x') {
        int u = 0;
        int i;
        for(i = 0 ; i < 2 ; i++) {
          c = getc(in);
          if(c >= '0' && c <= '9') {
            u <<= 4; u += c - '0';
          } else if(c >= 'A' && c <= 'F') {
            u <<= 4; u += c - 'A' + 10;
          } else if(c >= 'a' && c <= 'f') {
            u <<= 4; u += c - 'a' + 10;
          } else {
            putc(u, out);
            goto again;
          }
        }
        putc(u, out);
        continue;
      } else if (c == 'a') { 
        c = '\a';
      } else if (c == 'b') { 
        c = '\b';
      } else if (c == 'f') { 
        c = '\f';
      } else if (c == 'n') { 
        c = '\n'; 
      } else if (c == 'r') { 
        c = '\r';
      } else if (c == 't') { 
        c = '\t';
      } else if (c == 'v') { 
        c = '\v';
      } else if (c == '\n') {
        continue;
      } else if (c == EOF) {
        return;
      } else {
        putc(esc, out);
      }
    } else if (c == EOF) {
      return;
    }
    putc(c, out);
  }
}

void usage(int fatal)
{
  fprintf(stderr, 
"usage: escape [options] [FILE]\n"
"options are (at most one of each):\n"
"  -o FILE  write the output to file FILE (default stdout)\n"
"  -u   unescape (default escape)\n"
"  -cX  use char X as escape char for any X except newline (default \\)\n"
"stdin is taken as input if no FILE is specified.\n"
"The following escape sequences are recognised:\n"
"  XX: a litteral escape character\n"
"  Xa, Xb, Xf, Xn, Xr, Xt, Xv: like \\a, \\b, \\f, \\n, \\r, \\t, \\v in C;\n"
  );
  fprintf(stderr,   
"  Xo, Xoo, Xooo: octal number (one, two or three octal digits);\n"
"  Xx, Xxd, Xxdd: hex number (zero, one or two hex digits [0-9A-Fa-f]);\n"
"  X at end of line: join the line to the next one, if any, or indicate\n"
"    missing trailing newline if at EOF;\n"
"  XY: the two chars XY for any other char Y.\n"
"Sequence XX has precedence over all other ones: for instance, if the escape\n"
"char is 't', then 'tt' means a litteral 't' and any one of 't11', 't011',\n"
"'txb', 'txB', 'tx0b', 'tx0B' means a tabulation.\n"
  );
  exit(fatal ? EXIT_FAILURE : 0);
}

int main(int argc, char *argv[])
{
  int uflag = 0;
  int esc = '\\';
  char *ifname = NULL;
  char *ofname = NULL;
  FILE *in, *out;
  while (--argc) {
    char *a = *++argv;
    if (*a == '-') {
      ++a;
      if (*a == '-') {
        if (!strcmp(a, "help")) {
          usage(0);
        } else if (!strcmp(a, "version")) {
          fprintf(stderr, "escape version " VERSION "\n");
          exit(0);
        } else {
          usage(1);
        }
      } else {
        while (*a) {
          int c = *a++;
          switch (c) {
          case 'h': usage(0); break;
          case 'v': 
            fprintf(stderr, "escape version " VERSION "\n");
            exit(0);
            break;
          case 'u': 
            uflag = 1;
            break;
          case 'c':
            if (*a == 0 || *a == '\n' || a[1] != 0) {
              usage(1);
            }
            esc = *a++;
            break;
          case 'o':
            if (ofname != NULL) usage(1);
            if (*a == 0) {
              if(--argc < 0) usage(1); 
              ofname = *++argv;
            } else {
              ofname = a;
              a = "";
            }
            break;
          default:
            usage(1);
          }
        }
      }
    } else {
      if (ifname == NULL) {
        ifname = a;
      } else {
        usage(1);
      }
    }
  }
  
  if (ifname == NULL) {
    in = stdin;
  } else {
    in = fopen(ifname, "r");
    if (in == NULL) {
      fprintf(stderr, "cannot open %s: %s\n", ifname, strerror(errno));
      exit(EXIT_FAILURE);
    } 
  }
  if (ofname == NULL) {
    out = stdout;
  } else {
    out = fopen(ofname, "w");
    if (out == NULL) {
      fprintf(stderr, "cannot open %s: %s\n", ofname, strerror(errno));
      exit(EXIT_FAILURE);
    } 
  }
  
  if (uflag) {
    unescape(esc, in, out);
  } else {
    escape(esc, in, out);
  }
  fclose(in);
  fclose(out);
  return 0;
}

