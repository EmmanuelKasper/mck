mk: build/mint/1-18-0/mint.toc

freemint-1.18.0.tar.bz2:
	wget https://github.com/freemint/freemint/releases/download/freemint-1_18_0/freemint-1.18.0.tar.bz2

freemint/auto/mint000.prg: freemint-1.18.0.tar.bz2
	mkdir -p freemint
	bzcat $? | tar -C freemint -xvf -
	# mark the mint file as newer as the originating zip
	touch $@ 

build/auto/mint000.prg: freemint/auto/mint000.prg
	mkdir -p build/auto
	cp $? $@

build/mint/1-18-0/mint.cnf: build/auto/mint000.prg
	mkdir -p build
	cp -r freemint/mint build/

build/mint/1-18-0/mint.toc: build/mint/1-18-0/mint.cnf
	cp $? $?.orig
	sed 's/setenv HOSTNAME saturn/setenv HOSTNAME stmint/' $?.orig > $?
	cp $? $?.orig
	sed 's/#GEM=ROM/GEM=ROM/' $?.orig > $?
	rm $?.orig
	touch $@

