MCK: The St Mint Construction Kit
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
udisksctl loop-setup --file my_disk.img
```

