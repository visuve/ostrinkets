#pragma once

#include <string_view>
#include <cstdint>

namespace mem_search
{
	uint64_t find_value_in_process(std::string_view process_name, std::string_view value_to_search);
}