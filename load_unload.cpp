// load_unload.c++
 
#include <hooks/hooks.h>
#include <fstream>	// only needed for debug function getParameterNames()

#include "library_common.h"
#include "pkt4_change_hostname_log.h"
 
using namespace isc::hooks;
using namespace isc::log;
using namespace pkt4_change_hostname;
 
extern "C" {
 
static std::vector<std::string>
getParameterNames(LibraryHandle& handle) {
	std::vector<std::string> names;
	std::fstream config;

	isc::data::ConstElementPtr params = handle.getParameters();

	config.open("/tmp/config.log", std::fstream::out | std::fstream::app);
	config << "\n\n";

	if (!params ||
	    (params->getType() != isc::data::Element::map) ||
	    (params->size() == 0)) {
		config << "params not valid\n";
		config.close();
		return names;
	}

	auto const& map = params->mapValue();
	for (auto const& elem : map) {
		names.push_back(elem.first);
		isc::data::ConstElementPtr value = elem.second;

		config << elem.first << ": ";

		int type = value->getType();
		if (type == isc::data::Element::integer) {
			int val = value->intValue();
			config << val;
		} else if (type == isc::data::Element::real) {
			double val = value->doubleValue();
			config << val;
		} else if (type == isc::data::Element::boolean) {
			bool val = value->boolValue();
			config << val;
		} else if (type == isc::data::Element::null) {
			config << "NULL";
		} else if (type == isc::data::Element::string) {
			std::string val = value->stringValue();
			config << val;
#if KEA_HOOKS_VERSION >= 20400
		} else if (type == isc::data::Element::bigint) {
			isc::util::int128_t val = value->bigIntValue();
			config << val;
#endif
		} else if (type == isc::data::Element::list) {
			const std::vector<isc::data::ElementPtr>& val = value->listValue();
			config << "List - " ;
			for (const auto& v : val)
				config << v->str() << ", ";
		} else if (type == isc::data::Element::map) {
			const std::map<std::string, isc::data::ConstElementPtr>& val = value->mapValue();
			config << "Map - ";
			for (const auto& m : val)
				config << m.first << ": " << m.second->str() << ", ";
		} else {
			config << "Unknown type " << type;
			break;
		}
	}
	config << "\n";

	config.close();

	return names;
}

/*
 * Configuration:
 *
 * "ignore-subnets": [ "172.21.53.0/24", "192.168.0.0/16" ],
 * "process-subnets": {
 *	// use 3rd byte for number (e.g. 52), append "-52" to first part of FQDN add new
 *	// second part of FQDN net34, remove 3rd part of FQDN.
 *	// So tamar.armitage.org.uk becomes tamar-52.net34.armitage.uk
 *     "172.21.48.0/20": [ "0.0.255.0", "$0-%d", "+$1net%x", "-$3" ],
 *     "0.0.0.0/0", [ "0.255.255.0", "+$1net-%x" ]
 *     "2001:470:69dd::/48", [ "0:0:0:ffff::", "$0-%d" ]
 * }
 */
static bool
processParameters(LibraryHandle& handle) {
	isc::data::ConstElementPtr params = handle.getParameters();

	if (!params ||
	    (params->getType() != isc::data::Element::map) ||
	    (params->size() == 0)) {
		LOG_ERROR(pkt4_change_hostname_logger, NCHG_REQUIRES_CONFIG);
		return true;
	}

	auto const& map = params->mapValue();
	for (auto const& elem : map) {
		isc::data::ConstElementPtr value = elem.second;

		if (elem.first == "ignore-subnets") {
			// A list of strings
			if (value->getType() != isc::data::Element::list) {
				LOG_ERROR(pkt4_change_hostname_logger, NCHG_IGNORE_SUBNETS_NOT_LIST).arg(value->str());
				return true;
			}

			for (const auto& v : value->listValue()) {
				if (v->getType() != isc::data::Element::string) {
					LOG_ERROR(pkt4_change_hostname_logger, NCHG_IGNORE_SUBNETS_LIST_MEMBER_NOT_STRING).arg(value->str()).arg(v->str());
					return true;
				}
			}

			continue;
		}

		if (elem.first == "process-subnets") {
			// A map of <string, list<string>>
			if (value->getType() != isc::data::Element::map) {
				LOG_ERROR(pkt4_change_hostname_logger, NCHG_PROCESS_SUBNETS_NOT_MAP).arg(elem.first).arg(value->str());
				return true;
			}

			const std::map<std::string, isc::data::ConstElementPtr>& val = value->mapValue();
			for (const auto& v : val) {
				if (v.second->getType() != isc::data::Element::list) {
					LOG_ERROR(pkt4_change_hostname_logger, NCHG_PROCESS_SUBNETS_MAP_MEMBER_NOT_LIST).arg(v.first).arg(v.second->str());
					return true;
				}

				for (const auto& e : v.second->listValue()) {
					if (e->getType() != isc::data::Element::string) {
						LOG_ERROR(pkt4_change_hostname_logger, NCHG_PROCESS_SUBNETS_MAP_LIST_MEMBER_NOT_STRING).arg(v.first).arg(v.second->str()).arg(e->str());
						return true;
					}
				}
			}

			continue;
		}

		LOG_ERROR(pkt4_change_hostname_logger, NCHG_PARAMETER_UNKNOWN).arg(elem.first);
		return true;
	}

	return false;
}

int load(LibraryHandle &handle) {
	std::vector<std::string> names;
	int ret;

	names = getParameterNames(handle);

	ret = processParameters(handle);

	return (ret);
}
 
int unload() {
	return (0);
}
 
}
