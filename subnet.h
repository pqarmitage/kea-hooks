#include <string>
#include <kea/asiolink/io_address.h>
#include <endian.h>
 
#include <iostream>

namespace pqa {

class Subnet : public isc::asiolink::IOAddress
{

public:
	Subnet(const std::string& address_str)
		: isc::asiolink::IOAddress(address_str.substr(0, address_str.find('/')))
	{
		size_t slash = address_str.find('/');

		if (slash == std::string::npos)
			prefix_len = (isV4() ? isc::asiolink::V4ADDRESS_LEN : isc::asiolink::V6ADDRESS_LEN) * CHAR_BIT;
		else
			prefix_len = std::stoi(address_str.substr(slash + 1));
	}
	Subnet(const std::string& address_str, const std::string& prefix_str) : isc::asiolink::IOAddress(address_str)
	{
		prefix_len = stoi(prefix_str);
	}
	Subnet(const isc::asiolink::IOAddress& addr, const std::string& prefix_str) : isc::asiolink::IOAddress(addr)
	{
		prefix_len = stoi(prefix_str);
	}
	Subnet(const std::string& address_str, unsigned prefix) : isc::asiolink::IOAddress(address_str), prefix_len(prefix) {}
	Subnet(const isc::asiolink::IOAddress& addr, unsigned prefix = 200) : isc::asiolink::IOAddress(addr), prefix_len(prefix)
	{
		if (prefix == 200)
			prefix_len = (isV4() ? isc::asiolink::V4ADDRESS_LEN : isc::asiolink::V6ADDRESS_LEN) * CHAR_BIT;
	}

	unsigned prefixLen() { return prefix_len; }

private:
	uint16_t prefix_len { 0 };
};

class addrVal
{
public:
	addrVal(const std::string& address_str)
	{
		isc::asiolink::IOAddress addr = (address_str);

		convert_addr_to_shift_mask(addr);
	}
	addrVal(const isc::asiolink::IOAddress& addr)
	{
		convert_addr_to_shift_mask(addr);
	}
	addrVal(void)
	{
		// Assume class C subnetwork and we want the 3rd octet
		shift = 8;
		mask = 0xff;
	}

	unsigned getVal(const isc::asiolink::IOAddress& addr)
	{
		uint64_t val;

		if (addr.isV4()) {
			uint32_t val = addr.toUint32();

			val >>= shift;
			return val & mask;
		}

		// IPv6
		uint64_t val64s[2];
		convert_v6_to_64s(addr, val64s);

		if (shift >= 64)
			val = val64s[0] >> (shift - 64);
		else
			val = (val64s[1] >> shift) | (val64s[1] << (64 - shift));

		return val & mask;
	}

	int getVal(const std::string& addr_str)
	{
		isc::asiolink::IOAddress addr(addr_str);

		return getVal(addr);
	}

private:
	unsigned shift;
	unsigned mask;

	void convert_v6_to_64s(const isc::asiolink::IOAddress& addr, uint64_t *val)
	{
		std::vector<uint8_t> vec = addr.toBytes();
		uint8_t bytes[vec.size()];
		int i = 0;

		for (auto &n : vec)
			bytes[i++] = n;
		val[0] = be64toh(*reinterpret_cast<uint64_t *>(&bytes[0]));
		val[1] = be64toh(*reinterpret_cast<uint64_t *>(&bytes[sizeof(uint64_t)]));
	}

	void convert_addr_to_shift_mask(const isc::asiolink::IOAddress& addr)
	{
		// Ensure addr not empty address
		if (addr.isV4Zero() || addr.isV6Zero()) {
			// Should throw exception
			shift = 0;
			mask = 0;
			return;
		}

		shift = 0;
		if (addr.isV4()) {
			mask = addr.toUint32();
			while (!(mask & 1)) {
				mask >>= 1;
				shift++;
			}
		} else {
			uint64_t val[2] = { 0 };

			convert_v6_to_64s(addr, val);

			while (!(val[1] & 1)) {
				val[1] >>= 1;
				if (val[0] & 1)
					val[1] |= (1ULL << 63);
				val[0] >>= 1;
				shift++;
			}

			if (val[0])
				isc_throw(isc::Exception, "mask to big");
//				isc_throw_1(isc::Exception, "mask to big", val[0]);

			mask = val[1];
		}

		std::cout << std::dec << "shift " << shift << std::hex << " mask " << mask << "\n";
	}
};

}
