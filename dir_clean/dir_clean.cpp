#include <atomic>
#include <csignal>
#include <filesystem>
#include <iostream>

namespace
{
	namespace fs = std::filesystem;

	std::atomic<int> g_signal_status = 0;

	void signal_handler(int signal)
	{
		g_signal_status = signal;
	}

	uintmax_t remove_entry(const fs::directory_entry& entry)
	{
		try
		{
			switch (entry.status().type())
			{
				case fs::file_type::regular:
				{
					return fs::remove(entry) ? 1 : 0;
				}
				case fs::file_type::directory:
				{
					return fs::remove_all(entry);
				}
				default:
				{
					std::cerr << " -> skipped, invalid type: "
						<< static_cast<int>(entry.status().type()) << '.' << std::endl;
				}
			}
		}
		catch (const fs::filesystem_error& e)
		{
			std::cerr << " -> skipped, exception: " << e.what() << std::endl;
		}

		return 0;
	}

	void clean(const fs::path& path, const std::string& to_remove)
	{
		auto iter = fs::recursive_directory_iterator(path);
		uintmax_t removed_total = 0;

		for (auto& entry : iter)
		{
			if (g_signal_status)
			{
				std::cerr << "Signaled: " << g_signal_status << std::endl;
				break;
			}

			std::cout << entry;

			if (entry.path().filename() != to_remove)
			{
				std::cout << " -> skipped." << std::endl;
				continue;
			}

			if ((entry.status().permissions() & fs::perms::owner_exec) == fs::perms::none)
			{
				std::cout << " -> skipped, no perms." << std::endl;
				continue;
			}

			uintmax_t removed = remove_entry(entry);

			if (!removed)
			{
				std::cout << " -> remove failed." << std::endl;
				continue;
			}

			removed_total += removed;
			iter.disable_recursion_pending();
			std::cout << " -> removed." << std::endl;
		}

		std::cout << "Done. Removed: " << removed_total << " items." << std::endl;
	}
}

int main(int argc, char* argv[])
{
	if (std::signal(SIGINT, signal_handler) == SIG_ERR)
	{
		std::cerr << "Cannot attach SIGINT handler!" << std::endl;
		return EXIT_FAILURE;
	}

	if (argc < 3)
	{
		std::cerr << "Usage:" << std::endl;
		std::cerr << argv[0] << " <starting-location>" << 
			" <file-or-folder-name-to-recursively-delete>" << std::endl;
		return EXIT_FAILURE;
	}

	try
	{
		const fs::path start_dir = fs::path(argv[1]);

		if (!fs::exists(start_dir))
		{
			std::cerr << start_dir << " does not exist!";
			return EXIT_FAILURE;
		}

		clean(start_dir, argv[2]);
	}
	catch (const std::exception& e)
	{
		std::cerr << "An exception occurred: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}