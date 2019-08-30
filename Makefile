.PHONY: clean cleanall install ansible

MOUNT_POINT = target

ansible:
	ansible-playbook solomint.yml --inventory localhost,

install:
	cp -r build/* $(MOUNT_POINT)

clean:
	rm -fr build
	rm -fr freemint

cleanall: clean
	rm -f freemint-1.18.0.tar.bz2

include make.mk
