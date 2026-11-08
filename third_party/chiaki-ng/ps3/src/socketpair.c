#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

int socketpair(int domain, int type, int protocol, int sv[2])
{
    (void)domain; (void)protocol;
    if(type != SOCK_STREAM && type != SOCK_DGRAM) return -1;
    int a=socket(AF_INET, SOCK_DGRAM, 0), b=socket(AF_INET, SOCK_DGRAM, 0);
    if(a<0 || b<0) return -1;
    struct sockaddr_in sa, sb; socklen_t n=sizeof(sa);
    memset(&sa,0,sizeof(sa)); sa.sin_family=AF_INET; sa.sin_addr.s_addr=htonl(INADDR_LOOPBACK); sa.sin_port=0;
    if(bind(a,(struct sockaddr*)&sa,sizeof(sa))<0 || getsockname(a,(struct sockaddr*)&sa,&n)<0) return -1;
    memset(&sb,0,sizeof(sb)); sb.sin_family=AF_INET; sb.sin_addr.s_addr=htonl(INADDR_LOOPBACK); sb.sin_port=0;
    n=sizeof(sb); if(bind(b,(struct sockaddr*)&sb,sizeof(sb))<0 || getsockname(b,(struct sockaddr*)&sb,&n)<0) return -1;
    if(connect(a,(struct sockaddr*)&sb,sizeof(sb))<0 || connect(b,(struct sockaddr*)&sa,sizeof(sa))<0) return -1;
    sv[0]=a; sv[1]=b; return 0;
}
