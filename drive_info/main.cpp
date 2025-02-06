#include "drive_info.hpp"
#include <system_error>
#include <iostream>

int main()
{
	using namespace ostrinkets;

	try
	{
		std::vector<drive_info> drives = get_drive_info();

		if (drives.empty())
		{
			std::cerr << "Failed to find any drives." << std::endl;
			return EXIT_FAILURE;
		}

		std::cout << "Here are your drives:" << std::endl << std::endl;

		std::cout.width(25);
		std::cout << std::left << "Path:";

		std::cout.width(35);
		std::cout << "Description:";

		std::cout.width(15);
		std::cout << "Partitions:";

		std::cout << "Capacity:" << std::endl;

		for (const auto& drive : drives)
		{
			std::cout.width(25);
			std::cout << drive.path;

			std::cout.width(35);
			std::cout << drive.description;

			std::cout.width(15);
			std::cout << drive.partitions;

			std::cout << drive.capacity << std::endl;
		}
	}
	catch (const std::system_error& sys_error)
	{
		std::cerr << "An exception occurred while listing the drives:" << std::endl;
		std::cerr << sys_error.what() << std::endl;
		std::cerr << "Error code: " << sys_error.code().value() << std::endl;
		return sys_error.code().value();
	}
	catch (const std::exception& e)
	{
		std::cerr << "An exception occurred while listing the drives:" << std::endl;
		std::cerr << e.what();
		return EXIT_FAILURE;
	}

	return 0;
}
