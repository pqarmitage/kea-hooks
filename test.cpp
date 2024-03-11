#include <string>
#include <iostream>
 
#include "subnet.h"

using namespace std;
 
int main(int argc, char **argv)
{
#ifdef OLD_TESTS
	string hostname = argc >= 2 && argv[1][0] != '-' ? argv[1] : "tamar.armitage.org.uk";
	string new_hostname;
	uint8_t subnet_no = 53;

	new_hostname = hostname;
	new_hostname.insert(new_hostname.find('.'), "-" + to_string(subnet_no));
	cout << hostname << " -> " << new_hostname << "\n";

	string addr = argc >= 3 && argv[2][0] != '-' ? argv[2] : "1.2.3.4";

	pqa::Subnet sn(addr);
	cout << "Address is " << sn << " prefix " << sn.prefixLen() << "\n";

	pqa::Subnet a("2001:470:69dd:35::18");

	vector<uint8_t> vec = a.toBytes();
	cout << "num bytes " << vec.size() << "\n";

	for (const auto& b : vec)
		cout << hex <<+ b << " ";
	cout << "\n";

	uint64_t val[2] = { 0 };
	for (int i = 0; i <= 7; i++) {
		val[0] = (val[0] << 8) + vec[i];
		val[1] = (val[1] << 8) + vec[i + 8];
	}

	cout << hex << val[0] << " " << val[1] << "\n";
	pqa::addrVal vv("0:0:0:ffff::");

	cout << "Net val " << vv.getVal("2001:470:69dd:35::210") << "\n";

//	pqa::addrVal va("0:ffff:ffff:ffff:ffff:f000::");

	/* Test fqdn */
	pqa::fqdn me {"samson.armitage.org.uk"};

	cout << me.str() << "\n";
	me.erase(1);
	cout << me.str() << "\n";

	me.insert(1, "armitage");
	cout << me.str() << "\n";

	me.replace(2, "co");
	cout << me.str() << "\n";

	/* Test rule */
	pqa::rule r { "21+fred" };
#ifdef DEBUG
	cout << r.toStr() << "\n";
#endif
#endif

	/* Create some name change rules, and pass them some subnets and fqdn */
	pqa::Changes rules;
	std::list<std::string> my_rules;
	isc::asiolink::IOAddress host_addr { (argc < 4 || argv[3][0] == '-') ? "172.21.53.10" : argv[3] };

	my_rules = std::list<std::string> { "0.0.255.0", "1>-%d", "3+3rdextra", "5-", "2:dann", "5+last", "1+first" };
	rules.add(pqa::nameChange { "172.21.48.0/20", my_rules });

	my_rules = std::list<std::string> { "0.0.0.255", "1<%d-", "1>-%x" };
	rules.add(pqa::nameChange { "192.168.48.0/24", my_rules });

	my_rules = std::list<std::string> { "0.0.255.0", "1>-%d" };
	rules.add(pqa::nameChange { "172.16.0.0/12", my_rules });

	pqa::fqdn host { "samson.armitage.org.uk" };
	host.update(host_addr, rules);
	cout << "Updated to " << host.str() << "\n";

	/* Now for IPv6 */
	pqa::Changes rules6;
	isc::asiolink::IOAddress host_addr6 { argc < 5 || argv[4][0] == '-' ? "2001:470:69dd:35::210" : argv[4] };

	my_rules = std::list<std::string> { "0:0:0:ffff::", "1>-%d", "3+3rdextra", "5-", "2:dann", "5+last", "1+first" };
	rules6.add(pqa::nameChange { "2001:470:69dd:30::/60", my_rules });

	my_rules = std::list<std::string> { "::ffff", "1<%d-", "1>-%x" };
	rules6.add(pqa::nameChange { "fe80::/12", my_rules });

	my_rules = std::list<std::string> { "::ffff:ffff:ffff:ffff", "1>-%d-%x" };
	rules6.add(pqa::nameChange { "2001:470:69dd::/48", my_rules });

	my_rules = std::list<std::string> { "::ffff:ffff:ffff:ffff:0", "1+%{1,3,2}-xx", "2>-%d-%x" };
	rules6.add(pqa::nameChange { "2001:470:69dc::/48", my_rules });
	rules6.add(pqa::nameChange { "2001:470:69db::/48", my_rules });

	my_rules = std::list<std::string> { "::ffff:ffff:ffff:ffff:0", "1+%{1,-3,2}-xx", "2+%{2,-3,-2}-yy", "3+%{3,4,-2}-zz", "4>-%d-%x" };
	rules6.add(pqa::nameChange { "2001:470:69da::/48", my_rules });
	rules6.add(pqa::nameChange { "2001:470:69d9::/48", my_rules });

	host = pqa::fqdn { "samson.armitage.org.uk" };
	host.update(host_addr6, rules6);
	cout << "Updated to " << host.str() << "\n";
}
