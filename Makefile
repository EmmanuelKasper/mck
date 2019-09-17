.PHONY: clean cleanall install ansible

MOUNT_POINT = /media/manu/P1

text:
	ansible-playbook textmint.yml --inventory localhost,

xa:
	ansible-playbook xamint.yml --inventory localhost,

solo:
	ansible-playbook solomint.yml --inventory localhost,

install:
	cp -rv build/* $(MOUNT_POINT)

clean:
	rm -fr build
	rm -fr freemint

cleanall: clean
	rm -fr OKAMI_SH.ELL
	rm -f okamishl.zip
	rm -f freemint-1.18.0.tar.bz2

include make.mk
