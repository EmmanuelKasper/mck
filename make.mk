mk: target/mint/1-18-0/mint.toc

freemint-1.18.0.tar.bz2:
	wget https://github.com/freemint/freemint/releases/download/freemint-1_18_0/freemint-1.18.0.tar.bz2

freemint/auto/mint000.prg: freemint-1.18.0.tar.bz2
	mkdir -p freemint
	bzcat $? | tar -C freemint -xvf -
	# mark the mint file as newer as the originating zip
	touch $@ 

target/auto/mint000.prg: freemint/auto/mint000.prg
	mkdir -p target/auto
	cp $? $@

target/mint/1-18-0/mint.cnf: target/auto/mint000.prg
	mkdir -p target
	cp -r freemint/mint target/

target/mint/1-18-0/mint.toc: target/mint/1-18-0/mint.cnf
	cp $? $?.orig
	sed 's/setenv HOSTNAME saturn/setenv HOSTNAME stmint/' $?.orig > $?
	cp $? $?.orig
	sed 's/#GEM=ROM/GEM=ROM/' $?.orig > $?
	rm $?.orig
	touch $@

