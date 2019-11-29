# need at leash ksh for this makefile, as we do brace expansion
# bsd make
.SHELL: name=ksh
# gnu make
SHELL = ksh

.PHONY: clean cleanall install test

IMAGE_MOUNT_POINT = /mnt/loop0
DISK_IMAGE = ahdi-64M.img

all: xa

solo:
	ansible-playbook solomint.yml --inventory localhost,

text:
	ansible-playbook textmint.yml --inventory localhost,

xa:
	ansible-playbook xamint.yml --inventory localhost,

install:
	./helpers/mount-image.py $(DISK_IMAGE) > /dev/null 2>&1
	mount $(IMAGE_MOUNT_POINT)
	rm -fr $(IMAGE_MOUNT_POINT)/{auto,extra,mint}
	(cd build; tar cf - *) | (cd $(IMAGE_MOUNT_POINT); tar xf -)
	umount $(IMAGE_MOUNT_POINT)
	udisksctl loop-delete --block-device $$(losetup --list --noheadings --output NAME  --associated $(DISK_IMAGE))

cardinstall:
	mount /stmint
	rm -fr /stmint/auto /stmint/mint
	cp -r build/auto build/mint /stmint
	# rsync --archive --verbose build/auto /stmint/auto
	# rsync --archive --verbose build/extra /stmint/extra
	# rsync --archive --verbose build/mint /stmint/mint
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
	rm -fr etherne.zip etherne/
	rm -f freemint-1.18.0.tar.bz2 freemint-1-19-*-000-st_ste.zip

release:
	cp card.img st_mint-0.8.img
	zip st_mint-0.8.img.zip st_mint-0.8.img
	scp st_mint-0.8.img.zip manu@ada:/srv/www/subsole.org/static/retrocomputing/
	rm st_mint-0.8.img card.img

include make.mk
