# need at leash ksh for this makefile, as we do brace expansion
# bsd make
.SHELL: name=ksh
# gnu make
SHELL = ksh

.PHONY: clean cleanall install test solo text xa cardinstall demount

DISK_IMAGE = ahdi-128M.img
VERSION = 1.2
CC = m68k-atari-mint-gcc

all: xa

sash-3.8/sash:
	# sash with debian patches need GNU Make
	make -C sash-3.8 TARGET_OS=FreeMINT CC=$(CC)

mksh/mksh:
	cd mksh; \
	TARGET_OS=FreeMiNT CC=$(CC) CPPFLAGS="$$CPPFLAGS -DMKSH_SMALL" ./Build.sh
	m68k-atari-mint-size $@
	m68k-atari-mint-strip $@

csed/sed:
	make -C csed CC=$(CC)

solo:
	ansible-playbook solomint.yml --inventory localhost,

text: mksh/mksh csed/sed
	ansible-playbook textmint.yml --inventory localhost,

xa: mksh/mksh csed/sed
	ansible-playbook xamint.yml --inventory localhost,

$(DISK_IMAGE):
	unzip resources/$(DISK_IMAGE).zip

install: $(DISK_IMAGE) # first partition start at 1024th byte
	-mdeltree -i $(DISK_IMAGE)@@1024 ::{auto,extra,mint}
	-mcopy -s -i $(DISK_IMAGE)@@1024 build/* ::

cardinstall:
	mount /stmint
	rm -fr /stmint/{auto,extra,mint}
	cp -r build/{auto,extra,mint} /stmint
#	for dir in auto extra mint; do \
#	rsync --archive --verbose build/$$dir /stmint/$$dir; done
	umount /stmint

test:
	hatari --acsi $(DISK_IMAGE) 2> /tmp/hatari.log --conout 2

clean:
	rm -fr build/*
	rm -fr freemint
	rm -f *.retry

cleanall: clean
	rm -f resources/disk00 resources/command.tos
	rm -fr resources/2048_13b_68k.zip resources/2048.68K
	rm -fr resources/etherne.zip resources/etherne
	rm -fr resources/coreutils-8.21-mint-20131205-bin-mint-20131219.tar.bz2 resources/coreutils
	rm -f freemint-1.18.0.tar.bz2 freemint-1-19-*-000-st_ste.zip
	rm -f mksh/mksh
	rm -f $(DISK_IMAGE)
	make -C csed clean distclean
	make -C sash-3.8 clean

release:
	cp $(DISK_IMAGE) st_mint-$(VERSION).img
	zip st_mint-$(VERSION).img.zip st_mint-$(VERSION).img
	scp st_mint-$(VERSION).img.zip $${USER}@ada:/srv/www/subsole.org/static/retrocomputing
	rm st_mint-$(VERSION).img

include make.mk
