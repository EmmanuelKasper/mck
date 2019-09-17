MCK: The Mint Construction Kit
=================================

A proof of concept to build an Atari ST Mint distribution using Ansible as a build tool.

## Usage

Having ansible and a POSIX make installed, call

```
make
```

To install the distribution, mount the first partition of an Atari disk on $PWD/target
and call

```
make install
```

or
```
make MOUNT_POINT=/media/manu/P1 install
```

If working with disk images, you can mount the disk image with the command:
```
udisksctl loop-setup --file 1GB.img
udisksctl mount --block-device /dev/loop0p1
```
and unmount afterwards with
```
udisksctl unmount --block-device /dev/loop0p1
udisksctl loop-delete --block-device /dev/loop0
```

## ROADMAP

- rebuild solomint with latest freemint build
- try xaaes with latest freemint build
- try myaes with latest freemint build
- add okamishell, configure tos2win
- maybe: add demo programs to show multitasking capabilities
- try ethernet programm with freemint
- cross compile nweb
- cross compile suckless core tools