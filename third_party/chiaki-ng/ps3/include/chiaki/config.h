// PS3 direct-client port configuration.
#ifndef CHIAKI_CONFIG_H
#define CHIAKI_CONFIG_H

#define CHIAKI_LIB_ENABLE_OPUS 0
#define CHIAKI_LIB_ENABLE_PI_DECODER 0
#define CHIAKI_LIB_ENABLE_FFMPEG_DECODER 0
#define CHIAKI_LIB_ENABLE_MBEDTLS 1

/* PSL1GHT exposes IPv4 sockets only; retain layout declarations so the
 * upstream headers compile while the LAN-first port rejects IPv6 at runtime. */
#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 46
#endif
#ifndef CHIAKI_PS3_IPV6_LAYOUT
#define CHIAKI_PS3_IPV6_LAYOUT 1
#include <netinet/in.h>
#include <netdb.h>
struct in6_addr { unsigned char s6_addr[16]; };
static const struct in6_addr in6addr_any = {{0}};
struct sockaddr_in6 {
	unsigned char sin6_len;
	sa_family_t sin6_family;
	in_port_t sin6_port;
	unsigned long sin6_flowinfo;
	struct in6_addr sin6_addr;
	unsigned long sin6_scope_id;
};
struct sockaddr_storage {
	sa_family_t ss_family;
	unsigned char __ss_pad[126];
};
#endif

/* The first PS3 target is LAN-only: no Remote Play over Internet/RUDP. */
#define CHIAKI_PS3_LOCAL_LAN_ONLY 1

#endif // CHIAKI_CONFIG_H
