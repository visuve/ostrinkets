#include <atomic>
#include <bitset>
#include <cassert>
#include <csignal>
#include <cstddef>
#include <filesystem>
#include <chrono>
#include <iostream>
#include <optional>
#include <random>
#include <thread>

namespace
{

	std::atomic<int> g_signal;

	void signal_handler(int signal)
	{
		g_signal = signal;
	}

	template<typename T, typename N>
	T random_numeric_value(
		N min = std::numeric_limits<N>::min(),
		N max = std::numeric_limits<N>::max())
	{
		thread_local std::random_device device;
		thread_local std::mt19937_64 engine(device());
		thread_local std::uniform_int_distribution<N> distribution(min, max);

		return T(distribution(engine));
	}

	template <typename T>
	T since_epoch()
	{
		auto now = std::chrono::system_clock::now();
		return std::chrono::duration_cast<T>(now.time_since_epoch());
	}

	void sleep_for_random_time()
	{
		const auto sleep_time = random_numeric_value<std::chrono::milliseconds>(1, 20);

		std::cout << since_epoch<std::chrono::seconds>().count()
			<< " Sleeping for " << sleep_time.count() << "ms..." << std::endl;

		std::this_thread::sleep_for(sleep_time);
	}
}

#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#define last_error GetLastError()

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

	class resource
	{
	public:
		resource(const std::filesystem::path& path, bool is_disk) :
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

		~resource()
		{
			if (is_valid())
			{
				CloseHandle(m_handle);
				m_handle = INVALID_HANDLE_VALUE;
			}
		}

		bool is_valid() const
		{
			return m_handle != nullptr && m_handle != INVALID_HANDLE_VALUE;
		}

		bool is_empty() const
		{
			return m_size <= 0;
		}

		std::optional<std::byte> read_byte_at(uint64_t offset)
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

		bool write_byte_at(uint64_t offset, std::byte byte)
		{
			DWORD bytes_written = 0;
			OVERLAPPED overlapped = uint64_to_overlapped(offset);

			if (!WriteFile(m_handle, &byte, 1, &bytes_written, &overlapped))
			{
				return false;
			}

			return bytes_written == 1;
		}

		bool flush()
		{
			return FlushFileBuffers(m_handle);
		}

		bool flip_random_bit();

	private:
		HANDLE m_handle = INVALID_HANDLE_VALUE;
		uint64_t m_size = 0;
	};
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#define last_error errno

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


	class resource
	{
	public:
		resource(const std::filesystem::path& path, bool is_disk) :
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

		~resource()
		{
			if (is_valid())
			{
				close(m_descriptor);
				m_descriptor = -1;
			}
		}

		bool is_valid()
		{
			return m_descriptor > 0;
		}

		bool is_empty()
		{
			return m_size <= 0;
		}

		std::optional<std::byte> read_byte_at(uint64_t offset)
		{
			std::byte byte = { };

			const ssize_t bytes_read = pread(m_descriptor, &byte, 1, offset);

			if (bytes_read != 1)
			{
				return std::nullopt;
			}

			return byte;
		}

		bool write_byte_at(uint64_t offset, std::byte byte)
		{
			const ssize_t bytes_written = pwrite(m_descriptor, &byte, 1, offset);
			return bytes_written == 1;
		}

		bool flush()
		{
			return fsync(m_descriptor) != -1;
		}

		bool flip_random_bit();

	private:
		int m_descriptor = -1;
		uint64_t m_size = 0;
	};
#endif

bool resource::flip_random_bit()
{
	const uint64_t offset = random_numeric_value<uint64_t>(uint64_t(0), m_size - 1);
	std::optional<std::byte> random_source_byte = read_byte_at(offset);

	if (!random_source_byte.has_value())
	{
		std::cout << since_epoch<std::chrono::seconds>().count()
			<< " Failed to read @ " << offset << std::endl;
		return false;
	}

	const auto random_source_byte_value = std::to_integer<uint32_t>(random_source_byte.value());

	std::bitset<8> random_bits(random_source_byte_value);
	const size_t random_bit_index = random_numeric_value<size_t>(0, 7);
	random_bits.flip(random_bit_index);

	const uint32_t shuffled_value = random_bits.to_ulong();
	std::byte shuffled_byte = static_cast<std::byte>(shuffled_value);

	if (!write_byte_at(offset, shuffled_byte))
	{
		std::cout << since_epoch<std::chrono::seconds>().count()
			<< " Failed to write @ " << offset << std::endl;

		return false;
	}

	if (!flush())
	{
		return false;
	}

#ifdef _WIN32
	printf("%lld Flipped 0x%.2X -> 0x%.2X @ %llu\n",
		since_epoch<std::chrono::seconds>().count(),
		random_source_byte_value,
		shuffled_value,
		offset);
#else
	printf("%ld Flipped 0x%.2X -> 0x%.2X @ %lu\n",
		since_epoch<std::chrono::seconds>().count(),
		random_source_byte_value,
		shuffled_value,
		offset);
#endif 

	return true;
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cout << "Usage: " << argv[0] << " <target>" << std::endl;
		return EINVAL;
	}

	if (std::signal(SIGINT, signal_handler) == SIG_ERR)
	{
		std::cerr << "Cannot attach SIGINT handler!" << std::endl;
		return EXIT_FAILURE;
	}

	resource target_resource(argv[1], false); // TODO: remove hardcoded value

	if (!target_resource.is_valid())
	{
		std::cout << "Failed to open: " << argv[1] << std::endl;
		return last_error;
	}

	if (target_resource.is_empty())
	{
		std::cout << argv[1] << " appears empty!" << std::endl;
		return last_error;
	}

	while (!g_signal)
	{
		if (!target_resource.flip_random_bit())
		{
			return last_error;
		}

		sleep_for_random_time();
	}

	return 0;
}
