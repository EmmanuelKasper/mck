# These tests were taken from the testsuite of GNU sed 4.0.5
# (some were slightly edited to make them smaller)

=IN
äÆ¤â¤è ¤ßäÆ»ý¤Á
·¡¶ú¤â¤è ¤ß·¡¶ú»ý¤Á
¤³¤ÎµÖ¤Ë ºÚÅ¦¤Þ¤¹»ù
²È´Ö¤«¤Ê ¹ð¤é¤µ¤Í
¤½¤é¤ß¤Ä ÂçÏÂ¤Î¹ñ¤Ï
¤ª¤·¤ã¤Ê¤Ù¤Æ ¤ï¤ì¤³¤½µï¤ì
¤·¤­¤Ê¤Ù¤Æ ¤ï¤ì¤³¤½ ºÂ¤»
¤ï¤Ë¤³¤½¤Ï ¹ð¤é¤á
²È¤ò¤âÌ¾¤ò¤â
=OUT
äÆ¤â¤è ¤ßäÆ»ý¤Á
·¡¶ú¤â¤è ¤ß·¡¶ú»ý¤Á
¤³¤ÎµÖ¤Ë ºÚÅ¦¤Þ¤¹»ù
²È´Ö¤«¤Ê ¹ð¤é¤µ¤Í
¤½¤é¤ß¤Ä ÆüËÜ¤Î¹ñ¤Ï
¤ª¤·¤ã¤Ê¤Ù¤Æ ¤ï¤ì¤³¤½µï¤ì
¤·¤­¤Ê¤Ù¤Æ ¤ï¤ì¤³¤½ ºÂ¤»
¤ï¤Ë¤³¤½¤Ï ¹ð¤é¤á
²È¤ò¤âÌ¾¤ò¤â
=SED s/ÂçÏÂ/ÆüËÜ/

=OUT
\344\306\244\342\244\350 \244\337\344\306\273\375\244\301$
\267\241\266\372\244\342\244\350 \244\337\267\241\266\372\273\375\244\
\301$
\244\263\244\316\265\326\244\313 \272\332\305\246\244\336\244\271\273\
\371$
\262\310\264\326\244\253\244\312 \271\360\244\351\244\265\244\315$
\244\275\244\351\244\337\244\304 \302\347\317\302\244\316\271\361\244\
\317$
\244\252\244\267\244\343\244\312\244\331\244\306 \244\357\244\354\244\
\263\244\275\265\357\244\354$
\244\267\244\255\244\312\244\331\244\306 \244\357\244\354\244\263\244\
\275 \272\302\244\273$
\244\357\244\313\244\263\244\275\244\317 \271\360\244\351\244\341$
\262\310\244\362\244\342\314\276\244\362\244\342$
=SED
# it isn't entirely clear how lines should be wrapped.  Plus I am
# not sure whether sed is required to append a $ at the end of line or
# not.
l;d
.


=IN
foo foo fo oo f oo foo foo foo foo foo foo foo foo foo foo foo foo foo
=OUT
bar bar fo oo f oo bar bar bar bar bar bar bar bar bar bar bar bar bar
=SED
s/foo/bar/g
.

=IN
192.168.1.2 br b8<r b16<r b24< R|R|R| D
255.255.255.0 br b8<r b16<r b24< R|R|R| D~r
& DDD 24>dpP 16>11111111& dpP 8>11111111& dpP 11111111& dpP
| DDD 24>dpP 16>11111111& dpP 8>11111111& dpP 11111111& dpP
=OUT
192
168
1
0
192
168
1
255
=FILE gnutests/binary.sed
=FILE gnutests/binary2.sed
=FILE gnutests/binary3.sed

