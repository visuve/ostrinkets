#include "file_info.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>

#include <iostream>

namespace fstrinkets
{
	struct auto_descriptor
	{
		auto_descriptor(int descriptor) :
			m_descriptor(descriptor)
		{
		}

		~auto_descriptor()
		{
			if (m_descriptor != -1)
			{
				close(m_descriptor);
			}
		}

		operator int()
		{
			return m_descriptor;
		}

	private:
		int m_descriptor;
	};

	std::ostream& operator << (std::ostream& out, const timespec& time)
	{
		return out << time.tv_sec << '.' << time.tv_sec;
	}

	void print_file_info(const std::filesystem::path& path)
	{
		constexpr uint32_t open_flags = O_RDONLY;
		auto_descriptor descriptor(open(path.c_str(), open_flags));

		if (descriptor == -1)
		{
			std::cerr << "Failed to open '" << path << "'." << std::endl;
			return;
		}

		std::cout << path << ':' << std::endl;

		{
#if defined(__linux__)
			struct stat64 info = {};
			if (fstat64(descriptor, &info) != -1)
#else
			struct stat info = {};
			if (fstat(descriptor, &info) != -1)
#endif
			{
				std::cout << "\tfstat64:" << std::endl;
				std::cout << "\t\tst_dev: " << info.st_dev << std::endl;
				std::cout << "\t\tst_ino: " << info.st_ino << std::endl;
				std::cout << "\t\tst_nlink: " << info.st_nlink << std::endl;
				std::cout << "\t\tst_mode: " << info.st_mode << std::endl;
				std::cout << "\t\tst_uid: " << info.st_uid << std::endl;
				std::cout << "\t\tst_gid: " << info.st_gid << std::endl;
				std::cout << "\t\tst_rdev: " << info.st_rdev << std::endl;
				std::cout << "\t\tst_size: " << info.st_size << std::endl;
				std::cout << "\t\tst_blksize: " << info.st_blksize << std::endl;
				std::cout << "\t\tst_blocks: " << info.st_blocks << std::endl;
				std::cout << "\t\tst_atim: " << info.st_atim << std::endl;
				std::cout << "\t\tst_mtim: " << info.st_mtim << std::endl;
				std::cout << "\t\tst_ctim: " << info.st_ctim << std::endl;
				std::cout << std::endl;
			}
		}
		{
#if defined(__linux__)
			struct statvfs64 info = {};
			if (fstatvfs64(descriptor, &info) != -1)
#else
			struct statvfs info = {};
			if (fstatvfs(descriptor, &info) != -1)
#endif
			{
				std::cout << "\tfstatvfs64:" << std::endl;
				std::cout << "\t\tf_bsize: " << info.f_bsize << std::endl;
				std::cout << "\t\tf_frsize: " << info.f_frsize << std::endl;
				std::cout << "\t\tf_blocks: " << info.f_blocks << std::endl;
				std::cout << "\t\tff_bfree: " << info.f_bfree << std::endl;
				std::cout << "\t\tf_bavail: " << info.f_bavail << std::endl;
				std::cout << "\t\tf_files: " << info.f_files << std::endl;
				std::cout << "\t\tf_ffree: " << info.f_ffree << std::endl;
				std::cout << "\t\tf_favail: " << info.f_favail << std::endl;
				std::cout << "\t\tf_fsid: " << info.f_fsid << std::endl;
				std::cout << "\t\tf_flag: " << info.f_flag << std::endl;
				std::cout << "\t\tf_namemax: " << info.f_namemax << std::endl;
				std::cout << std::endl;
			}
		}
	}
}
