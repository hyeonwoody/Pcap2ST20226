
packet_OBJS := packet.o
fwriteTest_OBJS := fwriteTest.o
t_OBJS := t.o
CC := g++
CFLAGS:=-std=c++11 -I .
clean:
	rm -rf $(packet_OBJS) $(fwriteTest_OBJS) $(TARGET)
packet.o: packet.cpp packet.h  
	$(CC) -o $@ $(CFLAGS) -c $<
packet: $(packet_OBJS)  
	$(CC) pbPlots.cpp supportLib.cpp -o $@ $(packet_OBJS)

fwriteTest.o: fwriteTest.cpp
	$(CC) -o $@ $(CFLAGS) -c $<
fwriteTest: $(fwriteTest_OBJS)
	$(CC) -o $@ $(fwriteTest_OBJS)

t.o: t.cpp
	$(CC) -o $@ $(CFLAGS) -c $<
t: $(t_OBJS)
	$(CC) -o $@ $(t_OBJS)
TARGET = packet fwriteTest

run: packet
	rm -rf Debug.txt && ./packet $(ARGS)

ifeq ($(DEBUG),y)
CFLAGS+=-g -O0
ARGS+=>> Debug.txt
else
CFLAGS+=-O3
endif