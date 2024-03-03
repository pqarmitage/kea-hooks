// pkt_receive4.c__
 
#include <hooks/hooks.h>
#include <dhcp/pkt4.h>
#include <dhcpsrv/srv_config.h>
#include "library_common.h"
 
#include <string>
 
using namespace isc::dhcp;
using namespace isc::hooks;
using namespace std;
 
extern "C" {
 
int ddns4_update(CalloutHandle& handle) {
	string hostname;
	string new_hostname;
	bool fwd_update = true;
	bool rev_update = false;
	Pkt4Ptr resp4;
	string ci_addr_str;
	DdnsParamsPtr ddns_params;
	Subnet4Ptr subnet;

	handle.getArgument("hostname", hostname);
	handle.getArgument("fwd-update", fwd_update);
	handle.getArgument("rev-update", rev_update);
	handle.getArgument("response4", resp4);
	handle.getArgument("ddns-params", ddns_params);
	handle.getArgument("subnet4", subnet);

	ci_addr_str = resp4->getCiaddr().toText();
	interesting << "hostname " << hostname << " fwd: " << fwd_update << " rev: " << rev_update << "\n";
	interesting << "   " << " ci " << ci_addr_str <<  " gi " << resp4->getGiaddr().toText() << " si " << resp4->getSiaddr().toText() << " yi " << resp4->getYiaddr().toText() << "\n";
	uint32_t subnetid = ddns_params->getSubnetId();
	std::pair<isc::asiolink::IOAddress, uint8_t> sn_addr = subnet->get();
	vector<uint8_t> addr_octets = sn_addr.first.toBytes();
	interesting << "   Id: " << subnetid << " addr " << sn_addr.first.toText() << "/" << +sn_addr.second << " toText " << subnet->toText() << "subnet ref " << +addr_octets[2] << "\n";

	if (addr_octets[2] != 53) {
		new_hostname = hostname;
		interesting << "About to modify " << new_hostname << " dot post " << new_hostname.find('.') << " insert " << to_string(addr_octets[2]) << "\n";
		new_hostname.insert(new_hostname.find('.'), "-" + to_string(addr_octets[2]));
		interesting << hostname << " -> " << new_hostname << "\n";
		handle.setArgument("hostname", new_hostname);
//		string hwaddr = hwaddr_ptr->toText();
		handle.setContext("orig-name", hostname);
	}

interesting << resp4->toText() << endl;
	return 0;
}

// This callout is called at the "pkt4_receive" hook.
int pkt4_receive(CalloutHandle& handle) {
 
	// A pointer to the packet is passed to the callout via a "boost" smart
	// pointer. The include file "pkt4.h" typedefs a pointer to the Pkt4
	// object as Pkt4Ptr.  Retrieve a pointer to the object.
	Pkt4Ptr query4_ptr;
	handle.getArgument("query4", query4_ptr);
 
	// Point to the hardware address.
	HWAddrPtr hwaddr_ptr = query4_ptr->getHWAddr();
 
	// The hardware address is held in a public member variable. We'll classify
	// it as interesting if the sum of all the bytes in it is divisible by 4.
	//  (This is a contrived example after all!)
	long sum = 0;
	for (int i = 0; i < hwaddr_ptr->hwaddr_.size(); ++i) {
		sum += hwaddr_ptr->hwaddr_[i];
	}
 
string hwaddr1 = hwaddr_ptr->toText();
interesting << hwaddr1 << " - Label " << query4_ptr->getLabel() << "\n";

	// Classify it.
	if (sum % 4 == 0) {
		// Store the text form of the hardware address in the context to pass
		// to the next callout.
		string hwaddr = hwaddr_ptr->toText();
		handle.setContext("hwaddr", hwaddr);
	}
 
	return (0);
};
 
}
