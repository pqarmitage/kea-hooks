all:	example.so

.cpp.o:
	g++ -c -I /usr/include/kea -fpic -o $@ $<

example.so: load_unload.o pkt4_receive.o pkt4_send.o version.o pkt4_messages.o pkt4_change_hostname_log.o multi_threading_compatible.o
	g++ -L /usr/lib/aarch64-linux-gnu -fpic -shared -o $@ $^ \
		-lkea-dhcpsrv -lkea-dhcp++ -lkea-hooks -lkea-log -lkea-util -lkea-exceptions

version.o:	version.cpp
load_unload.o:	load_unload.cpp
pkt4_receive.o:	pkt4_receive.cpp
pkt4_send.o:	pkt4_send.cpp
pkt4_messages.o: pkt4_messages.cpp
pkt4_change_hostname_log.o: pkt4_change_hostname_log.cpp
multi_threading_compatible.o: multi_threading_compatible.cpp

load_unload.o pkt4_receive.o pkt4_send.o:	library_common.h
load_unload.o pkt4_receive.o pkt4_send.o pkt4_messages.o pkt4_change_hostname_log.o:	pkt4_messages.h

pkt4_messages.h pkt4_messages.cpp: pkt4_messages.mes
	kea-msg-compiler -e cpp pkt4_messages.mes

clean:
	rm -f *.o *.so
