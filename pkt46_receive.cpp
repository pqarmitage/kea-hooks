
extern "C" {
 
int make_v(ddns,_update)(CalloutHandle& handle) {
	string hostname;
	string new_hostname;
	bool fwd_update = true;
	bool rev_update = false;
	make_v(Pkt,Ptr) resp;
	string ci_addr_str;
	DdnsParamsPtr ddns_params;
	make_v(Subnet,Ptr) subnet;

	handle.getArgument("hostname", hostname);
	handle.getArgument("response" IPvs, resp);
	handle.getArgument("ddns-params", ddns_params);
	handle.getArgument("subnet" IPvs, subnet);

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

	LOG_INFO(pkt_change_hostname::pkt_change_hostname_logger, isc::log::NCHG_HOSTNAME_MODIFIED)
		.arg(hostname)
		.arg(new_hostname);

	OptionPtr opt_hostname = resp->getOption(DHO_HOST_NAME);
	if (opt_hostname)
		handle.setContext("orig-hostname", opt_hostname->toString());

	OptionPtr opt_fqdn = resp->getOption(DHO_FQDN);
	if (opt_fqdn && (!opt_hostname || opt_fqdn->toString() != opt_hostname->toText()))
		handle.setContext("orig-fqdn", opt_fqdn->toString());

	return 0;
}
 
}
