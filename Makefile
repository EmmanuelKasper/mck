.PHONY: clean cleanall install ansible

MOUNT_POINT = /stmint

ansible:
	ansible-playbook solomint.yml --inventory localhost,

install:
	cp -r target/* $(MOUNT_POINT)

clean:
	rm -fr target
	rm -fr freemint

cleanall: clean
	rm -f freemint-1.18.0.tar.bz2

include make.mk
