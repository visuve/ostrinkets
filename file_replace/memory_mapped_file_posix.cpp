#include "memory_mapped_file.hpp"

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#if defined(__linux__)
using file_status = struct stat64;
constexpr auto file_status_function = fstat64;
#else
using file_status = struct stat;
constexpr auto file_status_function = fstat;
#endif

class memory_mapped_file_impl
{
public:
	memory_mapped_file_impl(const std::filesystem::path& path) :
		_descriptor(open(path.c_str(), O_RDONLY))
	{
		if (_descriptor == -1)
		{
			throw std::system_error(errno, std::system_category(), "open");
		}

		file_status status;

		if (file_status_function(_descriptor, &status) == -1)
		{
			throw std::system_error(errno, std::system_category(), "fstat");
		}

		if (!S_ISREG(status.st_mode))
		{
			throw std::invalid_argument("invalid path");
		}

		_size = status.st_size;

		_view = mmap(nullptr, _size, PROT_READ, MAP_PRIVATE, _descriptor, 0);

		if (_view == MAP_FAILED)
		{
			throw std::system_error(errno, std::system_category(), "mmap");
		}
	}

	~memory_mapped_file_impl()
	{
		if (_descriptor)
		{
			::close(_descriptor);
		}
	}

	std::string_view data()
	{
		return { reinterpret_cast<char*>(_view), _size };
	}

	void close()
	{
		if (_view)
		{
			if (munmap(_view, _size) == -1)
			{
				throw std::system_error(errno, std::system_category(), "munmap");
			}
		}

		if (_descriptor > 0)
		{
			if (::close(_descriptor) == -1)
			{
				throw std::system_error(errno, std::system_category(), "close");
			}

			_descriptor = 0;
		}
	}

private:
	int _descriptor = 0;
	void* _view = nullptr;
	size_t _size = 0;
};

memory_mapped_file::memory_mapped_file(const std::filesystem::path& path) :
	_impl(new memory_mapped_file_impl(path))
{
}

memory_mapped_file::~memory_mapped_file()
{
	if (_impl)
	{
		delete _impl;
	}
}

std::string_view memory_mapped_file::data() const
{
	return _impl->data();
}

void memory_mapped_file::close()
{
	return _impl->close();
}
