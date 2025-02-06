#include "memory_mapped_file.hpp"

#include <Windows.h>

class memory_mapped_file_impl
{
public:
	memory_mapped_file_impl(const std::filesystem::path& path) :
		_file(CreateFileW(
			path.c_str(),
			GENERIC_READ,
			FILE_SHARE_READ,
			nullptr,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL))
	{
		if (_file == nullptr || _file == INVALID_HANDLE_VALUE)
		{
			throw std::system_error(GetLastError(), std::system_category(), "CreateFileW");
		}

		LARGE_INTEGER mapping_size;

		if (!GetFileSizeEx(_file, &mapping_size))
		{
			throw std::system_error(GetLastError(), std::system_category(), "GetFileSizeEx");
		}

		_size = mapping_size.QuadPart;

		_mapping = CreateFileMappingW(
			_file,
			nullptr,
			PAGE_READWRITE,
			mapping_size.HighPart,
			mapping_size.LowPart,
			nullptr);

		if (!_mapping)
		{
			throw std::system_error(GetLastError(), std::system_category(), "CreateFileMappingW");
		}

		_view = MapViewOfFile(_mapping, FILE_MAP_ALL_ACCESS, 0, 0, _size);

		if (!_view)
		{
			throw std::system_error(GetLastError(), std::system_category(), "MapViewOfFile");
		}
	}

	~memory_mapped_file_impl()
	{
		if (_view)
		{
			UnmapViewOfFile(_view);
		}

		if (_mapping)
		{
			CloseHandle(_mapping);
		}

		if (_file)
		{
			CloseHandle(_file);
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
			if (!UnmapViewOfFile(_view))
			{
				throw std::system_error(GetLastError(), std::system_category(), "UnmapViewOfFile");
			}

			_view = nullptr;
			_size = 0;
		}

		if (_mapping)
		{
			if (!CloseHandle(_mapping))
			{
				throw std::system_error(GetLastError(), std::system_category(), "CloseHandle");
			}

			_mapping = nullptr;
		}

		if (_file)
		{
			if (!CloseHandle(_file))
			{
				throw std::system_error(GetLastError(), std::system_category(), "CloseHandle");
			}

			_file = nullptr;
		}
	}

private:
	memory_mapped_file_impl(const memory_mapped_file_impl&) = delete;
	memory_mapped_file_impl(memory_mapped_file_impl&&) = delete;
	memory_mapped_file_impl& operator = (const memory_mapped_file_impl&) = delete;
	memory_mapped_file_impl& operator = (memory_mapped_file_impl&&) = delete;

	HANDLE _file = nullptr;
	HANDLE _mapping = nullptr;
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
