#
# test buffers.
#

=IN foo
=OUT
foo

=SED G

=OUT

foo
=SED H;x

=OUT
bar
foo
=SED h;s/foo/bar/;G

=OUT
foo
bar
=SED h;s/foo/bar/;H;x

=OUT a
=SED
s/foo/a\
b/;P;d
.

=OUT
a
a
b
=SED 
s/foo/a\
b/;P
.

=OUT
a\nb$
b$
=SED
s/foo/a\
b/;l;D
.

=IN
foo
bar
=OUT
foo$
foo
bar$
=SED l;n;l;d

=OUT
foo$
foo\nbar$
=SED l;N;l;d


