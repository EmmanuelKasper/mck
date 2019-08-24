.PHONY: cleanall

all: freemint/auto/mint000.prg

freemint-1.18.0.tar.bz2:
	wget https://github.com/freemint/freemint/releases/download/freemint-1_18_0/freemint-1.18.0.tar.bz2

freemint/auto/mint000.prg: freemint-1.18.0.tar.bz2
	mkdir -p freemint
	bzcat $< | tar -C freemint -xvf -
	# archive files are newer than their content
	touch $@ 

cleanall:
	rm -fr freemint
	rm -f freemint-1.18.0.tar.bz2
