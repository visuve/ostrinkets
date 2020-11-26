#include "file_info.hpp"
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
		print_file_info(path);
	}

	return 0;
}
