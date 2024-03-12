#ifndef _IP_VERSION_H_
#define _IP_VERSION_H_

#define IPs1(v) #v
#define IPs(v)	IPs1(v)
#define	IPvs	IPs(IPv)
#define	make_v3a(a,b,c)	a ## b ## c
#define make_v3(a,b,c)	make_v3a(a, b, c)
#define make_v(a,c)	make_v3(a, IPv, c)

#endif
