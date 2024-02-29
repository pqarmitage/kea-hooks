all:	example.so

.cpp.o:
	g++ -c -I /usr/include/kea -fpic -o $@ $<

example.so: load_unload.o pkt4_receive.o pkt4_send.o version.o
	g++ -L /usr/lib/aarch64-linux-gnu -fpic -shared -o example.so \
		load_unload.o pkt4_receive.o pkt4_send.o version.o \
		-lkea-dhcpsrv -lkea-dhcp++ -lkea-hooks -lkea-log -lkea-util -lkea-exceptions

version.o:	version.cpp
load_unload.o:	load_unload.cpp
pkt4_receive.o:	pkt4_receive.cpp
pkt4_send.o:	pkt4_send.cpp

load_unload.o pkt4_receive.o pkt4_send.o:	library_common.h

clean:
	rm -f *.o *.so
