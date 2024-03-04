// pkt_receive4.c__
 
#include <hooks/hooks.h>
#include <dhcp/pkt4.h>
#include <dhcpsrv/srv_config.h>
#include <string>

#include "library_common.h"
#include "pkt4_change_hostname_log.h"
 
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
	handle.getArgument("response4", resp4);
	handle.getArgument("ddns-params", ddns_params);
	handle.getArgument("subnet4", subnet);

	uint32_t subnetid = ddns_params->getSubnetId();
	std::pair<isc::asiolink::IOAddress, uint8_t> sn_addr = subnet->get();
	vector<uint8_t> addr_octets = sn_addr.first.toBytes();

	if (addr_octets[2] == 53)
		return 0;

	new_hostname = hostname;
	new_hostname.insert(new_hostname.find('.'), "-" + to_string(addr_octets[2]));
	handle.setArgument("hostname", new_hostname);

	LOG_INFO(pkt4_change_hostname::pkt4_change_hostname_logger, isc::log::NCHG_HOSTNAME_MODIFIED)
		.arg(hostname)
		.arg(new_hostname);

	OptionPtr opt_hostname = resp4->getOption(DHO_HOST_NAME);
	if (opt_hostname)
		handle.setContext("orig-hostname", opt_hostname->toString());

	OptionPtr opt_fqdn = resp4->getOption(DHO_FQDN);
	if (opt_fqdn && (!opt_hostname || opt_fqdn->toString() != opt_hostname->toText()))
		handle.setContext("orig-fqdn", opt_fqdn->toString());

	return 0;
}
 
}
