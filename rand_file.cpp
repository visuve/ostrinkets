#include <algorithm>
#include <atomic>
#include <chrono>
#include <csignal>
#include <filesystem>
#include <iostream>
#include <random>
#include <set>
#include <thread>

namespace
{
	namespace fs = std::filesystem;

	std::set<fs::path> list_directory(
		const fs::path& source,
		bool recursive,
		fs::file_type type,
		fs::perms permissions)
	{
		const auto filter = [type, permissions](auto&& iter)
		{
			std::set<fs::path> paths;

			std::cout << "Loading...";

			for (const fs::path& path : iter)
			{
				const fs::file_status status = fs::status(path);

				if (status.type() != type)
				{
					continue;
				}

				if ((status.permissions() & permissions) == fs::perms::none)
				{
					continue;
				}

				paths.insert(path);

				if (!(paths.size() % 10))
				{
					std::cout << '.';
				}
			}

			std::cout << std::endl;
			return paths;
		};

		if (recursive)
		{
			return filter(fs::recursive_directory_iterator(source));
		}

		return filter(fs::directory_iterator(source));
	}

	template <typename T>
	std::vector<T> set_to_shuffled_vector(const std::set<T>& set)
	{
		std::vector<T> vector(set.cbegin(), set.cend());
		std::random_device device;
		std::mt19937 engine(device());
		std::shuffle(vector.begin(), vector.end(), engine);
		return vector;
	}

	std::atomic<int> g_signal_status = 0;

	void signal_handler(int signal)
	{
		g_signal_status = signal;
	}
}

int main(int argc, char* argv[])
{
	const int status = std::system(nullptr);

	if (status == 0)
	{
		std::cerr << "Command processor not available!" << std::endl;
		return status;
	}

	if (std::signal(SIGINT, signal_handler) == SIG_ERR)
	{
		std::cerr << "Cannot attach SIGINT handler!" << std::endl;
		return EXIT_FAILURE;
	}

	try
	{
		const fs::path dir = argc > 1 ? argv[1] : fs::path(argv[0]).parent_path();

		if (!fs::exists(dir))
		{
			std::cerr << dir << " does not exist!" << std::endl;
			return EXIT_FAILURE;
		}

		auto file_set = list_directory(dir, true, fs::file_type::regular, fs::perms::owner_exec);

		if (file_set.empty())
		{
			std::cerr << "Missing access rights or " << dir << " is empty! !" << std::endl;
			return EXIT_FAILURE;
		}

		std::cout << "Processing " << dir << " with " << file_set.size() << " item(s)" << std::endl;
		const auto shuffled_files = set_to_shuffled_vector(file_set);
		file_set.clear();
		std::cout << "Press enter to continue, CTRL + C to exit" << std::endl;

		for (const fs::path& path : shuffled_files)
		{
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
			std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Here be dragons

			if (g_signal_status)
			{
				std::cerr << "Signaled: " << g_signal_status << std::endl;
				return g_signal_status;
			}

			const std::string padded = '"' + path.string() + '"';
			const int result = std::system(padded.c_str());
			std::cout << "Opened " << padded << " with result: " << result << std::flush;
		}
	}
	catch (const std::exception& e)
	{
		std::cerr << "An exception occurred: " << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}