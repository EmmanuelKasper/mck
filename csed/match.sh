
sed '1i\
failed=0
/^#/d
s/'\''/'\'\\\'\''/
/TRY  *\([^ ][^ ]*\)  *\([^ ][^ ]*\) *$/{
  s//\2 \1/
  h
  s/ .*//
  s/[<>]//g
  G
  i\
line=\\
  =
  /\(.*\)\n\(.*\) \(.*\)/{
    s,,#echo "echo '\''\1'\'' | '$sed' '\''s/\3/<\&>/'\''"\
out=`echo '\''\1'\'' | '$sed' '\''s/\3/<\&>/'\'' 2>/dev/null`\
if test "x$out" != '\''x\2'\''; then\
  failed=`expr $failed + 1`\
  echo line $line: '\''/\3/'\''"  \\"$out\\"  instead of  "'\''"\2"'\''\
fi,
  }
}
/FAIL  *\([^ ][^ ]*\) *$/{
  i\
line=\\
  =
  s,,out=`echo foo | '$sed' '\''/\1/h'\'' 2>/dev/null`\
if test "x$out" != "x"; then\
  failed=`expr $failed + 1`\
  echo '\''/\1/'\'' failed to provoke an error.\
fi,
}
$a\
if test $failed -ne 0; then\
  echo "$failed test(s) failed."\
  exit 1\
fi
' match.txt | sh

