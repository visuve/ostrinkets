#include "resource.hpp"

#include <cassert>
#include <iostream>

#include <Windows.h>

HANDLE open_disk_or_file(const std::filesystem::path& path)
{
	constexpr uint32_t desired_access = GENERIC_READ | GENERIC_WRITE;
	constexpr uint32_t share_mode = FILE_SHARE_READ | FILE_SHARE_WRITE;
	constexpr LPSECURITY_ATTRIBUTES security_attributes = nullptr;
	constexpr uint32_t creation_disposition = OPEN_EXISTING;
	constexpr uint32_t creation_flags = FILE_FLAG_RANDOM_ACCESS;
	constexpr HANDLE template_file = nullptr;

	return CreateFileW(
		path.c_str(),
		desired_access,
		share_mode,
		security_attributes,
		creation_disposition,
		creation_flags,
		template_file);
}

OVERLAPPED uint64_to_overlapped(uint64_t offset)
{
	ULARGE_INTEGER cast = {};
	cast.QuadPart = offset;

	OVERLAPPED overlapped = {};
	overlapped.Offset = cast.LowPart;
	overlapped.OffsetHigh = cast.HighPart;

	return overlapped;
}

resource::resource(const std::filesystem::path& path, bool is_disk) :
	m_handle(open_disk_or_file(path.c_str()))
{
	if (!is_valid())
	{
		return;
	}

	if (is_disk)
	{
		DWORD bytes_returned = 0;
		DISK_GEOMETRY disk_geo = {};
		constexpr uint32_t disk_geo_size = sizeof(DISK_GEOMETRY);

		if (!DeviceIoControl(
			m_handle,
			IOCTL_DISK_GET_DRIVE_GEOMETRY,
			nullptr,
			0,
			&disk_geo,
			disk_geo_size,
			&bytes_returned,
			nullptr))
		{
			return;
		}

		/*if (!DeviceIoControl(
			m_handle,
			IOCTL_DISK_DELETE_DRIVE_LAYOUT,
			nullptr,
			0,
			nullptr,
			0,
			nullptr,
			nullptr))
		{
			return;
		}*/

		assert(bytes_returned == disk_geo_size);

		m_size = disk_geo.Cylinders.QuadPart *
			disk_geo.TracksPerCylinder *
			disk_geo.SectorsPerTrack *
			disk_geo.BytesPerSector;
	}
	else
	{
		LARGE_INTEGER file_size = {};

		if (!GetFileSizeEx(m_handle, &file_size))
		{
			return;
		}

		m_size = file_size.QuadPart;
	}

	std::cout << path << " size is " << m_size << " bytes." << std::endl;
}

resource::~resource()
{
	if (is_valid())
	{
		CloseHandle(m_handle);
		m_handle = INVALID_HANDLE_VALUE;
	}
}

bool resource::is_valid() const
{
	return m_handle != nullptr && m_handle != INVALID_HANDLE_VALUE;
}

bool resource::is_empty() const
{
	return m_size <= 0;
}

std::optional<std::byte> resource::read_byte_at(uint64_t offset)
{
	std::byte byte[2] = {};
	DWORD bytes_to_read = 1;
	DWORD bytes_read = 0;
	OVERLAPPED overlapped = uint64_to_overlapped(offset);

	if (!ReadFile(m_handle, &byte, bytes_to_read, &bytes_read, &overlapped))
	{
		return std::nullopt;
	}

	if (bytes_read != 1)
	{
		return std::nullopt;
	}

	return byte[0];
}

bool resource::write_byte_at(uint64_t offset, std::byte byte)
{
	DWORD bytes_written = 0;
	OVERLAPPED overlapped = uint64_to_overlapped(offset);

	if (!WriteFile(m_handle, &byte, 1, &bytes_written, &overlapped))
	{
		return false;
	}

	return bytes_written == 1;
}

bool resource::flush()
{
	return FlushFileBuffers(m_handle);
}