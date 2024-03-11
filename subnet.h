// #define DEBUG

#include <string>
#include <kea/asiolink/io_address.h>
#include <endian.h>
#include <forward_list>
#include <vector>
#include <list>
#include <exception>
 
#ifdef DEBUG
#include <iostream>
#endif

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

// Used for the extraction of an element from the address for insertion
// into the host name.
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

	uint64_t getVal(const isc::asiolink::IOAddress& addr) const
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
		else if (shift == 0)
			val = val64s[1];
		else
			val = (val64s[1] >> shift) | (val64s[0] << (64 - shift));

		return val & mask;
	}

	uint64_t getVal(const std::string& addr_str) const
	{
		isc::asiolink::IOAddress addr(addr_str);

		return getVal(addr);
	}

private:
	unsigned shift;
	uint64_t mask;

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

			mask = val[1];
		}

#ifdef DEBUG
		std::cout << std::dec << "shift " << shift << std::hex << " mask " << mask << "\n";
#endif
	}
};

class Subnet : public isc::asiolink::IOAddress
{
private:
	uint16_t prefix_len { 0 };

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

	unsigned prefixLen() const { return prefix_len; }

	bool operator<(const Subnet& rval) const { return prefix_len >= rval.prefix_len; }

	bool matches(const isc::asiolink::IOAddress& addr) const {
		if (prefix_len == 0)
			return true;

		if (isV4()) {
			int32_t mask = (-1 << (32 - prefix_len));

			return (addr.toUint32() & mask) == toUint32();
		}

		// IPv6
		int64_t mask[2];
		uint64_t addr_val[2], match_val[2];

		if (prefix_len >= 64) {
			mask[1] = (-1 << (128 - prefix_len));
			mask[0] = -1;
		} else {
			mask[1] = 0;
			mask[0] = (-1 << ( 64 - prefix_len));
		}

		convert_v6_to_64s(addr, addr_val);
		convert_v6_to_64s(*this, match_val);

		return (((addr_val[0] & mask[0]) == match_val[0]) &&
			((addr_val[1] & mask[1]) == match_val[1]));
	}
};

class rule
{
public:
	enum class type {add, remove, modify, append, prepend};

	rule(const std::string& str) {
		size_t offs;

		field = stoul(str, &offs);
		if (str[offs] == '+')
			action = type::add;
		else if (str[offs] == '-') {
			action = type::remove;
			if (str.length() != offs + 1)
				isc_throw(isc::Exception, "String with remove");
			return;
		} else if (str[offs] == ':')
			action = type::modify;
		else if (str[offs] == '>')
			action = type::append;
		else if (str[offs] == '<')
			action = type::prepend;
		else
			isc_throw(isc::Exception, "Bad action");

		format = str.substr(offs + 1);
	}

	void
	process_rule(uint64_t val, std::vector<std::string>& names) const
	{
		if (action == type::remove) {
			if (!field || field > names.size())
				isc_throw(isc::Exception, "Remove field does not exist");
			names.erase(names.begin() + field - 1);
			return;
		}

		std::string new_format { format };

		if (format.find("%") != std::string::npos) {
			std::ostringstream val_str;

			size_t f = 0, l;
			while ((f = new_format.find("%", f)) != std::string::npos) {
				if (f < new_format.length() - 1 &&
				    (new_format[f + 1] == 'x' ||
				     new_format[f + 1] == 'd' ||
				     (new_format[f + 1] == '{' &&
				      (l = new_format.find("}", f + 1)) != std::string::npos))) {
					size_t len = 2;

// do validation when parsing parameters initially
// handle whitespace after number bfdore comma
					val_str.str("");
					if (new_format[f + 1] == 'd')
						val_str << val;
					else if (new_format[f + 1] == 'x')
						val_str << std::hex << val << std::dec;
					else {
						unsigned field;
						int offset = 1;
						int length_raw = 1000000;
						size_t length = std::string::npos;
						size_t end, ends, endo;

						field = stoul(new_format.substr(f + 2), &end);
						if (!end)
							isc_throw(isc::Exception, "Invalid field specifier");

						if (new_format[f + 2 + end] != '}' &&
						    new_format[f + 2 + end] != ',')
							isc_throw(isc::Exception, "Invalid field end");

						if (new_format[f + 2 + end] == ',') {
							offset = std::stoi(new_format.substr(f + 3 + end), &endo);
							if (!ends || offset == 0)
								isc_throw(isc::Exception, "Invalid field offset");
							if (new_format[f + 3 + end + endo] != '}' &&
							    new_format[f + 3 + end + endo] != ',')
								isc_throw(isc::Exception, "Invalid field offset end");

							if (new_format[f + 3 + end + endo] == ',') {
								length_raw = std::stoi(new_format.substr(f + 4 + end + endo), &ends);
								if (!ends)
									isc_throw(isc::Exception, "Invalid field length");
								if (new_format[f + 4 + end + endo + ends] != '}')
									isc_throw(isc::Exception, "Invalid field length end");
								len = 4 + end + endo + ends + 1;
							} else
								len = 3 + end + endo + 1;

						} else
							len = 2 + end + 1;

						if (field > names.size())
							isc_throw(isc::Exception, "Invalid field specified");

						if (offset < 0) {
							if (-offset > names[field - 1].length())
								offset = 1;
							else
								offset += names[field - 1].length() + 1;
						}

						if (length_raw < 0) {
							size_t tmp = offset;
							if (-length_raw > offset) {
								length = offset;
								offset = 1;
							} else {
								offset += length_raw + 1;
								length = -length_raw;
							}
						} else
							length = length_raw;

						val_str << names[field - 1].substr(offset - 1, length);
					}
					new_format.replace(f, len, val_str.str());
				}
			}
		}

		if (action == type::add) {
			if (!field || field > names.size() + 1)
				isc_throw(isc::Exception, "Modify field does not exist");

			if (field == names.size() + 1)
				names.push_back(new_format);
			else
				names.insert(names.begin() + field - 1, new_format);
			return;
		}

		if (action == type::modify) {
			if (!field || field > names.size())
				isc_throw(isc::Exception, "Modify field does not exist");

			names[field - 1] = new_format;

			return;
		}

		if (action == type::append) {
			if (!field || field > names.size())
				isc_throw(isc::Exception, "Append field does not exist");

			names[field - 1].append(new_format);

			return;
		}

		if (action == type::prepend) {
			if (!field || field > names.size())
				isc_throw(isc::Exception, "Prepend field does not exist");

			names[field - 1].insert(0, new_format);

			return;
		}
	}

#ifdef DEBUG
	const std::string toStr() const {
		std::ostringstream str;
		std::string op_str;

		if (action == type::add)
			str << "Add";
		else if (action == type::remove)
			str << "Remove";
		else if (action == type::modify)
			str << "Modify";

		str << ": " << field;

		if (action != type::remove)
			str << " - " << format;

		op_str = str.str();

		return op_str;
	}
#endif

private:
	type action;
	unsigned field;
	std::string format;	// not used for remove
				// n- - remove field
				// n+ - add field
				// n: - modify field
				// n< - prepend to field
				// n> - append to field
				// In add/modify string, $n is current value of field n
				// 			 ${n,f,l} is a substring
				// 			 f, l can be integers for char offset and start (allow -ve)
				// 			 strings - 'asdf' or "asdf" for match to replace from/to
				// 			 %{d|x} is the addrVal
};

