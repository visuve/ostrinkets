#include "drive_info.hpp"
#include <iostream>

int main()
{
	using namespace fstrinkets;

	std::vector<drive_info> drives = get_drive_info();

	if (drives.empty())
	{
		return EXIT_FAILURE;
	}

	std::cout << std::endl << "Here are your drives:" << std::endl << std::endl;

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

	return 0;
}