=IN
_cv_=emptyvar
ac_cv_prog_RANLIB=/usr/bin/ranlib
ac_cv_prog_CC=/usr/unsupported/\ \ /lib/_cv_/cc
ac_cv_prog_CPP=/usr/bin/cpp
SHELL=bash
GNU=GNU!UNIX
=OUT
: ${_cv_='emptyvar'}
: ${ac_cv_prog_RANLIB='/usr/bin/ranlib'}
: ${ac_cv_prog_CC='/usr/unsupported/\ \ /lib/_cv_/cc'}
: ${ac_cv_prog_CPP='/usr/bin/cpp'}
=SED
#n
# inspired by an autoconf generated configure script.
s/^\([a-zA-Z0-9_]*_cv_[a-zA-Z0-9_]*\)=\(.*\)/: \${\1='\2'}/p
.

=IN
# Compute Easter of 2002...
# usage:   (echo YEAR; cat easter.dc) | dc

2002

[ddsf[lfp[too early
]Pq]s@1583>@
ddd19%1+sg100/1+d3*4/12-sx8*5+25/5-sz5*4/lx-10-sdlg11*20+lz+lx-30%
d[30+]s@0>@d[[1+]s@lg11<@]s@25=@d[1+]s@24=@se44le-d[30+]s@21>@dld+7%-7+
[March ]smd[31-[April ]sm]s@31<@psnlmPpsn1z>p]splpx

# Compute square root of 2

16oAk2vpq
=OUT
31
March 2002
1.6A09E667A
=FILE gnutests/dc.sed

=IN
From crash@cygnus.com  Wed Mar  8 18:02:42 1995
Received: from s1.msi.umn.edu (s1.msi.umn.edu [128.101.24.1]) by cygnus.com (8.6.9/8.6.9) with ESMTP id SAA21692 for <crash@cygnus.com>; Wed, 8 Mar 1995 18:02:41 -0800
Received: from cygint.cygnus.com (cygint.cygnus.com [140.174.1.1]) by s1.msi.umn.edu (8.6.10/8.6.9) with ESMTP id TAA13398 for <molenda@msi.umn.edu>; Wed, 8 Mar 1995 19:59:18 -0600
Received: from phydeaux.cygnus.com (phydeaux.cygnus.com [140.174.1.85]) by cygnus.com (8.6.9/8.6.9) with SMTP id SAA21688 for <molenda@msi.umn.edu>; Wed, 8 Mar 1995 18:02:33 -0800
From: Jason Molenda <crash@cygnus.com>
Received: by phydeaux.cygnus.com (5.65/4.7) id AA06931; Wed, 8 Mar 1995 18:02:28 -0800
Message-Id: <9503090202.AA06931@phydeaux.cygnus.com>
Subject: Note for sed testsuite
To: molenda@msi.umn.edu
Date: Wed, 8 Mar 1995 18:02:24 -0800 (PST)
X-Mailer: ELM [version 2.4 PL23]

                _Summum Bonum_

    All the breath and the bloom of the
            year in the bag of one bee:
    All the wonder and wealth of the mine in
         the heart of one gem:
    In the core of one pearl all the shade and the
           shine of the sea:
    Breath and bloom, shade and shine, -- wonder,
        wealth, and -- how far above them --
          Truth, thats brighter than gem,
          Trust, that's purer than pearl, --
    Brightest truth, purest trust in the universe --
              all were for me
                 In the kiss of one girl.
        -- Robert Browning
=OUT
Path: mailnewsgateway
From crash@cygnus.com  Wed Mar  8 18: 02:42 1995
From: crash@cygnus.com (Jason Molenda)
Message-ID: <9503090202.AA06931.alt.buddha.fat.short.guy@phydeaux.cygnus.com>
Subject: Note for sed testsuite
Original-To: molenda@msi.umn.edu
Date: Wed, 8 Mar 1995 18:02:24 -0800 (PST)
X-Mailer: ELM [version 2.4 PL23]
Newsgroups: alt.buddha.short.fat.guy
Distribution: world
Sender: news@cygnus.com
Approved: alt.buddha.short.fat.guy@cygnus.com

                _Summum Bonum_

    All the breath and the bloom of the
            year in the bag of one bee:
    All the wonder and wealth of the mine in
         the heart of one gem:
    In the core of one pearl all the shade and the
           shine of the sea:
    Breath and bloom, shade and shine, -- wonder,
        wealth, and -- how far above them --
          Truth, thats brighter than gem,
          Trust, that's purer than pearl, --
    Brightest truth, purest trust in the universe --
              all were for me
                 In the kiss of one girl.
        -- Robert Browning
