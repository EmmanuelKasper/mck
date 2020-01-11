
#
# check that // is dynamically handled
#

=IN
foo
bar
OUT
<f>xx
bxr
SED
/f/b a
:b
s/[ao]/x/
:a
s//<&>/
//b b
.

#
# check that \1, \2 ... correctly allowed or disallowed

# allowed cases

=IN foo
=OUT foo:f
=SED s/.*\(f\).*/&:\1/
=SED /.*\(f\).*/s//&:\1/

=OUT foo:f
=SED s/.*\(f\)\(o\).*/&:\1/
=SED /.*\(f\)\(o\).*/s//&:\1/

=OUT foo:o
=SED s/.*\(f\)\(o\).*/&:\2/
=SED /.*\(f\)\(o\).*/ s//&:\2/

=OUT foo:o
=SED s/.*\(f\(o\)\2\).*/&:\2/
=SED /.*\(f\(o\)\2\).*/ s//&:\2/

=OUT foo:foo
=SED s/.*\(f\(o\)\2\).*/&:\1/
=SED /.*\(f\(o\)\2\).*/ s//&:\1/


# wrong cases
=FAIL s/./\1/
=FAIL /./ s//\1/
=FAIL s/\1//
=FAIL s/\(\1\)//
=FAIL s/\(\(\2\)\)//
=FAIL /\(.\)/ s//\2/

#
# test the t command
#

=IN foo
=OUT bar
=SED
# t should not branch before any subst
t a
s/.*/bar/p
:a
d
.

=SED
s/.*/bar/
# t should now branch
t a
s/.*/oops!/
:a
.

=SED
s/.*/bar/
s/X/Y/
s/Y/Z/
# t should branch even though the very last subst failed
t a
s/.*/oops!/
:a
.

=SED
s/.*/bar/
# t should now branch
t a
i\
oops!
:a
# after branching with t, t shouldn't branch
t b
q
:b
s/.*/ouch!/
.

=SED
s/.*/bar/
b a
i\
oops!
:a
# t should branch even after branching with b
t b
i\
oh, no!
:b
.

#
# test that i\ and a\ keep the indentation
#
=IN foo
=OUT
 \a\
  b\
foo
 b
 c
.
=SED
i\
 \\a\\\
  b\\
a\
 b\
 c
.

#
# test that labels are correctly parsed
#
=IN
a<space>
a
A<newline>p
Ap
=OUT
1
2
A
p
3
Ap
4
=SED
s/<space>/ /g
s/<newline>/\
/g
# a <space>
/a /ba 
/a/ba
/A\np/bA\
p
/Ap/bAp
i\
oops
q
 :a 
s/a.*/1/
 :a
s/a.*/2/
b
:A\
p
s/A.*/3/
: Ap
s/A.*/4/
.

#
# test \<newline> in RHS
#

=IN foo
=OUT
f<
>oo
=SED
s/f/&<\
>/
.

# test \n in translitteration

=IN foo
=OUT
f\n\n$
fuu
=SED
y/o/\n/
l
y/\n/u/
.

# test that \9 is recognised
=IN i,ii,iii,iv,v,vi,vii,viii,ix
=OUT ix,viii
=SED s/\(.*\),\(.*\),\(.*\),\(.*\),\(.*\),\(.*\),\(.*\),\(.*\),\(.*\)/\9,\8/

# test that line numbers are correctly handled
=IN
one
two
three
.

=OUT one
=SED 1q

=OUT
one
three
=SED 2d

=OUT 
1
one
=SED =;q



