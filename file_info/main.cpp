#include "file_info.hpp"
#include <iostream>
#include <system_error>
#include <vector>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
int wmain(int argc, wchar_t* argv[])
{
	_setmode(_fileno(stdout), _O_U16TEXT);
	_setmode(_fileno(stderr), _O_U16TEXT);
#else
int main(int argc, char* argv[])
{
#endif
	using namespace fstrinkets;

	if (argc <= 1)
	{
		return EINVAL;
	}

	const std::vector<std::filesystem::path> paths({ argv + 1, argv + argc });

	for (const std::filesystem::path& path : paths)
	{
		try
		{
			print_file_info(path);
		}
		catch (const std::system_error& sys_error)
		{
			std::cerr << "An exception occurred:" << std::endl;
			std::cerr << sys_error.what() << std::endl;
			std::cerr << "Error code: " << sys_error.code().value() << std::endl;
			return sys_error.code().value();
		}
		catch (const std::exception& e)
		{
			std::cerr << "An exception occurred:" << std::endl;
			std::cerr << e.what();
			return EXIT_FAILURE;
		}
	}

	return 0;
}