=FILE gnutests/distrib.sed

=IN
I can't quite remember where I heard it,
but I can't seem to get out of my head
the phrase
the final frontier
=OUT
I can't quite remember where I heard it,
but I can't seem to get out of my head
the phrase
space the final frontier
=SED $s/^/space /

=IN
x

=OUT
x

=SED s/^ *//

=IN
--enable-targets=sparc-sun-sunos4.1.3,srec
--enable-x11-testing=on
--enable-wollybears-in-minnesota=yes-id-like-that
=OUT
targets
x11-testing
wollybears-in-minnesota
=SED
# inspired by an autoconf generated configure script.
s/-*enable-//;s/=.*//
.


=IN
2
3
4
5
8
11
16
143
=OUT
2
3
2
2
5
2
2
2
11
2
2
2
2
13
11
=FILE gnutests/factor.sed

=IN
aaaaaaabbbbbbaaaaaaa
=OUT
aaaaaabbbbbbaaaaaaa
bbbbbb
aaaaaabbbbbbaaaaaaa
aaaaaabbbbbbaaaaaaa
aaaaaaabbbbbbaaaaaaa
aaaaaabbbbbbaaaaaaa
aaaaaaabbbbbbaaaaaa
bbbbbbbbbbbbbbbbbbb

bbbbbbbbbbbbbbbbbbb
bbbbbbbbbbbbbbbbbbb
bbbbbbbbbbbbbbbbbbb
bbbbbbbbbbbbbbbbbbb
bbbbbbbbbbbbbbbbbbbb
=SED
# test `fast' substitutions

h
s/a//
p
g
s/a//g
p
g
s/^a//p
g
s/^a//g
p
g
s/not present//g
p
g
s/^[a-z]//g
p
g
s/a$//
p
g

y/a/b/
h
s/b//
p
g
s/b//g
p
g
s/^b//p
g
s/^b//g
p
g
s/^[a-z]//g
p
g
s/b$//
p
g
.



=IN
   "...by imposing a tiny bit of order in a communication you are
   translating, you are carving out a little bit of order in the
   universe.  You will never succeed.  Everything will fail and come
   to an end finally.  But you have a chance to carve a little bit
   of order and maybe even beauty out of the raw materials that
   surround you everywhere, and I think there is no greater meaning
   in life."

             Donald L. Philippi, Oct 1930 - Jan 1993
=OUT
   "...by imposing a tiny bit of order in a communication you are
   translating, you are carving out a little bit of order in the
   universe.  You will never succeed.  Everything will fail and come
=SED 3q


=IN
	/usr/X11R6/lib        
	/usr/X11R5/lib        
	/usr/X11R4/lib        
	
	/usr/lib/X11R6        
	/usr/lib/X11R5        
	/usr/lib/X11R4        
	
	/usr/local/X11R6/lib  
	/usr/local/X11R5/lib  
	/usr/local/X11R4/lib  
	
	/usr/local/lib/X11R6  
	/usr/local/lib/X11R5  
	/usr/local/lib/X11R4  
	
	/usr/X11/lib          
	/usr/lib/X11          
	/usr/local/X11/lib    
	/usr/local/lib/X11    
	
	/usr/X386/lib         
	/usr/x386/lib         
	/usr/XFree86/lib/X11  
	
	/usr/lib              
	/usr/local/lib        
	/usr/unsupported/lib  
	/usr/athena/lib       
	/usr/local/x11r5/lib  
	/usr/lpp/Xamples/lib  
	
	/usr/openwin/lib      
	/usr/openwin/share/lib 
