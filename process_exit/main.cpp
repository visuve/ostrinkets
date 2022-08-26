#include "process.hpp"
#include <iostream>

#ifdef _WIN32
int wmain(int argc, wchar_t** argv, wchar_t** envp)
#else
int main(int argc, char** argv, char** envp)
#endif
{
	if (argc < 3)
	{
		std::wcerr << L"Usage: " << argv[0] << L"<path/to/executable> <arg1> <arg2> <argN> ..." << std::endl;
		return EINVAL;
	}

	try
	{
		process p(argv[1], { argv + 2, argv + argc}, envp);

		p.start();
		p.wait();

		int exit_code = p.exit_code();

		std::wcout << argv[1] << L" returned " << exit_code << std::endl;

		return exit_code;
	}
	catch (const std::exception& e)
	{
		std::wcerr << L"An exception occurred: " << e.what() << std::endl;
	}

	return -1;
}
