all:	example.so

.cpp.o:
	g++ -c -I /usr/include/kea -fpic -o $@ $<

example.so: load_unload.o pkt_receive.o pkt_send.o version.o pkt_messages.o pkt_change_hostname_log.o multi_threading_compatible.o subnet.o
	g++ -L /usr/lib/aarch64-linux-gnu -fpic -shared -o $@ $^ \
		-lkea-dhcpsrv -lkea-dhcp++ -lkea-hooks -lkea-log -lkea-util -lkea-exceptions

version.o:	version.cpp
load_unload.o:	load_unload.cpp
pkt_receive.o:	pkt_receive.cpp pkt46_receive.cpp
pkt_send.o:	pkt_send.cpp pkt46_send.cpp
pkt_messages.o: pkt_messages.cpp
pkt_change_hostname_log.o: pkt_change_hostname_log.cpp
multi_threading_compatible.o: multi_threading_compatible.cpp
subnet.o:	subnet.cpp

load_unload.o pkt_receive.o pkt_send.o:	library_common.h
load_unload.o pkt_receive.o pkt_send.o pkt_messages.o pkt_change_hostname_log.o:	pkt_messages.h
load_unload.o pkt_receive.o:	subnet.h load_unload.h
subnet.o:	subnet.h

pkt_messages.h pkt_messages.cpp: pkt_messages.mes
	kea-msg-compiler -e cpp pkt_messages.mes

clean:
	rm -f *.o *.so

dist-clean:	clean
	rm -f pkt_messages.h pkt_messages.cpp
