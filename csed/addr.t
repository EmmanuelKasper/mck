#
# testing addresses
#

=IN
one
two
three
four
.

=OUT
one
2
two
3
three
4
four
.

=SED 1n;=

=OUT
one
2
two
three
4
four
.

=SED 1,2n;=

=OUT
one
two
.

=SED 2q

=OUT one
=SED /two/,/two/d
=SED /two/,/one/d
=SED /two/,/four/d
=SED /two/,4d
=SED /two/,$d
=SED /two/,5d
=SED 2,/one/d
=SED 2,/two/d
=SED 2,/four/d
=SED 2,4d
=SED 2,$d
=SED 2,5d


=OUT
one
four
=SED /two/,/three/d
=SED /two/,3d
=SED 2,/three/d
=SED 2,3d

=OUT
one
three
four
.

=SED /two/d
=SED /two/,1d
=SED /two/,2d
=SED 2d
=SED 2,1d
=SED 2,2d

=OUT
one
two
three
.

=SED $d
=SED $,1d
=SED $,$d
=SED $,/one/d
=SED $,/four/d
=SED /four/d
=SED /four/,1d
=SED /four/,$d
=SED /four/,/one/d
=SED /four/,/four/d

=OUT
bar
two
three
four
.

=SED 
1c\
bar
=SED 
/one/c\
bar
=SED 
1,1c\
bar
=SED 
/one/,1c\
bar
.

=OUT
bar
.

=SED 
1,/four/c\
bar
.

OUT
.

=SED 
1,/one/c\
bar
=SED 
/one/,/one/c\
bar
=SED 
1,$c\
bar
.

#
# c\ perturbated
#

=OUT
one
three
bar
.

=SED
3n
2,3c\
bar
=SED
3b
2,3c\
bar
=SED
3b
2,4c\
bar
=SED
3!{
  2,3c\
bar
}
=SED
3!{
  2,4c\
bar
}
.

=OUT
one
three
four
.

=SED
3,$b
2,3c\
bar
=SED
3n
$n
2,3c\
bar
.

#
# grouping
#
# previous version used to output lines 1,7,8 only.

=IN
1 begin
2 .     skip
3 end   skip
4 begin skip
5 .
6 end
7 begin
8 end
.

=OUT
1 begin
5 .
6 end
7 begin
8 end
.

=SED
/skip/!{
  /begin/,/end/p
}
d
.


