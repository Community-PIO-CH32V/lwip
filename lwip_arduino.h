/* The header that makes lwIP findable.
 *
 * NOT an API. Including this is how a library says "I need lwIP" in a way
 * arduino-cli's dependency resolver can see: it matches an #include against
 * the headers at the top of a library's src/ and nothing deeper, and every
 * real lwIP header is "lwip/something.h". Without a header at this level,
 * nothing can ever name lwIP, and a sketch that includes LwipEthernet.h fails
 * on "lwip/netif.h: No such file or directory" -- which reads like a broken
 * include path rather than a library that was never added to the build.
 *
 * lwIP_Ethernet includes it for exactly that reason. Sketches want
 * <LwipEthernet.h>, not this.
 */
#pragma once

#include "lwip/init.h"
#include "lwip/opt.h"
