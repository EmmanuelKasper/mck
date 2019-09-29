.PHONY: clean cleanall install test
MOUNT_POINT = /media/manu/P1

all: xa

solo:
	ansible-playbook solomint.yml --inventory localhost,

text:
	ansible-playbook textmint.yml --inventory localhost,

xa:
	ansible-playbook xamint.yml --inventory localhost,

install:
	udisksctl loop-setup --file 1GB.img
	udisksctl mount --block-device /dev/loop0p1
	(cd build; tar cf - *) | (cd $(MOUNT_POINT); tar xf - )
	udisksctl unmount --block-device /dev/loop0p1
	udisksctl loop-delete --block-device /dev/loop0

test:
	hatari --acsi 1GB.img &

clean:
	rm -fr OKAMI_SH.ELL
	rm -fr build
	rm -fr freemint
	rm -f *.retry

cleanall: clean
	rm -f okamishl.zip
	rm -f freemint-1.18.0.tar.bz2 freemint-1-19-2a8-000-st_ste.zip

include make.mk
