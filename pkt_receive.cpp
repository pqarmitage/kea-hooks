#include <hooks/hooks.h>
#include <dhcp/pkt4.h>
#include <dhcp/pkt6.h>
#include <dhcpsrv/srv_config.h>
#include <string>

#include "library_common.h"
#include "pkt_change_hostname_log.h"
 
#include "load_unload.h"

#include "ip_version.h"


using namespace isc::dhcp;
using namespace isc::hooks;
using namespace std;
 
#define IPv	4
#include "pkt46_receive.cpp"
#undef IPv
 
#define IPv	6
#include "pkt46_receive.cpp"
#undef IPv
