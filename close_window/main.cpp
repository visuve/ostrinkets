#include "window_manager.hpp"

#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << "<pid>" << std::endl;
		return EINVAL;
	}

	try
	{
		int pid = std::stoi(argv[1]);

		if (window_manager().close_by_pid(pid))
		{
			std::cout << pid << " closed" << std::endl;
		}
		else
		{
			std::cerr << "Failed to close " << pid << std::endl;
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "An exception occurred: " << e.what() << std::endl;
	}

	return 0;
}