#define IPs1(v) #v
#define IPs(v)	IPs1(v)
#define	IPvs	IPs(IPv)
#define	make_v3a(a,b,c)	a ## b ## c
#define make_v3(a,b,c)	make_v3a(a, b, c)
#define make_v(a,c)	make_v3(a, IPv, c)

extern "C" {
 
// This callout is called at the "pkt{4|6}_send" hook.
int make_v(pkt,_send)(CalloutHandle& handle)
{
	make_v(Pkt,Ptr) response_ptr;
	string orig_hostname;
	string new_hostname;

	handle.getArgument("response" IPvs, response_ptr);

	OptionPtr opt_hostname = response_ptr->getOption(DHO_HOST_NAME);
	if (opt_hostname) {
		new_hostname = opt_hostname->toString();
		try {
			handle.getContext("orig-hostname", orig_hostname);

			OptionStringPtr opt_hostname_resp(new OptionString(make_v(Option::V,), DHO_HOST_NAME, orig_hostname));
			response_ptr->delOption(DHO_HOST_NAME);
			response_ptr->addOption(opt_hostname_resp);
		} catch (const NoSuchCalloutContext&) {
			// This would mean the kea has added the HOST_NAME option AFTER the call of ddns{4|6}_update()
			LOG_INFO(pkt_change_hostname::pkt_change_hostname_logger, isc::log::NCHG_HOSTNAME_NO_ORIG)
				.arg(new_hostname);
		}
	}

	OptionPtr opt_fqdn = response_ptr->getOption(DHO_FQDN);
	if (opt_fqdn) {
		string orig_fqdn;

		if (new_hostname.empty())
			new_hostname = opt_fqdn->toString();

		try {
			handle.getContext("orig-fqdn", orig_fqdn);
		} catch (const NoSuchCalloutContext&) {
			orig_fqdn = orig_hostname;
		}

		if (!orig_fqdn.empty()) {
			OptionStringPtr opt_fqdn_resp(new OptionString(make_v(Option::V,), DHO_FQDN, orig_fqdn));
			response_ptr->delOption(DHO_FQDN);
			response_ptr->addOption(opt_fqdn_resp);
		} else {
			// This would mean the kea has added the FQDN option AFTER the call of ddns{4|6}_update()
			LOG_INFO(pkt_change_hostname::pkt_change_hostname_logger, isc::log::NCHG_FQDN_NO_ORIG)
				.arg(new_hostname);
		}
	}

	if (!orig_hostname.empty() && !new_hostname.empty()) {
		 LOG_INFO(pkt_change_hostname::pkt_change_hostname_logger, isc::log::NCHG_HOSTNAME_RESTORED)
				.arg(new_hostname)
				.arg(orig_hostname);
	}

	return (0);
}
 
}
