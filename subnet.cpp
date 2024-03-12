#include <kea/asiolink/io_address.h>
#include <endian.h>
#include <vector>
 
#include "subnet.h"

namespace pqa {

void convert_v6_to_64s(const isc::asiolink::IOAddress& addr, uint64_t *val)
{
	std::vector<uint8_t> vec = addr.toBytes();
	uint8_t bytes[vec.size()];
	int i = 0;

	for (const auto &n : vec)
		bytes[i++] = n;
	val[0] = be64toh(*reinterpret_cast<uint64_t *>(&bytes[0]));
	val[1] = be64toh(*reinterpret_cast<uint64_t *>(&bytes[sizeof(uint64_t)]));
}

}
