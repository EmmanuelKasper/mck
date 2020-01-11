#
# test handling of 8bit chars and the l command
#

echo $sed '-f tmp.sed tmp.in > tmp.result 2>&1
if ! cmp -s tmp.out tmp.result
then
  echo ---- test failed when running sed script:
  escape -c% tmp.sed
  echo -- on following data:
  escape -c% tmp.in
  echo -- expected:
  escape -c% tmp.out
  echo -- but got:
  escape -c% tmp.result
  echo --
fi' > tmp.try

echo '\1\2\3\4\5\6\7
\10\11\12\13\14\15\16\17
\20\21\22\23\24\25\26\27
\30\31\32\33\34\35\36\37
 \\$$
\330\331\332\333\334\335\336\337' | escape -u > tmp.in


echo '\001\002\003\004\005\006\a$
\b\t$
\v\f\r\016\017$
\020\021\022\023\024\025\026\027$
\030\031\032\033\034\035\036\037$
 \\$$$
\330\331\332\333\334\335\336\337$' > tmp.out

echo 'l;d' > tmp.sed
. tmp.try

echo 'foo' > tmp.in
echo 'foo\nfoo$' > tmp.out
echo 'h;G;l;d' > tmp.sed
. tmp.try 


echo '%x01%x02%x03%x04abcdefbcdef%x7e%x7f%x80%x81%x82
%x83%x84%x85%x86%x87%x88%b%x89%x8a%x8b%x8c%x8d%x8e%x8f
%xfe%xff' | escape -uc% > tmp.in

echo 's/%x01%x02/foo/p
s/bcde/%a%b%f%r%t%v/g
y/%b%f/AZ/
s/[%x84-%x8c]\{1,\}/x/g
s/^..$/&&&/' | escape -uc% > tmp.sed

echo 'foo%x03%x04abcdefbcdef%x7e%x7f%x80%x81%x82
foo%x03%x04a%aAZ%r%t%vf%aAZ%r%t%vf%x7e%x7f%x80%x81%x82
%x83xAx%x8d%x8e%x8f
%xfe%xff%xfe%xff%xfe%xff' | escape -uc% > tmp.out

. tmp.try

# \x80 not handled by hhsed
echo 's/foo/%x800/' | escape -uc% > tmp.sed
echo 'foo' > tmp.in
echo '%x800' | escape -uc% > tmp.out
. tmp.try



