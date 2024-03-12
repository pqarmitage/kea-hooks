// pkt4_send.c++
 
#include <hooks/hooks.h>
#include <dhcp/pkt4.h>
#include <dhcp/option_string.h>
#include <string>
 
#include "library_common.h"
#include "pkt_change_hostname_log.h"

using namespace isc::dhcp;
using namespace isc::hooks;
using namespace std;
 
extern "C" {
 
// This callout is called at the "pkt4_send" hook.
int pkt4_send(CalloutHandle& handle)
{
	Pkt4Ptr response_ptr;
	string orig_hostname;
	string new_hostname;

	handle.getArgument("response4", response_ptr);

	OptionPtr opt_hostname = response_ptr->getOption(DHO_HOST_NAME);
	if (opt_hostname) {
		new_hostname = opt_hostname->toString();
		try {
			handle.getContext("orig-hostname", orig_hostname);

			OptionStringPtr opt_hostname_resp(new OptionString(Option::V4, DHO_HOST_NAME, orig_hostname));
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
			OptionStringPtr opt_fqdn_resp(new OptionString(Option::V4, DHO_FQDN, orig_fqdn));
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
