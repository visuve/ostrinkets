#pragma once

#include <iostream>
#include <cstdint>

namespace ostrinkets::serial
{
	struct options
	{
		const char* path = nullptr;
		uint32_t baud_rate = 115200;
	};

	void read(const options&, std::ostream&);
};