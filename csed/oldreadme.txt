SED (AKA HHSED)						2 Oct 91

This all started with a port of sed (SIMTEL20's PD:<MSDOS.TXTUTL>SED.ARC)
to TC 2.0. I was basically thrashing in the dark with NO idea what sed
was *really* supposed to do.  (I only wanted something to work with shar
files.)

It worked .. kinda .. and I threw it back up on SIMTEL20 as SED12.ZIP.
Then Howard L. Helman (h**2) found it, discovered it wouldn't compile
with BCC++, and jumped in with a truly massive, detailed rewrite
(his HHSED contribution).

It's all integrated now and appears to work in MS-DOS as compiled
by various flavors of Borland compilers.

I am *not* a sed wizard:  I'm only putting this sucker together
and fielding it to the world.  For details, you need to get with
Howard.

There's room for improvement:  see the documentation.

There's competition (the GNU sed).  Differences between them and us?
Beats me.  Who's best?  Who knows.

This would be public domain, except people keep *stealing* from the
public domain, copyrighting it as their own, and restricting the rest
of us from using it again.  With that filthy, gods-cursed problem in
mind, I'll copyright this sucker for the original author, me and h**2,
just to keep the wicked in line.

You, Dear Reader, may do *anything* you wish with it except steal it.

Copyright (c) 1991 Eric S. Raymond, David P Kirschbaum & Howard L. Helman
All Rights Reserved

David Kirschbaum
Toad Hall
kirsch@usasoc.soc.mil

=== Recent History and Comments ===

Return-Path: <helman@elm.sdd.trw.com>
Date: Mon, 23 Sep 91 09:24:22 PDT
From: helman@elm.sdd.trw.com (Howard L. Helman)

This archive contains my update to the sed stream editor.

The following files should be in the archive:
   1. sed.h       -- source header file 
   2. sedcomp.c   -- source C command compiler
   3. sedexec.c   -- source C sed execution code
   4. README      -- this file

TC 2.0-related files (TC20.ZIP)
   1. TCCONFIG.TC -- TC v2.0 config file
   2. SED.PRJ     -- TC v2.0 project file

Document-related files (DOCS.ZIP)
   1. sed.tex    -- documentation in LaTex format
   2. sed.sty    -- style file for LaTex document
   3. sed.lst    -- documentation in plain ASCII

   Sed.lst should print on any dumb printer it is exactly 66 lines
   per page and has no special character.

   To see a really nice printout of the documentation you should
   try to run the .tex version.  A LaTex style file for sed is
   also included.   

Miscellaneous background files (HISTORY.ZIP)


Compile Instructions (from h**2, slightly edited):

To create sed.com from the supplied source the following can be used
(assuming BCC):
   1. sed.h sedcomp.c and sedexec.c were placed in the default diretory
   2. A turboc.cfg is created containing the following:
		-w-stk -w-pia -a -G -O -Z  -P-.c 
		-Ic:\BC2\INCLUDE
		-Lc:\BC2\LIB
   3. Adjust the defaults for INCLUDE and LIB for your system
   4. invoke the complier: 
        BCC -mt -lt -esed sedcomp.c sedexec.c c:\bc2\lib\wildargs.obj
   5. sed.com created.
   6. TCC should work the -P-.c should not be needed.
      [If your TCC is not configured to consider signed chars the
      default, you should include the -K- switch to force signed chars.]
   7. BC or TC could also be used in a manner similar to the above.
      [(For TC 2.0, use the provided TCCONFIG.TC and SED.PRJ files.)]
   8. On the sun4 under Unix the command was:
        cc -o hhsed sedcomp.c sedexec.c   

========

v1.4, 20 Sep 91:

The new h**2 code (and probably v1.2) was broken under TC 2.0
(with the last line of the included test files garbaged).

There were also several dozen warning messages that disturbed me.

After chasing down and eliminating all the warnings and (the
*real* bug fix) switching to signed chars, it started working.

Redistributing for testing.  Now including the little test files
which I found somewhere (I forget):
  ctrans
  pascal
  sedtest.bat

and the TC 2.0 integrated environment related files:
  tcconfig.tc
  sed.prj

Feel free to expand our test files in any way you desire.
I wish we had a full suite of sed tests.

David Kirschbaum
Toad Hall
kirsch@usasoc.soc.mil

========

v1.5, 2 Oct 91

Received more changes from Howard Helman, and posted them
all manually.  Then received his final version of the
source (HHSED14C.MSG) and started to go with that.

Then received his final FINAL version (HHSED15A.MSG)
and am going with that.

(There are lots of warning messages when compiling
under TC 2.0.  If you ignore them, it runs just fine.)

Edited his README file a bit to integrate everything,
add a nice historical intro, etc.

See sed14b.pch for his v1.4 -> v1.4b patches.

David Kirschbaum
Toad Hall
