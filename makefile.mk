
all: traceloop.o icmpcode_v6.o recv_v4.o recv_v6.o sig_alrm.o main

traceloop.o: traceloop.c
	gcc -c traceloop.c

icmpcode_v6.o: icmpcode_v6.c
	gcc -c icmpcode_v6.c

recv_v4.o: recv_v4.c
	gcc -c recv_v4.c

recv_v6.o: recv_v6.c
	gcc -c recv_v6.c

sig_alrm.o: sig_alrm.c
	gcc -c sig_alrm.c

main: trace.h traceloop.o icmpcode_v6.o recv_v4.o recv_v6.o sig_alrm.o main.c
	gcc -o main main.c traceloop.o icmpcode_v6.o recv_v4.o recv_v6.o sig_alrm.o

run: main
	./main

clean:
	rm -f *.o main