class nameChange : public Subnet, addrVal
{
	// Subnet is for matching against the address
	// addrVal is for picking the number to use to add to the hostname
	
//	bool operator<(const nameChange & rval) const \{ return 
public:
	nameChange(const std::string& subnet, std::list<std::string>& process) : Subnet(subnet) , addrVal(process.front())
	{
		bool first = true;

		for (const auto& s : process) {
			if (first) {
				first = false;
				continue;
			}

			rule r { s };
			rules.push_back(r);
		}
	}

	void update(const isc::asiolink::IOAddress& addr, std::vector<std::string>& names) const {
		for (const auto& r : rules) {
			r.process_rule(getVal(addr), names);
		}
	}

#ifdef DEBUG
	void print_rules() {
		for (const auto& r : rules)
			std::cout << r.toStr() << "\n";
	}
#endif

private:
	std::list<rule> rules;
};

class Changes
{
public:
	const class nameChange& add(const class nameChange& entry)
	{
		if (changes.empty()) {
			changes.push_front(entry);
			return entry;
		}

		for (auto c = changes.cbegin(); c != changes.cend(); c++) {
			if (*c < entry)
				continue;

			changes.insert(c, entry);

			return entry;
		}

		changes.push_back(entry);

		return entry;
	}

	const nameChange& find(const std::string& addr_str) {
		isc::asiolink::IOAddress addr { addr_str };
		for (const auto& nc : changes) {
			if (nc.matches(addr))
				return nc;
		}

		return *(reinterpret_cast<nameChange*>(0));
	}

	const nameChange& find(const isc::asiolink::IOAddress& addr) const {
		for (const auto& nc : changes) {
			if (nc.matches(addr))
				return nc;
		}

		return *reinterpret_cast<nameChange*>(0);
	}

private:
	std::list<class nameChange> changes;
};

class fqdn
{
public:
	fqdn(const std::string name) {
		size_t start = 0;
		size_t next;

		do {
			next = name.find('.', start);
			elements.push_back(name.substr(start, next == std::string::npos ? next : next - start));
			start = next + 1;
		} while (next != std::string::npos);
	}

	void
	replace(size_t index, const std::string ele)
	{
		elements[index] = ele;
	}

	void
	insert(size_t index, const std::string ele)
	{
		elements.insert(elements.begin() + index, ele);
	}

	void
	erase(size_t index)
	{
		elements.erase(elements.begin() + index);
	}

	std::string 
	str()
	{
		std::ostringstream name;
		std::string name_str;

		for (const auto&ele : elements) {
			name << ele << ".";
		}

		name_str = name.str();
		name_str.pop_back();

		return name_str;
	}

	pqa::fqdn&
	update(isc::asiolink::IOAddress& addr, const pqa::Changes& changes)
	{
		// Update the fqdn based on the rule
		const nameChange& rule { changes.find(addr) };

		if (&rule)
			rule.update(addr, elements);

		return *this;
	}

private:
	std::vector<std::string> elements;
};

}
