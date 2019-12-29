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

To copy all the stuff in the first partition of a TOS/DOS disk image named 1GB.img, call

```
make install
```

To copy all the stuff in a partition mounted on /stmint, call
```
make cardinstall
```

## Roadmap / Status
see https://github.com/EmmanuelKasper/mck/wiki/Roadmap


