example.so: load_unload.c++ pkt4_receive.c++ pkt4_send.c++ version.c++ library_common.h
	g++ -I /usr/include/kea -L /usr/lib/aarch64-linux-gnu -fpic -shared -o example.so \
		load_unload.c++ pkt4_receive.c++ pkt4_send.c++ version.c++ \
		-lkea-dhcpsrv -lkea-dhcp++ -lkea-hooks -lkea-log -lkea-util -lkea-exceptions
