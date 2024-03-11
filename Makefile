pcap2ST20226_OBJS := pcap.o
packet_OBJS := packet.o
fwriteTest_OBJS := fwriteTest.o
t_OBJS := t.o
CC := g++
CFLAGS:=-std=c++11 -I . -O0

clean:
	rm -rf $(packet_OBJS) $(fwriteTest_OBJS) $(pcap2ST20226_OBJS) $(TARGET)

pcap.o: pcap2St20226.cpp
	$(CC) -c -o $@ $(CFLAGS) $<
pcap: $(pcap2ST20226_OBJS)
	$(CC) $(CFLAGS) -o $@ $(pcap2ST20226_OBJS) -lpcap

packet.o: packet.cpp packet.h  
	$(CC) -o $@ $(CFLAGS) -c $<
packet: $(packet_OBJS)  
	$(CC) -o $@ $(packet_OBJS)

fwriteTest.o: fwriteTest.cpp
	$(CC) -o $@ $(CFLAGS) -c $<
fwriteTest: $(fwriteTest_OBJS)
	$(CC) -o $@ $(fwriteTest_OBJS)

t.o: t.cpp
	$(CC) -o $@ $(CFLAGS) -c $<
t: $(t_OBJS)
	$(CC) -o $@ $(t_OBJS)
TARGET = packet fwriteTescap

run: packet
	rm -rf Debug.txt && ./packet $(ARGS)

ifeq ($(DEBUG),y)
CFLAGS+=-g -O0
ARGS+=>> Debug.txt
# else
# CFLAGS+=-O3
endif