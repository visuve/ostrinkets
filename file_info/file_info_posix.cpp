#include "file_info.hpp"

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/statvfs.h>
#include <unistd.h>

#if defined(__linux__)
#include <mntent.h>
#else
#include <sys/mount.h>
#endif

#include <iostream>

namespace ostrinkets
{
	struct auto_descriptor
	{
		auto_descriptor(int descriptor) :
			_descriptor(descriptor)
		{
		}

		~auto_descriptor()
		{
			if (_descriptor != -1)
			{
				close(_descriptor);
			}
		}

		operator int() const
		{
			return _descriptor;
		}

	private:
		int _descriptor;
	};

	std::ostream& operator << (std::ostream& out, const timespec& time)
	{
		return out << time.tv_sec << '.' << time.tv_sec;
	}

	void print_file_info(const std::filesystem::path& path)
	{
		constexpr uint32_t open_flags = O_RDONLY;
		auto_descriptor descriptor(open(path.c_str(), open_flags));

		[[maybe_unused]]
		dev_t device_number = 0;

		if (descriptor == -1)
		{
			const std::string message("Failed to open " + path.string());
			throw std::system_error(errno, std::system_category(), message);
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

				device_number = info.st_dev;
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
#if defined(__linux__)
		{
			FILE* file = setmntent("/proc/mounts", "r");

			if (!file)
			{
				return;
			}

			dev_t guess = 1;
			struct mntent* mount_table_entry = nullptr;

			do
			{
				++guess;
				mount_table_entry = getmntent(file);

				if (guess == device_number && mount_table_entry)
				{
					std::cout << "\tgetmntent:" << std::endl;
					std::cout << "\t\tmnt_fsname: " << mount_table_entry->mnt_fsname << std::endl;
					std::cout << "\t\tmnt_dir:" << mount_table_entry->mnt_dir << std::endl;
					std::cout << "\t\tmnt_type: " << mount_table_entry->mnt_type << std::endl;
					std::cout << "\t\tmnt_opts: " << mount_table_entry->mnt_opts << std::endl;
					std::cout << "\t\tmnt_freq: " << mount_table_entry->mnt_freq << std::endl;
					std::cout << "\t\tmnt_passno: " << mount_table_entry->mnt_passno << std::endl;
					std::cout << std::endl;
				}
			}
			while (guess < device_number && mount_table_entry);

			endmntent(file);
		}
#else
		{
			struct statfs* mntbuf = {};
			int mntsize = getmntinfo(&mntbuf, MNT_NOWAIT);
			
			std::string_view absolute = std::filesystem::absolute(path).string();

			for (int i = 0; i < mntsize; ++i)
			{
				if (!absolute.starts_with(mntbuf[i].f_mntonname))
				{
					continue;
				}

				std::cout << "\tgetmntinfo:" << std::endl;
				std::cout << "\t\tf_version: " << mntbuf[i].f_version << std::endl;
				std::cout << "\t\tf_type: " << mntbuf[i].f_type << std::endl;
				std::cout << "\t\tf_flags: " << mntbuf[i].f_flags<< std::endl;
				std::cout << "\t\tf_bsize: " << mntbuf[i].f_bsize << std::endl;
				std::cout << "\t\tf_iosize: " << mntbuf[i].f_iosize << std::endl;
				std::cout << "\t\tf_blocks: " << mntbuf[i].f_blocks << std::endl;
				std::cout << "\t\tf_bfree: " << mntbuf[i].f_bfree << std::endl;
				std::cout << "\t\tf_bavail: " << mntbuf[i].f_bavail << std::endl;
				std::cout << "\t\tf_files: " << mntbuf[i].f_files << std::endl;
				std::cout << "\t\tf_ffree: " << mntbuf[i].f_ffree << std::endl;
				std::cout << "\t\tf_syncwrites: " << mntbuf[i].f_syncwrites << std::endl;
				std::cout << "\t\tf_asyncwrites: " << mntbuf[i].f_asyncwrites << std::endl;
				std::cout << "\t\tf_syncreads: " << mntbuf[i].f_syncreads << std::endl;
				std::cout << "\t\tf_asyncreads: " << mntbuf[i].f_asyncreads << std::endl;
				std::cout << "\t\tf_spare: " << mntbuf[i].f_spare << std::endl;
				std::cout << "\t\tf_namemax: " << mntbuf[i].f_namemax << std::endl;
				std::cout << "\t\tf_owner: " << mntbuf[i].f_owner << std::endl;
				// std::cout << "\t\tf_fsid: " << mntbuf[i].f_fsid << std::endl;
				std::cout << "\t\tf_charspare: " <<  mntbuf[i].f_charspare << std::endl;
				std::cout << "\t\tf_fstypename: " << mntbuf[i].f_fstypename << std::endl;
				std::cout << "\t\tf_mntfromname: " << mntbuf[i].f_mntfromname << std::endl;
				std::cout << "\t\tf_mntonname: " <<  mntbuf[i].f_mntonname << std::endl;

				break;
			}
		}
#endif
	}
}