=OUT
	/usr/X11R6/include        
	/usr/X11R5/include        
	/usr/X11R4/include        
	
	/usr/include/X11R6        
	/usr/include/X11R5        
	/usr/include/X11R4        
	
	/usr/local/X11R6/include  
	/usr/local/X11R5/include  
	/usr/local/X11R4/include  
	
	/usr/local/include/X11R6  
	/usr/local/include/X11R5  
	/usr/local/include/X11R4  
	
	/usr/X11/include          
	/usr/include/X11          
	/usr/local/X11/include    
	/usr/local/include/X11    
	
	/usr/X386/include         
	/usr/x386/include         
	/usr/XFree86/include/X11  
	
	/usr/include              
	/usr/local/include        
	/usr/unsupported/include  
	/usr/athena/include       
	/usr/local/x11r5/include  
	/usr/lpp/Xamples/include  
	
	/usr/openwin/include      
	/usr/openwin/share/include 
=SED
# inspired by an autoconf generated configure script.
s;lib;include;
.

=IN
1)  Muammar Qaddafi
2)  Mo'ammar Gadhafi
3)  Muammar Kaddafi
4)  Muammar Qadhafi
5)  Moammar El Kadhafi
6)  Muammar Gadafi
7)  Mu'ammar al-Qadafi
8)  Moamer El Kazzafi
9)  Moamar al-Gaddafi
10) Mu'ammar Al Qathafi
11) Muammar Al Qathafi
12) Mo'ammar el-Gadhafi
13) Moamar El Kadhafi
14) Muammar al-Qadhafi
15) Mu'ammar al-Qadhdhafi
16) Mu'ammar Qadafi
17) Moamar Gaddafi
18) Mu'ammar Qadhdhafi
19) Muammar Khaddafi
20) Muammar al-Khaddafi
21) Mu'amar al-Kadafi
22) Muammar Ghaddafy
23) Muammar Ghadafi
24) Muammar Ghaddafi
25) Muamar Kaddafi
26) Muammar Quathafi
27) Muammar Gheddafi
28) Muamar Al-Kaddafi
29) Moammar Khadafy 
30) Moammar Qudhafi
31) Mu'ammar al-Qaddafi
32) Mulazim Awwal Mu'ammar Muhammad Abu Minyar al-Qadhafi
=OUT
1)  Muammar Qaddafi
2)  Mo'ammar Gadhafi
3)  Muammar Kaddafi
4)  Muammar Qadhafi
5)  Moammar El Kadhafi
6)  Muammar Gadafi
7)  Mu'ammar al-Qadafi
8)  Moamer El Kazzafi
9)  Moamar al-Gaddafi
10) Mu'ammar Al Qathafi
11) Muammar Al Qathafi
12) Mo'ammar el-Gadhafi
13) Moamar El Kadhafi
14) Muammar al-Qadhafi
15) Mu'ammar al-Qadhdhafi
16) Mu'ammar Qadafi
17) Moamar Gaddafi
18) Mu'ammar Qadhdhafi
19) Muammar Khaddafi
20) Muammar al-Khaddafi
21) Mu'amar al-Kadafi
22) Muammar Ghaddafy
23) Muammar Ghadafi
24) Muammar Ghaddafi
25) Muamar Kaddafi
26) Muammar Quathafi
27) Muammar Gheddafi
28) Muamar Al-Kaddafi
29) Moammar Khadafy 
30) Moammar Qudhafi
31) Mu'ammar al-Qaddafi
32) Mulazim Awwal Mu'ammar Muhammad Abu Minyar al-Qadhafi
=SED
# The Khadafy test is brought to you by Scott Anderson . . .
/M[ou]'\{0,1\}am\{1,2\}[ae]r .*\([AEae]l[- ]\)\{0,1\}[GKQ]h\{0,1\}[aeu]\{1,\}\([dtz][dhz]\{0,1\}\)\{1,\}af[iy]/!d
.

