swapdown
========

Unloads swap by disabling and enabling swap in one pass.
You can use this program either with suid-bit or without it.


Requirements
------------

gcc, make and POSIX environment.


Installation
------------

```sh
$ make
$ make install    # Non-suid version
# or
$ make install-suid
```


Usage
-----

```sh
$ swapdown
# Then either wait till it completes or press Ctrl+C to stop
```
