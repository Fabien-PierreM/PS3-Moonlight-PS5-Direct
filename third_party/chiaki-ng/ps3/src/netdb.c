#include <netdb.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res)
{
    (void)service;
    if(!node || !res || (hints && hints->ai_family == AF_INET6)) return EAI_FAMILY;
    struct addrinfo *ai = calloc(1, sizeof(*ai));
    struct sockaddr_in *sin = calloc(1, sizeof(*sin));
    if(!ai || !sin) { free(ai); free(sin); return EAI_FAIL; }
    sin->sin_family = AF_INET;
    if(inet_aton(node, &sin->sin_addr) == 0) { free(ai); free(sin); return EAI_NONAME; }
    ai->ai_family = AF_INET;
    ai->ai_socktype = hints ? hints->ai_socktype : SOCK_DGRAM;
    ai->ai_protocol = hints ? hints->ai_protocol : 0;
    ai->ai_addrlen = sizeof(*sin);
    ai->ai_addr = (struct sockaddr *)sin;
    *res = ai;
    return 0;
}

void freeaddrinfo(struct addrinfo *ai)
{
    while(ai) { struct addrinfo *next = ai->ai_next; free(ai->ai_addr); free(ai->ai_canonname); free(ai); ai = next; }
}

int getnameinfo(const struct sockaddr *sa, socklen_t salen, char *host, socklen_t hostlen, char *serv, socklen_t servlen, int flags)
{
    (void)salen; (void)serv; (void)servlen; (void)flags;
    if(!sa || sa->sa_family != AF_INET || !host || hostlen == 0) return EAI_FAMILY;
    const struct sockaddr_in *sin = (const struct sockaddr_in *)sa;
    const char *p = inet_ntoa(sin->sin_addr);
    if(!p || strlen(p) + 1 > hostlen) return EAI_FAIL;
    strcpy(host, p); return 0;
}

const char *gai_strerror(int e)
{
    switch(e) { case EAI_FAMILY: return "address family unsupported"; case EAI_NONAME: return "name is not numeric IPv4"; default: return "address lookup failed"; }
}
