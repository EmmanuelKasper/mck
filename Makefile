# this make file needs bsd make
MAKE = pmake
# needs ksh or bash for this makefile, as we do brace expansion
.SHELL: name=ksh
SHELL = /bin/ksh

.PHONY: help text xaaes single install-card test clean cleanall

# 4 partitions of 127MB, as larger partitions create a logical sector size > 4096 bytes, that
# linux cannot grok, see https://git.io/JTKC2
DISK_IMAGE = ahdi-512M.img
VERSION = 1.3
CC = m68k-atari-mint-gcc

# Needed for GEMDOS fat compatibility
export MTOOLS_SKIP_CHECK = 1
export MTOOLS_NO_VFAT =1 

all: xaaes

help:
	@echo "Available targets:"
	@echo "text: build a text only distribution, jumping to a mksh shell after boot"
	@echo "single: build a graphical distribution, using the TOS ROM single task AES"
	@echo "xaaes: built a graphical distribution, using XaAES and Teradesk"
	@echo "install-card: copy the build distribution into a disk image"
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
	ansible-playbook playbook-aes-tos.yml --inventory localhost,

text: mksh/mksh csed/sed minix/commands/term/term
	ansible-playbook playbook-text-only.yml --inventory localhost,

xaaes: mksh/mksh csed/sed minix/commands/term/term
	ansible-playbook playbook-xaaes.yml --inventory localhost,

$(DISK_IMAGE):
	unzip resources/$(DISK_IMAGE).zip

install-card: $(DISK_IMAGE)
# first partition start at 1024th byte
# verify with `parted $(DISK_IMAGE) -- unit b print`
	-mdeltree -i $(DISK_IMAGE)@@1024 ::{auto,extra,mint}
	-mdeltree -i $(DISK_IMAGE)@@133170176 ::demodata
	-mcopy -s -i $(DISK_IMAGE)@@1024 build/* ::
	-mcopy -s -i $(DISK_IMAGE)@@133170176 demodata ::

mount-copy:
	mount /stmint
	rm -fr /stmint/{auto,extra,mint}
	cp -r build/{auto,extra,mint} /stmint
#	for dir in auto extra mint; do \
#	rsync --archive --verbose build/$$dir /stmint/$$dir; done
	umount /stmint

test: $(DISK_IMAGE)
	hatari --mono --gemdos-drive skip --acsi $(DISK_IMAGE) \
		--conout 2 2> /tmp/hatari.log

upgrade-test:
	./mint-upgrade build/ ~/Projects/emul/atari/freemint_partition
	@echo
	ls /home/manu/Projects/emul/atari/freemint_partition/{AUTO,MINT}

clean:
	rm -f $(DISK_IMAGE)
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
	-rm st_mint-*.*.img.zip
	$(MAKE) -C csed clean distclean
	$(MAKE) -C minix/commands clean

release:
	cp $(DISK_IMAGE) st_mint-$(VERSION).img
	zip st_mint-$(VERSION).img.zip st_mint-$(VERSION).img
	scp st_mint-$(VERSION).img.zip $${USER}@ada:/srv/www/subsole.org/static/retrocomputing
	rm st_mint-$(VERSION).img

include make.mk
