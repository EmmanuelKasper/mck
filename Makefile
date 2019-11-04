.PHONY: clean cleanall install test

all: xa

solo:
	ansible-playbook solomint.yml --inventory localhost,

text:
	ansible-playbook textmint.yml --inventory localhost,

xa:
	ansible-playbook xamint.yml --inventory localhost,

imageinstall:
	udisksctl loop-setup --file 1GB.img
	udisksctl mount --block-device /dev/loop0p1
	rm -fr /media/manu/P1/*
	(cd build; tar cf - *) | (cd /media/manu/P1; tar xf -)
	udisksctl unmount --block-device /dev/loop0p1
	udisksctl loop-delete --block-device /dev/loop0

cardinstall:
	mount /stmint
	rm -fr /stmint/auto /stmint/extra /stmint/mint
	cp -r build/auto build/extra build/mint /stmint
	umount /stmint

test:
	hatari --acsi 1GB.img &

clean:
	rm -fr OKAMI_SH.ELL
	rm -fr build/*
	rm -fr freemint
	rm -f *.retry

cleanall: clean
	rm -f resources/disk00 resources/command.tos
	rm -fr resources/2048_13b_68k.zip resources/2048.68K
	rm -f freemint-1.18.0.tar.bz2 freemint-1-19-*-000-st_ste.zip

release:
	cp card.img st_mint-0.8.img
	zip st_mint-0.8.img.zip st_mint-0.8.img
	scp st_mint-0.8.img.zip manu@ada:/srv/www/subsole.org/static/retrocomputing/
	rm st_mint-0.8.img card.img

include make.mk
