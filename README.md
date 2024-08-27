MCK: The ST Mint Construction Kit
=================================

This project has NO MAINTENANCE INTENDED as project goals have been reached with ST Mint 1.4  

A proof of concept to build an Atari ST Freemint distribution using Ansible as a build tool.
Freemint is an Unix-like kernel for the Atari ST, see https://freemint.github.io/ for details

## Ready to use disk images for your Atari
See http://www.subsole.org/st_mint

## Dependencies
You will need a Linux system the following packages installed:
- ansible
- mtools
- bmake
from the Debian / Ubuntu archive

and
- cross-mint-essential
- zlib-m68k-atari-mint
from Vincent Rivière's m68k-atari-mint cross-tools.

A [Vagrant](https://www.vagrantup.com/) virtual machine dev environment is provided for quick bootstrap. To use
run
~~~
vagrant up
vagrant ssh
# in vm
cd /vagrant
~~~
and you are ready to build.

## Usage

First make sure you pulled the minix git submodule:

```
git submodule update --init
```

To build the distribution:

```
make
```

To create an Atari AHDI partitioned disk image with the distribution

```
make image
```

## Roadmap / Status
see https://github.com/EmmanuelKasper/mck/wiki/Roadmap


