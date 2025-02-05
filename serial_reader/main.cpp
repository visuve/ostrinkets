#include "serial_reader.hpp"

int main(int argc, char** argv)
{
	using namespace ostrinkets;

	if (argc <= 1)
	{
		return EINVAL;
	}

	try
	{
		serial::options opt;
		opt.path = argv[1];

		serial::read(opt, std::cout);
	}
	catch (const std::system_error& sys_error)
	{
		std::cerr << "An exception occurred while reading serial:" << std::endl;
		std::cerr << sys_error.what() << std::endl;
		std::cerr << "Error code: " << sys_error.code().value() << std::endl;
		return sys_error.code().value();
	}
	catch (const std::exception& e)
	{
		std::cerr << "An exception occurred while reading serial:" << std::endl;
		std::cerr << e.what();
		return EXIT_FAILURE;
	}

	return 0;
}
