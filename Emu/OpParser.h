#pragma once

#include "Cpu.h"

#include <functional>
#include <map>
#include <string>

class OpParser
{
public:
	OpParser(Cpu&, ConstNums&);

private:
	Cpu& cpu;
	ConstNums& c_nums;

	u16 string_to_word(const std::string&) const;
	u16 parse_opcode(const std::string&) const;

	void parse_instructions();

	template<typename T>
	T get_map(const std::string& name, const std::map<std::string, T>& type_map) {
		T ret_val;
		auto it{ type_map.find(name) };

		if (it != type_map.cend()) {
			ret_val = it->second;
		}
		else {
			throw("Illegal name! - fn() = parse_type, name = " + name);
		}

		return ret_val;
	}
};

