# Makefile for small-sed

.SUFFIXES:
.SUFFIXES: .t .log .c .o .sh

DEFS = -DDEBUG
CFLAGS = -g -W -Wall -ansi --pedantic -O2 $(DEFS)

sed: sedcomp.o sedexec.o
	$(CC) $(CFLAGS) sedcomp.o sedexec.o -o $@

sedcomp.o sedexec.o: sed.h

.c.o: sedcomp.c
	$(CC) $(CFLAGS) -c $< -o $@

#
# test
#

TTESTS = buffers.log interval.log misc.log addr.log
SHTESTS = 8bit.log match.log
GNUTESTS = gnutests.log

test:  $(TTESTS) $(SHTESTS) $(GNUTESTS)

$(TTESTS) $(SHTESTS) $(GNUTESTS): sed
$(TTESTS) $(GNUTESTS): test.sed

# general test rules

.sh.log:
	@echo testing $<
	@sed=$${SED-./sed} sh -f $< 2>&1 > $@ || \
	(cat $@; exit 1)

.t.log:
	@echo testing $<
	@sed -f test.sed $< | sed=$${SED-./sed} sh 2>&1 > $@ || \
	(cat $@ ; exit 1)

# special cases

8bit.log: escape

escape: escape.o
	$(CC) $(CFLAGS) $< -o $@

match.log: match.txt


# 
# expand
#

expand:
	@for i in `grep -l '	' *.[ch]`; \
	do \
	  echo expanding $$i; \
	  expand <$$i >expand.tmp; \
	  mv expand.tmp $$i; \
	done

#
# indent
#

INDENTFILES = *.[ch]
INDENT = indent -kr -i2
	
indent:
	@err=0; \
	for i in $(INDENTFILES); do \
	  $(INDENT) <$$i 2>err.tmp | expand >indent.tmp; \
	  if ! test -s err.tmp; then \
	    if ! cmp -s indent.tmp $$i; then \
	      echo indenting $$i; \
	      mv $$i $$i~; \
	      mv indent.tmp $$i; \
	    fi \
	  else \
	    err=`expr $$err + 1`; \
	    echo in $$i:; \
	    cat err.tmp; \
	  fi \
	done; \
	rm -f err.tmp indent.tmp; \
	if [ $$err -ne 0 ]; then \
	  echo $$err 'file(s)' untouched because of warnings; \
	  false; \
	fi

#
# clean, distclean, tgz
#

clean:
	rm -f ? *~ */*~ *.o core *.stackdump a.out a.exe
	rm -f tmp.* *.tmp */tmp.* */*.tmp *.log

distclean: clean
	rm -f sed sed.exe escape escape.exe

tgz:	distclean
	@here=`pwd`;dir=`basename $$here`;\
	tgz=`echo $$dir|tr A-Z- a-z_`-`date +%y%m%d`.tgz;\
	echo "cd ..; tar cf - $$dir | gzip -c -9 > $$tgz";\
	cd ..; tar cf - $$dir | gzip -c -9 > $$tgz

