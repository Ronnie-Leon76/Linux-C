#include "unp.h"
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/udp.h>

#define BUFSIZE 1500

struct rec
{                          /* of outgoing UDP data */
    u_short rec_seq;       /* sequence number */
    u_short rec_ttl;       /* TTL packet left with */
    struct timeval rec_tv; /* time packet left */
};

/* globals */
char recvbuf[BUFSIZE];
char sendbuf[BUFSIZE];

int datalen; /* # bytes of data following ICMP header */
char *host;
u_short sport, dport;
int nsent; /* add 1 for each sendto () */
pid_t pid; /* our PID */
int probe, nprobes;
int sendfd, recvfd; /* send on UDP sock, read on raw ICMP sock */
int ttl, max_ttl;
int verbose;

/* function prototypes */
//const char *icmpcode_v4(int code);
const char *icmpcode_v6(int code);
//int recv_v4(int seq, struct timeval *tv);
int recv_v6(int seq, struct timeval *tv);
void sig_alrm(int signo);
char *Sock_ntop_host(const struct sockaddr *sockaddr, socklen_t addrlen);
struct addrinfo *Host_serv (const char *hostname, const char *service, int family, int socktype);
int sock_cmp_addr(const struct sockaddr *sa1, const struct sockaddr *sa2, socklen_t salen);
void sock_set_port(struct sockaddr *sa, socklen_t salen, int port);
void traceloop(void);
void tv_sub(struct timeval *out, struct timeval *in);
struct proto
{
    const char *(*icmpcode)(int);
    int (*recv)(int, struct timeval *);
    struct sockaddr *sasend; /* sockaddr{} for send, from getaddrinfo */
    struct sockaddr *sarecv; /* sockaddr{} for receiving */
    struct sockaddr *salast; /* last sockaddr{} for receiving */
    struct sockaddr *sabind; /* sockaddr{} for binding source port */
    socklen_t salen;         /* length of sockaddr{}s */
    int icmpproto;           /* IPPROTO_xxx value for ICMP */
    int ttllevel;            /* setsockopt () level to set TTL */
    int ttloptname;          /* setsockopt () name to set TTL */
} * pr;

#ifdef IPV6

#include <netinet/ip6.h>
#include <netinet/icmp6.h>

#endif