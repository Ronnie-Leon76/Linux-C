#include "trace.h"

const char *
icmpcode_v4(int code)
{
    static char errbuf[100];
    switch (code)
    {
    case 0:
        return ("network unreachable");
    case 1:
        return ("host unreachable");
    case 2:
        return ("protocol unreachable");
    case 3:
        return ("port unreachable");
    case 4:
        return ("fragmentation required but DF bit set");
    case 5:
        return ("source route failed");
    case 6:
        return ("destination network unknown");
    case 7:
        return ("destination host unknown");
    case 8:
        return ("source host isolated (obsolete)");
    case 9:
        return ("destination network administratively prohibited");
    case 10:
        return ("destination host administratively prohibited");
    case 11:
        return ("network unreachable for TOS");
    case 12:
        return ("host unreachable for TOS");
    case 13:
        return ("communication administratively prohibited by filtering");
    case 14:
        return ("host recedence violation");
    case 15:
        return ("precedence cutoff in effect");
    default:
        sprintf(errbuf, "[unknown code %d]", code);
        return errbuf;
    }
}

// extern int gotalarm;

int recv_v4(int seq, struct timeval *tv)
{
    int hlen1, hlen2, icmplen, ret;
    socklen_t len;
    ssize_t n;
    struct ip *ip, *hip;
    struct icmp *icmp;
    struct udphdr *udp;
    int gotalarm = 0;
    alarm(3);
    for (;;)
    {
        if (gotalarm)
            return (-3); /* alarm expried */
        len = pr->salen;
        n = recvfrom(recvfd, recvbuf, sizeof(recvbuf), 0, pr->sarecv, &len);
        if (n < 0)
        {
            if (errno == EINTR)
                continue;
            else
                //err_sys("recvfrom error");
                exit(1);
        }

        ip = (struct ip *)recvbuf; /* start of IP header */
        hlen1 = ip->ip_hl << 2;    /* length of IP header */

        icmp = (struct icmp *)(recvbuf + hlen1); /* start of ICMP header */
        if ((icmplen = n - hlen1) < 8)
            continue; /* not enough to look at ICMP header */
        if (icmp->icmp_type == ICMP_TIMXCEED &&
            icmp->icmp_code == ICMP_TIMXCEED_INTRANS)
        {
            if (icmplen < 8 + sizeof(struct ip))
                continue; /* not enough data to look at inner IP */
            hip = (struct ip *)(recvbuf + hlen1 + 8);
            hlen2 = hip->ip_hl << 2;
            if (icmplen < 8 + hlen2 + 4)
                continue; /* not enough data to look at UDP ports */
            udp = (struct udphdr *)(recvbuf + hlen1 + 8 + hlen2);
            if (hip->ip_p == IPPROTO_UDP &&
                udp->uh_sport == htons(sport) &&
                udp->uh_dport == htons(dport + seq))
            {
                ret = -2; /* we hit an intermediate router */
                break;
            }
        }
        else if (icmp->icmp_type == ICMP_UNREACH)
        {
            if (icmplen < 8 + sizeof(struct ip))
                continue; /* not enough data to look at inner IP */

            hip = (struct ip *)(recvbuf + hlen1 + 8);
            hlen2 = hip->ip_hl << 2;
            if (icmplen < 8 + hlen2 + 4)
                continue; /* not enough data to look at UDP ports */

            udp = (struct udphdr *)(recvbuf + hlen1 + 8 + hlen2);
            if (hip->ip_p == IPPROTO_UDP &&
                udp->uh_sport == htons(sport) &&
                udp->uh_dport == htons(dport + seq))
            {
                if (icmp->icmp_code == ICMP_UNREACH_PORT)
                    ret = -1; /* have reached destination */
                else
                    ret = icmp->icmp_code; /* 0, 1, 2, ... */
                break;
            }
        }
        if (verbose)
        {
            printf(" (type = %d, code = %d)\n",
                   // Sock_ntop_host(pr->sarecv, pr->salen),
                   icmp->icmp_type, icmp->icmp_code);
        }
        /* Some other ICMP error, recvfrom() again */
    }
    alarm(0);               /* don't leave alarm running */
    Gettimeofday(tv, NULL); /* get time of packet arrival */
    return (ret);
}

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
    void sig_alrm(int signo)
    {
        signal(SIGALRM, sig_alrm);
    }

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

    void traceloop(void)
    {
        traceloop();
    }

    exit(0);
}