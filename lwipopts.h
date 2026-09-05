/* lwIP configuration.
 *
 * NO_SYS=1: no RTOS, no threads. The stack runs from the Ethernet interrupt
 * and from the sketch's loop, exactly as it does on the two prior ports to
 * this silicon, and every call into it must therefore come from one of those
 * two places -- never from both at once. LWIP_ASSERT_CORE_LOCKED is what turns
 * a violation of that into a message instead of a corrupted pbuf chain.
 */
#pragma once

#include <stdint.h>

#define NO_SYS                      1
#define LWIP_TIMERS                 1
#define SYS_LIGHTWEIGHT_PROT        0

/* The socket and netconn APIs need threads; the raw/callback API does not.
 * Arduino's Client/Server classes sit on the raw API. */
#define LWIP_SOCKET                 0
#define LWIP_NETCONN                0

#define LWIP_IPV4                   1
#define LWIP_IPV6                   0

#define LWIP_ARP                    1
#define LWIP_ETHERNET               1
#define LWIP_ICMP                   1
#define LWIP_RAW                    1
#define LWIP_UDP                    1
#define LWIP_TCP                    1
#define LWIP_DHCP                   1
#define LWIP_DNS                    1
#define LWIP_IGMP                   1

/* SO_REUSEADDR. A server that restarts must be able to re-bind its port while
 * the previous incarnation's connections are still in TIME_WAIT -- without
 * this, re-running a sketch fails to bind for a couple of minutes and looks
 * like a broken server rather than a working TCP stack. */
#define SO_REUSE                    1

/* Memory.
 *
 * These come out of the heap, which is ~700 KB here, so lwIP is not the thing
 * under pressure on this part -- unlike the MicroPython port, where it
 * competed with the GC. The numbers below are sized for a handful of
 * simultaneous connections rather than trimmed to a minimum.
 */
#define MEM_ALIGNMENT               4
#define MEM_SIZE                    (32 * 1024)

#define MEMP_NUM_PBUF               32
#define MEMP_NUM_UDP_PCB            8
#define MEMP_NUM_TCP_PCB            12
#define MEMP_NUM_TCP_PCB_LISTEN     8
#define MEMP_NUM_TCP_SEG            32
#define MEMP_NUM_REASSDATA          8
#define MEMP_NUM_ARP_QUEUE          8
#define MEMP_NUM_SYS_TIMEOUT        12

#define PBUF_POOL_SIZE              24
#define PBUF_POOL_BUFSIZE           1536

#define TCP_MSS                     1460
#define TCP_SND_BUF                 (8 * TCP_MSS)
#define TCP_SND_QUEUELEN            ((4 * TCP_SND_BUF) / TCP_MSS)
#define TCP_WND                     (8 * TCP_MSS)
#define TCP_QUEUE_OOSEQ             1
#define LWIP_TCP_KEEPALIVE          1

#define LWIP_NETIF_HOSTNAME         1
#define LWIP_NETIF_STATUS_CALLBACK  1
#define LWIP_NETIF_LINK_CALLBACK    1
#define LWIP_NETIF_TX_SINGLE_PBUF   1

/* Checksums are computed in software. The MAC on this part can offload them,
 * but the prior ports did not enable it and an incorrectly configured offload
 * produces frames a peer silently drops -- which is a poor trade against a few
 * microseconds on a 100 Mbit link. */
#define CHECKSUM_GEN_IP             1
#define CHECKSUM_GEN_UDP            1
#define CHECKSUM_GEN_TCP            1
#define CHECKSUM_CHECK_IP           1
#define CHECKSUM_CHECK_UDP          1
#define CHECKSUM_CHECK_TCP          1

#define LWIP_STATS                  0
#define LWIP_DEBUG                  0

/* Both cores can run code, and lwIP has no locking of its own under NO_SYS.
 * This turns "called from the wrong context" into an assert rather than a
 * corrupted pbuf chain found three hours later. */
#define LWIP_ASSERT_CORE_LOCKED()   ch32h4_lwip_assert_core_locked()

#ifndef __ASSEMBLER__
#ifdef __cplusplus
extern "C" {
#endif
void ch32h4_lwip_assert_core_locked(void);
unsigned long ch32h4_lwip_now_ms(void);
uint32_t ch32h4_lwip_rand(void);

/* SNTP hands the time here; see the SNTP block below. */
void ch32h4_sntp_set_time(uint32_t sec, uint32_t us);
#ifdef __cplusplus
}
#endif
#endif

