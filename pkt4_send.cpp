// pkt4_send.c++
 
#include <hooks/hooks.h>
#include <dhcp/pkt4.h>
#include "library_common.h"
 
#include <string>
 
using namespace isc::dhcp;
using namespace isc::hooks;
using namespace std;
 
extern "C" {
 
// This callout is called at the "pkt4_send" hook.
int pkt4_send(CalloutHandle& handle) {
 
	// Obtain the hardware address of the "interesting" client.  We have to
	// use a try...catch block here because if the client was not interesting,
	// no information would be set and getArgument would thrown an exception.
	string hwaddr;
	Pkt4Ptr response4_ptr;
	try {
		handle.getContext("hwaddr", hwaddr);
 
		// getContext didn't throw so the client is interesting.  Get a pointer
		// to the reply.
		handle.getArgument("response4", response4_ptr);
 
		// Get the string form of the IP address.
		string ipaddr = response4_ptr->getYiaddr().toText();
 
		// Write the information to the log file.
		interesting << hwaddr << " " << ipaddr <<
			" Ci " << response4_ptr->getCiaddr().toText() <<
			" Gi " << response4_ptr->getGiaddr().toText() <<
			" Si " << response4_ptr->getSiaddr().toText() <<
			"\n";
 
		// ... and to guard against a crash, we'll flush the output stream.
		flush(interesting);
 
	} catch (const NoSuchCalloutContext&) {
		// No such element in the per-request context with the name "hwaddr".
		// This means that the request was not an interesting, so do nothing
		// and dismiss the exception.
	 }
 
	OptionPtr opt12 = response4_ptr->getOption(12);
	if (opt12)
		interesting << "opt12 " << opt12->toString() << ": " << opt12->toText() << "\n";
	else
		interesting << "no option 12\n";
	OptionPtr opt81 = response4_ptr->getOption(81);
	if (opt81)
		interesting << "opt81 " << opt81->toString() << ": " << opt81->toText() << "\n";
	else
		interesting << "no option 81\n";
	flush(interesting);

	return (0);
}
 
}
