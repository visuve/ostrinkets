#include "resource.hpp"

#include <iostream>

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#if defined(__linux__)
#include <linux/fs.h>
using FileInfo = struct stat64;
constexpr auto FileInfoFunction = fstat64;
constexpr unsigned long int disk_size_request = BLKGETSIZE64;
#else
using FileInfo = struct stat;
constexpr auto FileInfoFunction = fstat;
constexpr unsigned long int disk_size_request = DIOCGMEDIASIZE;
#endif

resource::resource(const std::filesystem::path& path, bool is_disk) :
	m_descriptor(open(path.c_str(), O_RDWR))
{

	if (!is_valid())
	{
		return;
	}

	if (is_disk)
	{
		if (ioctl(m_descriptor, disk_size_request, &m_size) != 0)
		{
			m_size = 0;
			return;
		}
	}
	else
	{
		FileInfo fileInfo = {};

		if (FileInfoFunction(m_descriptor, &fileInfo) != 0)
		{
			return;
		}

		m_size = fileInfo.st_size;
	}

	std::cout << path << " size is " << m_size << " bytes." << std::endl;
}

resource::~resource()
{
	if (is_valid())
	{
		close(m_descriptor);
		m_descriptor = -1;
	}
}

bool resource::is_valid() const
{
	return m_descriptor > 0;
}

bool resource::is_empty() const
{
	return m_size <= 0;
}

std::optional<std::byte> resource::read_byte_at(uint64_t offset)
{
	std::byte byte = {};

	const ssize_t bytes_read = pread(m_descriptor, &byte, 1, offset);

	if (bytes_read != 1)
	{
		return std::nullopt;
	}

	return byte;
}

bool resource::write_byte_at(uint64_t offset, std::byte byte)
{
	const ssize_t bytes_written = pwrite(m_descriptor, &byte, 1, offset);
	return bytes_written == 1;
}

bool resource::flush()
{
	return fsync(m_descriptor) != -1;
}