=IN
A dialogue on poverty

        On the night when the rain beats,
        Driven by the wind,
[...]
        -- Yamanoue Okura
=OUT
1
A dialogue on poverty
2

3
        On the night when the rain beats,
4
        Driven by the wind,
5
[...]
6
        -- Yamanoue Okura
=SED =

=INFILE gnutests/mac-mf.inp
=OUTFILE gnutests/mac-mf.good
=FILE gnutests/mac-mf.sed

=IN
s%@CFLAGS@%%g
s%@CPPFLAGS@%-I/%g
s%@CXXFLAGS@%-x c++%g
s%@DEFS@%$DEFS%g
s%@LDFLAGS@%-L/usr/lib%g
s%@LIBS@%-lgnu -lbfd%g
s%@exec_prefix@%%g
s%@prefix@%$prefix%g
s%@RANLIB@%$RANLIB%g
s%@CC@%/usr/local/bin/gcc%g
s%@CPP@%$CPP%g
s%@XCFLAGS@%$XCFLAGS%g
s%@XINCLUDES@%$XINCLUDES%g
s%@XLIBS@%$XLIBS%g
s%@XPROGS@%$XPROGS%g
s%@TCLHDIR@%$TCLHDIR%g
s%@TCLLIB@%$TCLLIB%g
s%@TKHDIR@%$TKHDIR%g
s%@TKLIB@%$TKLIB%g
s%@PTY_TYPE@%$PTY_TYPE%g
s%@EVENT_TYPE@%$EVENT_TYPE%g
s%@SETUID@%$SETUID%g
=OUT
s%@CFLAGS@%%g
s%@CPPFLAGS@%-I/%g
s%@CXXFLAGS@%-x c++%g
s%@DEFS@%$DEFS%g
s%@LDFLAGS@%-L/usr/lib%g
s%@LIBS@%-lgnu -lbfd%g
s%@exec_prefix@%%g
s%@prefix@%$prefix%g
s%@RANLIB@%$RANLIB%g
s%@CC@%/usr/local/bin/gcc%g
s%@CPP@%$CPP%g
s%@XCFLAGS@%$XCFLAGS%g
s%@XINCLUDES@%$XINCLUDES%g
s%@XLIBS@%$XLIBS%g
s%@XPROGS@%$XPROGS%g
s%@TCLHDIR@%$TCLHDIR%g
s%@TCLLIB@%$TCLLIB%g
s%@TKHDIR@%$TKHDIR%g
s%@TKLIB@%$TKLIB%g
s%@PTY_TYPE@%$PTY_TYPE%g
s%@EVENT_TYPE@%$EVENT_TYPE%g
s%@SETUID@%$SETUID%g
=SED
# straight out of an autoconf-generated configure.
# The input should look just like the input after this is run.
#
# Protect against being on the right side of a sed subst in config.status. 
s/%@/@@/; s/@%/@@/; s/%g$/@g/; /@g$/s/[\\\\&%]/\\\\&/g; 
 s/@@/%@/; s/@@/@%/; s/@g$/%g/
.

=IN
   "...by imposing a tiny bit of order in a communication you are
   translating, you are carving out a little bit of order in the
   universe.  You will never succeed.  Everything will fail and come
   to an end finally.  But you have a chance to carve a little bit
   of order and maybe even beauty out of the raw materials that
   surround you everywhere, and I think there is no greater meaning
   in life."

             Donald L. Philippi, Oct 1930 - Jan 1993
=OUT
   universe.  You will never succeed.  Everything will fail and come
   to an end finally.  But you have a chance to carve a little bit
   of order and maybe even beauty out of the raw materials that
=SED
#n
3,5p
.

