import datetime
import socket
import sys


def traceroute(hostname_or_address, max_hops, timeout, port_number = None):
    dest_addr = socket.gethostbyname(hostname_or_address)
    proto_icmp = socket.getprotobyname("icmp")
    proto_udp = socket.getprotobyname("udp")
    if port_number is None:
        port = 80 
    else:
        port = port_number

    for ttl in range(1, max_hops + 1):
        rx = socket.socket(socket.AF_INET, socket.SOCK_RAW, proto_icmp)
        rx.settimeout(timeout)
        rx.bind(("", port))
        tx = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, proto_udp)
        tx.setsockopt(socket.SOL_IP, socket.IP_TTL, ttl)
        start = datetime.datetime.now()
        tx.sendto("".encode(), (dest_addr, port))

        try:
            _, curr_addr = rx.recvfrom(512)
            curr_addr = curr_addr[0]
                
                
        except socket.timeout:
            curr_addr = "*"
            
        finally:
            end = datetime.datetime.now()
            rx.close()
            tx.close()

        yield curr_addr, (end - start).microseconds

        if curr_addr == dest_addr:
            break


if __name__ == "__main__":
    max_hops, port, dest_name = None, None, None
    for j, arg in enumerate(sys.argv):
        if arg == '-m':
            max_hops = int(sys.argv[j + 1])
            
        elif arg == '-p':
            port = int(sys.argv[j + 1])
            
        elif arg == '-t':
            dest_name = sys.argv[j + 1]
            
        elif arg == '-h':
            print("Usage: traceroute.py [-m max_hops] [-p port] [-t dest_name]")
            sys.exit(0)
      
    if dest_name is None:
        print("Usage: traceroute.py [-m max_hops] [-p port] [-t dest_name]")
        sys.exit(1)
    else:
        print("traceroute to %s (%s), %d hops max" % (dest_name, socket.gethostbyname(dest_name), max_hops))
        for i,v in enumerate(traceroute(dest_name, max_hops, 2, port)):
            print("%d\t%s\t%d ms" % (i+1, v[0], v[1]/1000))      
    
     

    
   