/* ---- SNTP ---------------------------------------------------------------
 *
 * The clock the whole system hangs off. File timestamps come from it through
 * FatFs' get_fattime(), and TLS certificate validity is checked against it --
 * a board that thinks it is 2000 rejects every certificate ever issued, with
 * an error that says nothing about the clock.
 *
 * SNTP_SET_SYSTEM_TIME_US, not SNTP_SET_SYSTEM_TIME: the microsecond form is
 * what lwIP calls when it has the fraction, and taking it costs nothing. */
#define SNTP_SET_SYSTEM_TIME_US(sec, us)  ch32h4_sntp_set_time(sec, us)

/* Resolve pool.ntp.org and friends by name, which is what anyone configures.
 * Costs the DNS resolver, which is already on. */
#define SNTP_SERVER_DNS             1

/* Take the NTP servers DHCP offers, when it offers any. Many home routers do
 * and it is the least surprising default -- a board on a network with its own
 * time server should use it rather than reaching out to the public pool. Both
 * halves are needed: the DHCP client has to ask for the option, and SNTP has
 * to look at what came back. */
#define LWIP_DHCP_GET_NTP_SRV       1
#define SNTP_GET_SERVERS_FROM_DHCP  1
#define SNTP_MAX_SERVERS            2

/* Poll, rather than listen: this is a client on a small board, and the
 * unicast poll mode is the only one that works behind NAT anyway. */
#define SNTP_UPDATE_DELAY           3600000   /* an hour, the RFC's minimum */

/* lwIP's default checks the response's stratum and mode. Keep it: an answer
 * from a server advertising itself as unsynchronised is worse than no answer,
 * because it looks like success. */
#define SNTP_SUPPRESS_DELAY_CHECK   0


/* lwIP's timers need a millisecond clock. */
#define sys_now  ch32h4_lwip_now_ms

/* DNS transaction IDs and ephemeral ports need a source of randomness. Not
 * decorative: an off-path attacker who guesses the ID and source port can
 * answer a query before the real server. The bar is "not a counter", so this
 * is a xorshift32 seeded from the timebase -- NOT the hardware RNG, which is
 * the mbedTLS entropy source and must not be a hard dependency of DNS. */
#define LWIP_RAND()  ch32h4_lwip_rand()

/* ---- mDNS ---------------------------------------------------------------
 *
 * The responder, so the board answers to a name -- <hostname>.local -- and
 * advertises what it offers. That is how the Arduino IDE finds a board to
 * upload to over the network, and how anything else finds it without being
 * told an address that DHCP may change tomorrow.
 *
 * It needs IGMP, which is already on above: mDNS is multicast, and without a
 * group join the queries never arrive. mdns.c #errors rather than silently not
 * working, which is the right way round.
 *
 * MDNS_MAX_SERVICES is 2 rather than the default 1: OTA advertises _arduino
 * and a sketch will want one of its own. Each costs a little RAM per netif and
 * pushes the output packet from 512 to 1450 bytes, which is why it is not
 * larger.
 *
 * LWIP_NUM_NETIF_CLIENT_DATA is where mdns hangs its per-interface state.
 * Without it mdns_resp_add_netif() has nowhere to put anything and fails at
 * run time, having compiled perfectly. */
#define LWIP_MDNS_RESPONDER         1
#define MDNS_MAX_SERVICES           2
#define LWIP_NUM_NETIF_CLIENT_DATA  1

/* ---- Memory corruption detectors ----------------------------------------
 *
 * Off by default; -DCH32H4_LWIP_ASSERT_CONSOLE turns them on together with the
 * assert output in arch/cc.h.
 *
 * These exist because of a fault that only appeared at certain code layouts:
 * memp_free() indexing memp_pools[] thousands of entries out of range, so the
 * read landed in unprogrammed flash and returned the erased pattern. A wild
 * index that comes and goes with an unrelated codegen flag is memory
 * corruption somewhere upstream, and a symptom that hides when you look at it
 * is not something to chase directly.
 *
 * OVERFLOW_CHECK puts canaries either side of every pool element and every
 * heap block; at 2 it checks ALL of them on every alloc and free, which is
 * slow and is what makes it catch the write close to where it happened rather
 * than much later. SANITY_CHECK verifies the free lists are not looped or
 * cross-linked. */
#ifdef CH32H4_LWIP_ASSERT_CONSOLE
#define MEMP_OVERFLOW_CHECK         2
#define MEMP_SANITY_CHECK           1
#define MEM_OVERFLOW_CHECK          2
#define MEM_SANITY_CHECK            1
#endif
