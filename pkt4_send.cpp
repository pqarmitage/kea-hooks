// pkt4_send.c++
 
#include <hooks/hooks.h>
#include <dhcp/pkt4.h>
#include <dhcp/option_string.h>
#include <string>
 
#include "library_common.h"
#include "pkt4_change_hostname_log.h"

using namespace isc::dhcp;
using namespace isc::hooks;
using namespace std;
 
extern "C" {
 
// This callout is called at the "pkt4_send" hook.
int pkt4_send(CalloutHandle& handle)
{
	Pkt4Ptr response4_ptr;
	string orig_name;
	string new_name;

	handle.getArgument("response4", response4_ptr);

	try {
		handle.getContext("orig-name", orig_name);

		try {
			handle.getContext("new-name", new_name);
			LOG_INFO(pkt4_change_hostname::pkt4_change_hostname_logger, isc::log::LOG_HOSTNAME_RESTORED)
				.arg(new_name)
				.arg(orig_name);
		} catch (const NoSuchCalloutContext&) {
			LOG_INFO(pkt4_change_hostname::pkt4_change_hostname_logger, isc::log::LOG_HOSTNAME_RESTORED_NO_NEW)
				.arg(orig_name);
		}
	} catch (const NoSuchCalloutContext&) {
		// No such element in the per-request context with the name "orig-name".
		orig_name.clear();
	}
 
	OptionPtr opt_hostname = response4_ptr->getOption(DHO_HOST_NAME);
	if (opt_hostname) {
		string orig_hostname;
		try {
			handle.getContext("orig-hostname", orig_hostname);
		} catch (const NoSuchCalloutContext&) {
			orig_hostname = orig_name;
		}

		if (!orig_hostname.empty()) {
			OptionStringPtr opt_hostname_resp(new OptionString(Option::V4, DHO_HOST_NAME, orig_hostname));
			response4_ptr->delOption(DHO_HOST_NAME);
			response4_ptr->addOption(opt_hostname_resp);
		}
	}

	OptionPtr opt_fqdn = response4_ptr->getOption(DHO_FQDN);
	if (opt_fqdn) {
		string orig_fqdn;
		try {
			handle.getContext("orig-fqdn", orig_fqdn);
		} catch (const NoSuchCalloutContext&) {
			orig_fqdn = orig_name;
		}

		if (!orig_fqdn.empty()) {
			OptionStringPtr opt_fqdn_resp(new OptionString(Option::V4, DHO_FQDN, orig_fqdn));
			response4_ptr->delOption(DHO_FQDN);
			response4_ptr->addOption(opt_fqdn_resp);
		}
	}

	return (0);
}
 
}
