// pkt_receive6.cpp
 
#include <hooks/hooks.h>
#include <dhcp/pkt6.h>
#include <dhcpsrv/srv_config.h>
#include <string>

#include "library_common.h"
#include "pkt4_change_hostname_log.h"
 
#include "load_unload.h"

    #include <fstream>

using namespace isc::dhcp;
using namespace isc::hooks;
using namespace std;
 
extern "C" {
 
int ddns6_update(CalloutHandle& handle) {
	string hostname;
	string new_hostname;
	bool fwd_update = true;
	bool rev_update = false;
	Pkt6Ptr resp6;
	string ci_addr_str;
	DdnsParamsPtr ddns_params;
	Subnet6Ptr subnet;

	handle.getArgument("hostname", hostname);
	handle.getArgument("response6", resp6);
	handle.getArgument("ddns-params", ddns_params);
	handle.getArgument("subnet6", subnet);

	uint32_t subnetid = ddns_params->getSubnetId();
	std::pair<isc::asiolink::IOAddress, uint8_t> sn_addr = subnet->get();
	vector<uint8_t> addr_octets = sn_addr.first.toBytes();

	// Change if this is a subnet we are NOT changing
	for (const auto& sn : ignore_subnets) {
		if (sn.matches(sn_addr.first))
			return 0;
	}

	// Update the hostname if the subnet matches
	pqa::fqdn host(hostname);
	host.update(sn_addr.first, name_changes);
	new_hostname = host.str();

	if (new_hostname != hostname)
		handle.setArgument("hostname", new_hostname);

	LOG_INFO(pkt4_change_hostname::pkt4_change_hostname_logger, isc::log::NCHG_HOSTNAME_MODIFIED)
		.arg(hostname)
		.arg(new_hostname);

	OptionPtr opt_hostname = resp6->getOption(DHO_HOST_NAME);
	if (opt_hostname)
		handle.setContext("orig-hostname", opt_hostname->toString());

	OptionPtr opt_fqdn = resp6->getOption(DHO_FQDN);
	if (opt_fqdn && (!opt_hostname || opt_fqdn->toString() != opt_hostname->toText()))
		handle.setContext("orig-fqdn", opt_fqdn->toString());

	return 0;
}
 
}
