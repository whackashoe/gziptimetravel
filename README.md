gziptimetravel
================

gziptimetravel is a simple \*nix utility to view and change the timestamps of gzip archives.

Installation
============

`git clone https://github.com/whackashoe/gziptimetravel.git`
`cd gziptimetravel`
`make`
`sudo make install`

Usage
=====

This will change the header of `input_b` to equal `input_a's`.
    gziptimetravel input_a.tar.gz | gziptimetravel -S input_b.tar.gz


Find out when the gzip archive was created:

    gziptimetravel -p input_a.tar.gz

Set the timestamp of all files to '0':

    find *.gz | xargs gziptimetravel -s0
