# Minix 3 source code needs BSD make
MAKE = bmake
# needs ksh or bash for this makefile, as we do brace expansion in `make image`
SHELL = /bin/bash

.PHONY: help text xaaes single image test clean cleanall

# 4 partitions of 127MB, as larger partitions create a logical sector size > 4096 bytes, that
# linux cannot grok, see https://git.io/JTKC2
DISK_IMAGE = ahdi-512M.img
VERSION = 1.5
EMUTOS_VERSION = 1.3
CC = m68k-atari-mint-gcc

all: xaaes

help:
	@echo "Available targets:"
	@echo "text: build a text only distribution, jumping to a mksh shell after boot"
	@echo "single: build a graphical distribution, using the TOS ROM single task AES"
	@echo "xaaes: built a graphical distribution, using XaAES and Teradesk"
	@echo "image: copy the build distribution into a disk image"
	@echo "test: boot a disk image into hatari"

mksh/mksh:
	cd mksh; \
	TARGET_OS=FreeMiNT CC=$(CC) CPPFLAGS="$$CPPFLAGS -DMKSH_SMALL" ./Build.sh
	m68k-atari-mint-size $@
	m68k-atari-mint-strip $@

minix/commands/term/term:
	$(MAKE) -C minix/commands

csed/sed:
	$(MAKE) -C csed CC=$(CC)

single:
	ansible-playbook playbook-aes-tos.yml

text: mksh/mksh csed/sed minix/commands/term/term
	ansible-playbook playbook-text-only.yml

xaaes: mksh/mksh csed/sed minix/commands/term/term
	ansible-playbook playbook-xaaes.yml

$(DISK_IMAGE):
	unzip resources/$(DISK_IMAGE).zip

image: $(DISK_IMAGE)
# first partition start at 1024th byte
# verify with `parted $(DISK_IMAGE) -- unit b print`
# MTOOLS export are needed for GEMDOS fat compatibility
	-MTOOLS_SKIP_CHECK=1 MTOOLS_NO_VFAT=1 mdeltree -i $(DISK_IMAGE)@@1024 ::{auto,extra,mint}
	-MTOOLS_SKIP_CHECK=1 MTOOLS_NO_VFAT=1 mdeltree -i $(DISK_IMAGE)@@133170176 ::demodata
	-MTOOLS_SKIP_CHECK=1 MTOOLS_NO_VFAT=1 mcopy -s -i $(DISK_IMAGE)@@1024 build/* ::
	-MTOOLS_SKIP_CHECK=1 MTOOLS_NO_VFAT=1 mcopy -s -i $(DISK_IMAGE)@@133170176 demodata ::

mount-copy:
	mount /stmint
	rm -fr /stmint/{auto,extra,mint}
	cp -r build/{auto,extra,mint} /stmint
#	for dir in auto extra mint; do \
#	rsync --archive --verbose build/$$dir /stmint/$$dir; done
	umount /stmint

test: $(DISK_IMAGE) emutos-256k-$(EMUTOS_VERSION)/etos256us.img
	hatari --mono --gemdos-drive skip --acsi $(DISK_IMAGE)\
		--tos emutos-256k-$(EMUTOS_VERSION)/etos256us.img \
		--cpuclock 32 --memsize 4 --conout 2 2> /tmp/hatari.log

gemdos-test: emutos-256k-$(EMUTOS_VERSION)/etos256us.img
	hatari --tos emutos-256k-$(EMUTOS_VERSION)/etos256us.img \
		--cpuclock 32 --mono --memsize 4 --harddrive build

emutos-256k-$(EMUTOS_VERSION)/etos256us.img: emutos-256k-$(EMUTOS_VERSION).zip
	unzip emutos-256k-$(EMUTOS_VERSION).zip
	touch emutos-256k-$(EMUTOS_VERSION)/etos256us.img # ensure img is newer than zip

emutos-256k-$(EMUTOS_VERSION).zip:
	wget https://sourceforge.net/projects/emutos/files/emutos/$(EMUTOS_VERSION)/emutos-256k-$(EMUTOS_VERSION).zip/download -O emutos-256k-$(EMUTOS_VERSION).zip

upgrade-test:
	./mint-upgrade build/ ~/Projects/emul/atari/freemint_gemdos
	@echo
	ls ~/Projects/emul/atari/freemint_gemdos/{AUTO,MINT}

clean:
	rm -f $(DISK_IMAGE)
	rm -fr build/*
	rm -fr freemint
	rm -f *.retry

cleanall: clean
	rm -f resources/disk00 resources/command.tos
	rm -fr resources/2048_*.zip resources/2048.68K
	rm -fr resources/etherne.zip resources/etherne
	rm -fr resources/coreutils-*
	rm -fr resources/ncurses-* 
	rm -f freemint-1.18.0.tar.bz2 freemint-1-19-*-000-st_ste.zip
	rm -f mksh/mksh
	rm -f $(DISK_IMAGE)
	rm -f st_mint-*.*.img.zip
	rm -fr emutos-256k-$(EMUTOS_VERSION).zip emutos-256k-$(EMUTOS_VERSION)
	$(MAKE) -C csed clean distclean
	$(MAKE) -C minix/commands clean

release:
	cp $(DISK_IMAGE) st_mint-$(VERSION).img
	zip st_mint-$(VERSION).img.zip st_mint-$(VERSION).img
	scp st_mint-$(VERSION).img.zip $${USER}@ada:/srv/www/subsole.org/static/retrocomputing
	rm st_mint-$(VERSION).img
