# need at leash ksh for this makefile, as we do brace expansion
# bsd make
.SHELL: name=ksh
# gnu make
SHELL = ksh

.PHONY: clean cleanall install test

IMAGE_MOUNT_POINT = /mnt/loop0
DISK_IMAGE = ahdi-64M.img
VERSION = 1.0

all: xa

sash-3.8/sash:
	# needs GNU Make
	make -C sash-3.8 TARGET_OS=FreeMINT CC=m68k-atari-mint-gcc

solo:
	ansible-playbook solomint.yml --inventory localhost,

text: sash-3.8/sash
	ansible-playbook textmint.yml --inventory localhost,

xa: sash-3.8/sash
	ansible-playbook xamint.yml --inventory localhost,

install:
	./helpers/mount-image.py $(DISK_IMAGE) 0 > /dev/null 2>&1
	mount $(IMAGE_MOUNT_POINT)
	rm -fr $(IMAGE_MOUNT_POINT)/{auto,extra,mint}
	(cd build; tar cf - *) | (cd $(IMAGE_MOUNT_POINT); tar xf -)
	umount $(IMAGE_MOUNT_POINT)
	udisksctl loop-delete --block-device $$(losetup --list --noheadings --output NAME  --associated $(DISK_IMAGE))

cardinstall:
	mount /stmint
	rm -fr /stmint/{auto,extra,mint}
	cp -r build/{auto,extra,mint} /stmint
#	for dir in auto extra mint; do \
#	rsync --archive --verbose build/$$dir /stmint/$$dir; done
	umount /stmint

test:
	hatari --acsi $(DISK_IMAGE) &

clean:
	rm -fr OKAMI_SH.ELL
	rm -fr build/*
	rm -fr freemint
	rm -f *.retry

cleanall: clean
	rm -f resources/disk00 resources/command.tos
	rm -fr resources/2048_13b_68k.zip resources/2048.68K
	rm -fr resources/etherne.zip resources/etherne
	rm -f freemint-1.18.0.tar.bz2 freemint-1-19-*-000-st_ste.zip
	make -C sash-3.8 clean

release:
	cp ahdi-64M.img st_mint-$(VERSION).img
	zip st_mint-$(VERSION).img.zip st_mint-$(VERSION).img
	scp st_mint-$(VERSION).img.zip manu@ada:/srv/www/subsole.org/static/retrocomputing
	rm st_mint-$(VERSION).img

include make.mk
