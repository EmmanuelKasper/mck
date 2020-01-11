# test.sed - run tests from files containing input, command and output.
# Test files contain line-entries, multi-line-entries, and comments. 
# - a line-entry matches /^=\([A-Z][A-Z]*\)  *\([^ ].*\)$/, where \1 is
#   the keyword and \2 is the contents of the entry.
# - a multiline entry starts with an empty keyword /^=\([A-Z][A-Z]*\) *$/
#   and its contents is the folowing lines up to a line matching /^\. *$/.
# - anything out of line-entries and multiline-entries is comments,
#   and may ony contain empty lines, or lines beginning with #.
# test.sed will put all entry contents in file tmp.KEYWORD;
# In addition, some keywords will perform actions.
#
# recognised keywords:
# - FILE: test that "sed -f `cat FILE` IN" produces OUT
# - SED: test that "sed -f SED IN" produces OUT
# - FAIL: test that "sed -f SED IN" produces an error
# - SH: tests that "sh -f SH" does not return a non-zero exit code

1i\
failed=0

/^=[A-Z][A-Z]*  */b keyword
/^=[A-Z][A-Z]* *$/b keyword
/^\. *$/{
  : keyword
  s/^=//
  s/^\([^ ]*\) *$/\1/
  x
  /./i\
END
  /^INFILE/{
    i\
cp `cat tmp.INFILE` tmp.IN
  }
  /^OUTFILE/{
    i\
cp `cat tmp.OUTFILE` tmp.OUT
  }
  /^FILE/{
    i\
cp `cat tmp.FILE` tmp.SED
    s/.*/SED/
  }
  /^SED/{
    i\
line=\\
    =
    i\
$sed -f tmp.SED tmp.IN 2> err.tmp > out.tmp\
if test -s err.tmp\
then\
  failed=`expr $failed + 1`\
  echo ---- test failed line $line: commands:\
  cat tmp.SED\
  echo -- provoked the following error:\
  cat err.tmp\
  echo --\
else\
  if ! cmp -s tmp.OUT out.tmp\
  then\
    failed=`expr $failed + 1`\
    echo ---- test failed line $line: running commands:\
    cat tmp.SED\
    echo -- on following input:\
    cat tmp.IN\
    echo -- yields output:\
    cat out.tmp\
    echo -- instead of intended:\
    cat tmp.OUT\
    echo --\
  fi\
fi
  }
  /^FAIL/{
    i\
line=\\
    =
    i\
$sed -f tmp.FAIL tmp.IN 2> err.tmp >/dev/null\
if ! test -s err.tmp\
then\
  failed=`expr $failed + 1`\
  echo ---- test failed line $line: running commands:\
  cat tmp.FAIL\
  echo -- on following input:\
  cat tmp.IN\
  echo -- should have provoked an error.\
fi
  }
  /^SH/{
    i\
line=\\
    =
    i\
if sh -f tmp.SH\
then\
  failed=`expr $failed + 1`\
  echo ---- test failed line $line: commands:\
  cat tmp.SH\
  echo --\
fi
  }
  s/.*//
  x
  /^\.$/{
    g
    d
  }
  / /G
  h 
  / /{
    s/^\([A-Z]*\)  *\(.*\)\(\n\)/cat <<'END' > tmp.\1\3\2/p
    s/.*/./
    b keyword
  }
  s/^[A-Z]*$/cat <<'END' > tmp.&/
  b
}

$a\
rm -f tmp.* *.tmp\
if test $failed -ne 0\
then\
  echo "$failed test(s) failed."\
  exit 1\
fi\

x
/^$/{
  # between entries
  x
  /^ *$/d
  /^#/d
  i\
line=\\
  =
  i\
echo "Syntax error line $line:"
  s/.*/echo '&'/
  q
}
x