=IN
$B$H$J$j$N$?$1$,$-$K(B
$B$?$F$+$1$?$N$O(B
$B$?$F$+$1$?$+$C$?$+$i(B
$B$?$F$+$1$?!#(B
=OUT
$B$H$J$j$NM9JX6I$K(B
$B$?$F$+$1$?$N$O(B
$B$?$F$+$1$?$+$C$?$+$i(B
$B$?$F$+$1$?!#(B
=SED
s/$?$1$,$-/M9JX6I/
.

=IN
This file is unique
in that it does
end in a newline.
=OUT
This file is uniquewakuwaku
in that it doeswakuwaku
end in a newline.wakuwaku
=SED
s/$/wakuwaku/g
.

=IN
foo foo fo oo f oo foo foo foo foo foo foo foo foo foo foo foo foo foo
foo foo fo oo f oo foo foo foo foo foo foo foo foo foo foo foo foo foo
=OUT
foo foo fo oo f oo foo foo foo foo foo foo foo bar foo foo foo foo foo
=SED
# the first one matches, the second doesn't
1s/foo/bar/10
2s/foo/bar/20

# The second line should be deleted.  ssed 3.55-3.58 do not.
t
d
.

=MOO MOO
=IN
``Democracy will not come today, this year,
  nor ever through compromise and fear.
  I have as much right as the other fellow has
  to stand on my two feet and own the land.
  I tire so of hearing people say
  let things take their course,
  tomorrow is another day.
  I do not need my freedom when I'm dead.
  I cannot live on tomorrow's bread.
  Freedom is a strong seed
  planted in a great need.
  I live here, too.
  I want freedom just as you.''
    ``The Weary Blues'', Langston Hughes
=OUT
``Democracy will not come today, this year,
  nor ever through compromise and fear.
MOO
  I have as much right as the other fellow has
  to stand on my two feet and own the land.
MOO
  I tire so of hearing people say
  let things take their course,
  tomorrow is another day.
MOO
  I do not need my freedom when I'm dead.
MOO
  I cannot live on tomorrow's bread.
MOO
  Freedom is a strong seed
  planted in a great need.
MOO
  I live here, too.
MOO
  I want freedom just as you.''
    ``The Weary Blues'', Langston Hughes
=SED
/\.$/r tmp.MOO
.

=IN
eeefff
=OUT
eeefff
Xeefff
XYefff
XYeYff
XYeYYf
XYeYYY
XYeYYY
=SED
# Check that the empty regex recalls the last *executed* regex,
# not the last *compiled* regex
p
s/e/X/p
:x
s//Y/p
/f/bx
.

=IN
miss mary mack mack//mack/ran down/the track  track  track
slashes\aren't%used enough/in/casual-conversation///
possibly sentences would be more attractive if they ended in two slashes//
=OUT

///
//
=SED
# inspired by an autoconf generated configure script.
s%/[^/][^/]*$%%
s%[\/][^\/][^\/]*$%%
s,.*[^\/],,
.

=IN


# If the @code{N} command had added the last line, print and exit
# If the @code{N} command had added the last line, print and exit
bb
ccc
ccc
ccc
ccc
# On the last line, print and exit
=OUT

# If the @code{N} command had added the last line, print and exit
bb
ccc
# On the last line, print and exit
=SED
h

:b
# On the last line, print and exit
$b
N
/^\(.*\)\n\1$/ {
    # The two lines are identical.  Undo the effect of
    # the n command.
    g
    bb
}

# If the @code{N} command had added the last line, print and exit
$b

# The lines are different; print the first and go
# back working on the second.
P
D
.

=IN
roses are red
violets are blue
my feet are cold
your feet are blue
=OUT
roses are red
violets are blue
my feet are cold
your feet are too
=SED
# from the ChangeLog (Fri May 21 1993)
\xfeetxs/blue/too/
.

=IN

b
bc
bac
baac
baaac
baaaac
=OUT
x
xbx
xbxcx
xbxcx
xbxcx
xbxcx
xbxcx
=SED
# from the ChangeLog (Wed Sep 5 2001)
s/a*/x/g
.

=IN
# Makefile.in generated automatically by automake 1.5 from Makefile.am.

# Copyright 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001
# Free Software Foundation, Inc.
# This Makefile.in is free software; the Free Software Foundation
# gives unlimited permission to copy and/or distribute it,
# with or without modifications, as long as this notice is preserved.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY, to the extent permitted by law; without
# even the implied warranty of MERCHANTABILITY or FITNESS FOR A
# PARTICULAR PURPOSE.

@SET_MAKE@

# Automake requirements

SHELL = @SHELL@

PACKAGE = sed

EXTRA_DIST = BUGS THANKS README.boot bootstrap.sh dc.sed autogen \
	m4/codeset.m4  m4/gettext.m4  m4/iconv.m4      m4/lcmessage.m4 \
	m4/getline.m4  m4/glibc21.m4  m4/isc-posix.m4  m4/progtest.m4 \
	m4/obstack.m4

subdir = .
ACLOCAL_M4 = $(top_srcdir)/aclocal.m4
mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs
CONFIG_HEADER = config.h
CONFIG_CLEAN_FILES = bootstrap.sh intl/Makefile
DIST_SOURCES =
DATA = $(noinst_DATA)

HEADERS = $(noinst_HEADERS)


RECURSIVE_TARGETS = info-recursive dvi-recursive install-info-recursive \
	uninstall-info-recursive all-recursive install-data-recursive \
	install-exec-recursive installdirs-recursive install-recursive \
	uninstall-recursive check-recursive installcheck-recursive
DIST_COMMON = README $(noinst_HEADERS) ./stamp-h.in ABOUT-NLS AUTHORS \
	COPYING ChangeLog INSTALL Makefile.am Makefile.in NEWS THANKS \
	TODO acconfig.h aclocal.m4 bootstrap.sh.in config.guess \
	config.sub config_h.in configure configure.ac depcomp \
	install-sh missing mkinstalldirs
DIST_SUBDIRS = $(SUBDIRS)
all: config.h
	$(MAKE) $(AM_MAKEFLAGS) all-recursive

.SUFFIXES:
$(srcdir)/Makefile.in:  Makefile.am  $(top_srcdir)/configure.ac $(ACLOCAL_M4)
	cd $(top_srcdir) && \
	  $(AUTOMAKE) --gnu  Makefile
Makefile:  $(srcdir)/Makefile.in  $(top_builddir)/config.status
	cd $(top_builddir) && \
	  CONFIG_HEADERS= CONFIG_LINKS= \
	  CONFIG_FILES=$@ $(SHELL) ./config.status

$(top_builddir)/config.status: $(srcdir)/configure $(CONFIG_STATUS_DEPENDENCIES)
	$(SHELL) ./config.status --recheck
$(srcdir)/configure:  $(srcdir)/configure.ac $(ACLOCAL_M4) $(CONFIGURE_DEPENDENCIES)
	cd $(srcdir) && $(AUTOCONF)

$(ACLOCAL_M4):  configure.ac m4/codeset.m4 m4/getline.m4 m4/gettext.m4 m4/glibc21.m4 m4/iconv.m4 m4/isc-posix.m4 m4/lcmessage.m4 m4/obstack.m4 m4/progtest.m4
	cd $(srcdir) && $(ACLOCAL) $(ACLOCAL_AMFLAGS)
config.h: stamp-h
=OUT
#Makefile.in generated automatically by automake 1.5 from Makefile.am.

#Copyright 1994, 1995, 1996, 1997, 1998, 1999, 2000, 2001
#Free Software Foundation, Inc.
#This Makefile.in is free software; the Free Software Foundation
#gives unlimited permission to copy and/or distribute it,
#with or without modifications, as long as this notice is preserved.

#This program is distributed in the hope that it will be useful,
#but WITHOUT ANY WARRANTY, to the extent permitted by law; without
#even the implied warranty of MERCHANTABILITY or FITNESS FOR A
#PARTICULAR PURPOSE.

"@SET_MAKE@"

#Automake requirements

"SHELL = @SHELL@"

"PACKAGE = sed"

"EXTRA_DIST = BUGS THANKS README.boot bootstrap.sh dc.sed autogen \\"
"	m4/codeset.m4  m4/gettext.m4  m4/iconv.m4      m4/lcmessage.m4 \\"
"	m4/getline.m4  m4/glibc21.m4  m4/isc-posix.m4  m4/progtest.m4 \\"
"	m4/obstack.m4"

"subdir = ."
"ACLOCAL_M4 = $(top_srcdir)/aclocal.m4"
"mkinstalldirs = $(SHELL) $(top_srcdir)/mkinstalldirs"
"CONFIG_HEADER = config.h"
"CONFIG_CLEAN_FILES = bootstrap.sh intl/Makefile"
"DIST_SOURCES ="
"DATA = $(noinst_DATA)"

"HEADERS = $(noinst_HEADERS)"


"RECURSIVE_TARGETS = info-recursive dvi-recursive install-info-recursive \\"
"	uninstall-info-recursive all-recursive install-data-recursive \\"
"	install-exec-recursive installdirs-recursive install-recursive \\"
"	uninstall-recursive check-recursive installcheck-recursive"
"DIST_COMMON = README $(noinst_HEADERS) ./stamp-h.in ABOUT-NLS AUTHORS \\"
"	COPYING ChangeLog INSTALL Makefile.am Makefile.in NEWS THANKS \\"
"	TODO acconfig.h aclocal.m4 bootstrap.sh.in config.guess \\"
"	config.sub config_h.in configure configure.ac depcomp \\"
"	install-sh missing mkinstalldirs"
"DIST_SUBDIRS = $(SUBDIRS)"
"all: config.h"
"	$(MAKE) $(AM_MAKEFLAGS) all-recursive"

".SUFFIXES:"
"$(srcdir)/Makefile.in:  Makefile.am  $(top_srcdir)/configure.ac $(ACLOCAL_M4)"
"	cd $(top_srcdir) && \\"
"	  $(AUTOMAKE) --gnu  Makefile"
"Makefile:  $(srcdir)/Makefile.in  $(top_builddir)/config.status"
"	cd $(top_builddir) && \\"
"	  CONFIG_HEADERS= CONFIG_LINKS= \\"
"	  CONFIG_FILES=$@ $(SHELL) ./config.status"

"$(top_builddir)/config.status: $(srcdir)/configure $(CONFIG_STATUS_DEPENDENCIES)"
"	$(SHELL) ./config.status --recheck"
"$(srcdir)/configure:  $(srcdir)/configure.ac $(ACLOCAL_M4) $(CONFIGURE_DEPENDENCIES)"
"	cd $(srcdir) && $(AUTOCONF)"

"$(ACLOCAL_M4):  configure.ac m4/codeset.m4 m4/getline.m4 m4/gettext.m4 m4/glibc21.m4 m4/iconv.m4 m4/isc-posix.m4 m4/lcmessage.m4 m4/obstack.m4 m4/progtest.m4"
"	cd $(srcdir) && $(ACLOCAL) $(ACLOCAL_AMFLAGS)"
"config.h: stamp-h"
=SED
# Inspired by xemacs' config.status script
# submitted by John Fremlin (john@fremlin.de)

/^# Generated/d
s%/\*\*/#.*%%
s/^ *# */#/
/^##/d
/^#/ {
  p
  d
}
/./ {
  s/\([\"]\)/\\\1/g
  s/^/"/
  s/$/"/
}
.


