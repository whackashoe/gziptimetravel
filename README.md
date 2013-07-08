gziptimetravel
================

gziptimetravel is a simple \*nix utility to view and change the timestamps of gzip archives.

Installation
============

* `git clone https://github.com/whackashoe/gziptimetravel.git`

* `cd gziptimetravel`

* `cmake .`

* `make`

* `sudo make install`

Usage
=====

This will change the header of `input_b` to equal `input_a's`.

```gziptimetravel input_a.tar.gz | gziptimetravel -S input_b.tar.gz```


Find out when the gzip archive was created:

```gziptimetravel -p input_a.tar.gz```

Set the timestamp of all files to '0':

```gziptimetravel -s0 *.gz```

Set the modification time of the file to the mtime header value, while setting header value to 0, pretty printing the time, and printing out the name of the file:

```gziptimetravel -npms0 *.gz```