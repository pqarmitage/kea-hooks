// pkt4_send.c++
 
#include <hooks/hooks.h>
#include <dhcp/pkt4.h>
#include <dhcp/option_string.h>
#include "library_common.h"
 
#include <string>
 
using namespace isc::dhcp;
using namespace isc::hooks;
using namespace std;
 
extern "C" {
 
// This callout is called at the "pkt4_send" hook.
int pkt4_send(CalloutHandle& handle)
{
	Pkt4Ptr response4_ptr;

	handle.getArgument("response4", response4_ptr);

	// Obtain the hardware address of the "interesting" client.  We have to
	// use a try...catch block here because if the client was not interesting,
	// no information would be set and getArgument would thrown an exception.
	try {
		string hwaddr;

		handle.getContext("hwaddr", hwaddr);
 
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
interesting << "No hwaddr callout\n"; flush(interesting);
	}
 
	string orig_name;
	try {
		handle.getContext("orig-name", orig_name);
 
		interesting << "orig-name: " << orig_name << "\n";

		// ... and to guard against a crash, we'll flush the output stream.
		flush(interesting);

		handle.setArgument("hostname", orig_name);
 
	} catch (const NoSuchCalloutContext&) {
		// No such element in the per-request context with the name "hwaddr".
		// This means that the request was not an interesting, so do nothing
		// and dismiss the exception.
		//
		interesting << "No orig-name context\n";
		orig_name.clear();
	}
 
interesting << "I am here" << endl;

interesting << "Pkt: " << response4_ptr->toText() << endl;
interesting << "Done toText" << endl; flush(interesting);
interesting << "About to get option" << endl; flush(interesting);
	OptionPtr opt_hostname = response4_ptr->getOption(DHO_HOST_NAME);
interesting << "Got option " << (!opt_hostname ? "(NULL)" : "(success)") << endl; flush(interesting);

	if (opt_hostname) {
		interesting << "opt_hostname " << opt_hostname->toString() << ": " << opt_hostname->toText() << endl;
flush(interesting);

		string orig_hostname;
		try {
			handle.getContext("orig-hostname", orig_hostname);
	 
			interesting << "orig-hostname: " << orig_hostname << "\n";

			// ... and to guard against a crash, we'll flush the output stream.
			flush(interesting);

		} catch (const NoSuchCalloutContext&) {
			interesting << "hostname option but no orig-hostname context\n";
			orig_hostname = orig_name;
		}

		if (!orig_hostname.empty()) {
			OptionStringPtr opt_hostname_resp(new OptionString(Option::V4, DHO_HOST_NAME, orig_hostname));
interesting << "About to del hostname" << std::endl;
			response4_ptr->delOption(DHO_HOST_NAME);
interesting << "About to add" << std::endl;
			response4_ptr->addOption(opt_hostname_resp);
interesting << "Done add" << std::endl;
		}
	} else
		interesting << "no hostname option" << endl;
flush(interesting);

	OptionPtr opt_fqdn = response4_ptr->getOption(DHO_FQDN);
	if (opt_fqdn) {
		interesting << "opt_fqdn " << opt_fqdn->toString() << ": " << opt_fqdn->toText() << "\n";

		string orig_fqdn;
		try {
			handle.getContext("orig-fqdn", orig_fqdn);
	 
			interesting << "orig-fqdn: " << orig_fqdn << "\n";

			// ... and to guard against a crash, we'll flush the output stream.
			flush(interesting);
		} catch (const NoSuchCalloutContext&) {
			interesting << "FQDN option but no orig-fqdn context\n";
			orig_fqdn = orig_name;
		}

		if (!orig_fqdn.empty()) {
			OptionStringPtr opt_fqdn_resp(new OptionString(Option::V4, DHO_FQDN, orig_fqdn));
interesting << "About to del fqdn" << std::endl;
			response4_ptr->delOption(DHO_FQDN);
interesting << "About to add" << std::endl;
			response4_ptr->addOption(opt_fqdn_resp);
interesting << "Done add" << std::endl;
		}
	} else
		interesting << "no FQDN option\n";

	interesting << "Done\n";
	flush(interesting);

	return (0);
}
 
}
