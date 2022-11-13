#include "./trace.h"

struct proto proto_v4 = {
    icmpcode_v4,
    recv_v4,
    NULL,
    NULL,
    NULL,
    NULL,
    0,
    IPPROTO_ICMP, IPPROTO_IP, IP_TTL};

#ifdef IPV6
struct proto proto_v6 = {icmpcode_v6, recv_v6, NULL, NULL, NULL, NULL, 0,
                         IPPROTO_ICMPV6, IPPROTO_IPV6, IPV6_UNICAST_HOPS};
#endif

int datalen = sizeof(struct rec); /* defaults */
int max_ttl = 30;
int nprobes = 3; // we send three probes per hop
u_short dport = 32768 + 666;

int main(int argc, char **argv)
{
    int c;
    struct addrinfo *ai;
    char *h;
    opterr = 0; /* don't want getopt () writing to stderr */

    /*
     *Provide the -m flag to let the user change the maximum number of hops
     *the traceroute will go through. The default is 30.
     *the flag -v option causes most received ICMP messages to be printed.
     */
    while ((c = getopt(argc, argv, "m:v")) != -1)
    {
        switch (c)
        {
        case 'm':
            if ((max_ttl = atoi(optarg)) <= 1)
                printf("invalid -m value");
            break;

        case 'v':
            verbose++;
            break;
        case '?':
            printf("unrecognized option: %c", c);
        }
    }
    if (optind != argc - 1)
        printf("usage: traceroute [ -m <maxttl> -v ] <hostname>");
    host = argv[optind];

    pid = getpid();
    signal(SIGALRM, sig_alrm);

    // destination hostname or IP address is processed by host_serv() function, returning a pointer to an addrinfo structures
    struct addrinfo *Host_serv(const char *hostname, const char *service, int family, int socktype)
    {
        ai = Host_serv(host, NULL, 0, 0);
    }
    char *Sock_ntop_host(const struct sockaddr *sockaddr, socklen_t addrlen)
    {
        h = Sock_ntop_host(ai->ai_addr, ai->ai_addrlen);
    }
    printf("traceroute to %s (%s) : %d hops max, %d data bytes\n", ai->ai_canonname ? ai->ai_canonname : h, h, max_ttl, datalen);

    /* initialize according to protocol */
    if (ai->ai_family == AF_INET)
    {
        pr = &proto_v4;
#ifdef IPV6
    }
    else if (ai->ai_family == AF_INET6)
    {
        pr = &proto_v6;
        if (IN6_IS_ADDR_V4MAPPED(&(((struct sockaddr_in6 *)ai->ai_addr)->sin6_addr)))
            err_quit("cannot traceroute IPv4-mapped IPv6 address");
#endif
    }
    else
        printf("unknown address family %d", ai->ai_family);

    pr->sasend = ai->ai_addr; /* contains destination address */
    pr->sarecv = calloc(1, ai->ai_addrlen);
    pr->salast = calloc(1, ai->ai_addrlen);
    pr->sabind = calloc(1, ai->ai_addrlen);
    pr->salen = ai->ai_addrlen;

    traceloop();

    exit(0);
}