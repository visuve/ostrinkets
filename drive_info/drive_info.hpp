#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace ostrinkets
{
	struct drive_info
	{
		std::string path;
		std::string description;
		uint32_t partitions = 0;
		uint64_t capacity = 0;
	};

	std::vector<drive_info> get_drive_info();
}