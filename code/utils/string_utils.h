#pragma once

#include "globalincs/pstypes.h"

namespace util {

template <typename Out>
void split_string(const SCP_string& s, char delim, Out result)
{
	SCP_stringstream ss(s);
	SCP_string item;
	while (std::getline(ss, item, delim)) {
		if (!item.empty()) {
			*(result++) = item;
		}
	}
}

std::vector<std::string> split_string(const std::string& s, char delim)
{
	std::vector<std::string> elems;
	split_string(s, delim, std::back_inserter(elems));
	return elems;
}

} // namespace util
