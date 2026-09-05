# lwIP 2.2.1, as an Arduino library for the CH32H41x core

This branch is [lwIP](https://github.com/lwip-tcpip/lwip) rearranged so that
[ArduinoCore-CH32H4](https://github.com/Community-PIO-CH32V/ArduinoCore-CH32H4)
can consume it as an ordinary Arduino library — pulled in by a sketch's
`#include`, with no build flag to remember.

The stack itself is unmodified. What changed:

**Pruned to what a NO_SYS=1 Ethernet port builds.** `api/` is the netconn and
socket layer and needs threads. `netif/ppp`, the SLIP and 6LoWPAN interfaces
have no hardware here. `contrib/`, `doc/` and `test/` are not source. Of
`apps/`, `sntp` and `mdns` are kept — the clock matters because certificate
validity is checked against it, and mDNS is wanted next — and the rest go.

They are deleted rather than left to their `#if` guards because arduino-cli
compiles every directory under a library's `src/` recursively, with no way to
exclude one. Most would have compiled to nothing; carrying them anyway is a
lot of source to read past for a board that will never enable them.

**Flattened.** An Arduino library gets one include root, `<lib>/src`. So
`src/include/lwip`, `src/include/compat` and `src/include/netif` moved to the
top level, and `src/core`, `src/netif` and `src/apps` came up beside them.
`netif/` holds the headers and the sources together, which is not a collision:
a `.c` and a `.h` with the same stem sit in one directory perfectly happily.

**The port headers live here.** lwIP ships no `lwipopts.h` and no
`arch/cc.h` — both are the port's to supply, and `lwip/opt.h` and
`lwip/arch.h` include them by those bare names. They are at the include root
now, which is the one place they can be without the build having to guarantee
that one `-I` comes before another.

## Updating

Rebase this branch onto the new upstream release and re-apply the prune. The
two port headers are ours and carry forward unchanged unless lwIP's options
change.
