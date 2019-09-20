.PHONY: clean cleanall install test
MOUNT_POINT = /media/manu/P1

text:
	ansible-playbook textmint.yml --inventory localhost,

xa:
	ansible-playbook xamint.yml --inventory localhost,

solo:
	ansible-playbook solomint.yml --inventory localhost,

install:
	udisksctl loop-setup --file 1GB.img
	udisksctl mount --block-device /dev/loop0p1
	(cd build; tar cf - *) | (cd $(MOUNT_POINT); tar xf - )
	udisksctl unmount --block-device /dev/loop0p1
	udisksctl loop-delete --block-device /dev/loop0

test:
	hatari --acsi 1GB.img &

clean:
	rm -fr build
	rm -fr freemint

cleanall: clean
	rm -fr OKAMI_SH.ELL
	rm -f okamishl.zip
	rm -f freemint-1.18.0.tar.bz2

include make.mk
