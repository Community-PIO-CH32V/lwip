/* lwIP's compiler/platform layer.
 *
 * Deliberately minimal. lwIP's own headers supply the integer types from
 * <stdint.h>, so all that is left is what it cannot infer.
 */
#pragma once

#include <assert.h>

/* Diagnostics are compiled out.
 *
 * lwIP's debug output goes through printf, and on this core printf reaches
 * whatever `Serial` is -- which is a USB CDC endpoint. An assert firing inside
 * the stack, potentially from the Ethernet interrupt, would then try to push a
 * string through TinyUSB from interrupt context while the main loop may be
 * inside the same code. Silence is the safer default; point these at
 * ch32h4_console_puts (a raw, unbuffered UART) when chasing a stack bug,
 * because that one is safe to call from anywhere. */
#define LWIP_PLATFORM_DIAG(x)

/* Define CH32H4_LWIP_ASSERT_CONSOLE to route lwIP's asserts to the raw UART,
 * exactly as the note above suggests. Off by default for the reason given
 * there; on, it costs the assert strings in flash and nothing else.
 *
 * The return address is what makes it worth having: most of lwIP's assert
 * messages appear at several call sites, and the message alone often does not
 * say which one fired. */
#ifdef CH32H4_LWIP_ASSERT_CONSOLE
#include "ch32h4_console.h"
#define LWIP_PLATFORM_ASSERT(x)                                     \
    do {                                                            \
        ch32h4_console_puts("\nLWIP ASSERT: ");                      \
        ch32h4_console_puts(x);                                      \
        ch32h4_console_puts("\n  ra=");                              \
        ch32h4_console_puthex((uint32_t)(uintptr_t)                  \
                              __builtin_return_address(0));         \
        ch32h4_console_puts("\n");                                   \
        ch32h4_console_flush();                                     \
    } while (0)
#else
#define LWIP_PLATFORM_ASSERT(x)   do { } while (0)
#endif

/* lwIP's own ctype is smaller than newlib's and drags in no locale tables. */
#define LWIP_NO_CTYPE_H 1
