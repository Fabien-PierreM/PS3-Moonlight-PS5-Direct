#ifndef CHIAKI_PS3_NETDB_H
#define CHIAKI_PS3_NETDB_H
#include <sys/socket.h>
#include <netinet/in.h>
#include <stddef.h>

struct addrinfo {
    int ai_flags;
    int ai_family;
    int ai_socktype;
    int ai_protocol;
    socklen_t ai_addrlen;
    struct sockaddr *ai_addr;
    char *ai_canonname;
    struct addrinfo *ai_next;
};

#define AI_PASSIVE 0x0001
#define NI_NUMERICHOST 0x0002
#define EAI_FAIL 1
#define EAI_FAMILY 2
#define EAI_NONAME 3

int getaddrinfo(const char *, const char *, const struct addrinfo *, struct addrinfo **);
void freeaddrinfo(struct addrinfo *);
int getnameinfo(const struct sockaddr *, socklen_t, char *, socklen_t, char *, socklen_t, int);
const char *gai_strerror(int);
#endif
