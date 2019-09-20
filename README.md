MCK: The ST Mint Construction Kit
=================================

A proof of concept to build an Atari ST Freemint distribution using Ansible as a build tool.
Freemint is an Unix-like kernel for the Atari ST, see https://freemint.github.io/ for details

## Ready to use disk images for your Atari
See http://www.subsole.org/st_mint

## Usage

Having ansible and a POSIX make installed, call

```
make
```

To copy all the stuff in the first partition of a TOS/DOS disk image, call

```
make install
```

or
```
make MOUNT_POINT=/media/manu/P1 install
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