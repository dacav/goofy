# Goofy - The Gopher Server

Goofy is a [Gopher][rfc1436] server written in C++11 with [libevent][]

[rfc1436]: <https://tools.ietf.org/html/rfc1436>
[libevent]: <http://libevent.org/>

## Current state (March 2017)

Goofy is in development. While it is technically usable, it currently lacks
configuration and everything that would make it a daemon.  This is a
spare-time project after all :)

Consider everything as experimental, and feel free to play with it.
Comments, suggestions and patches are welcome.

## Compilation

Goofy is not using a serious build system, yet. The build is based ona
simple general purpose makefile. In order to use it, just run `make` in the
`src` directory.

I plan to use GNU Autotools or CMake for this purpose.

## Dependencies

Goofy depends only on Libevent2.

## How to use it

Currently Goofy looks for a file named `root.gophermap` in the run
directory. See [this][gophermap]. The file can point to other gophermap
files, to directories and supports the `URL:` syntax.

The support is not considered stable yet (in particular, I need to verify it
works exactly as other gopher servers).

Since it doesn't have a configuration system, yet, it only runs on port 7070
(the choice is handy for experimentation).

[gophermap]: <https://en.wikipedia.org/wiki/Gophermap>

## Architecture

Goofy is single threaded and uses an asynchronous semantics based on
LibEvent.
