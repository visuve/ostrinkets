#include "mem_search.hpp"

#include <iostream>

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		std::cerr << "Usage "
			<< argv[0] << " <process-name> <value>" << std::endl;
		return EXIT_FAILURE;
	}

	try
	{
		std::string_view process_name = argv[1];
		std::string_view value_to_search = argv[2];
		uint64_t address = mem_search::find_value_in_process(process_name, value_to_search);

		if (address != 0)
		{
			std::cout << value_to_search << " found at: " << std::hex << address << std::endl;
			return EXIT_SUCCESS;
		}

		std::cout << value_to_search << " not found." << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cout << "An exception occurred: " << e.what() << std::endl;
	}

	return EXIT_FAILURE;
}
