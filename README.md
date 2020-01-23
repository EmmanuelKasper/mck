MCK: The ST Mint Construction Kit
=================================

A proof of concept to build an Atari ST Freemint distribution using Ansible as a build tool.
Freemint is an Unix-like kernel for the Atari ST, see https://freemint.github.io/ for details

## Ready to use disk images for your Atari
See http://www.subsole.org/st_mint

## Dependencies
You will need the following packages installed:
- ansible
- mtools
- make
from the Debian / Ubuntu archive

and
- cross-mint-essential
- zlib-m68k-atari-mint
from Vincent Rivière's m68k-atari-mint cross-tools

## Usage

To build the distribution:

```
make
```

To copy all the stuff in the first partition of a TOS/DOS disk image named 1GB.img:

```
make install
```

To copy all the stuff in a partition mounted on /stmint, call
```
make cardinstall
```

## Roadmap / Status
see https://github.com/EmmanuelKasper/mck/wiki/Roadmap


