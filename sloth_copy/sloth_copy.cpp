#include <atomic>
#include <chrono>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <thread>
#include <unistd.h>

namespace
{
	std::atomic<int> g_signal = 0;

	void signal_handler(int signal)
	{
		g_signal = signal;
	}

	template <typename T>
	T since_epoch()
	{
		auto now = std::chrono::system_clock::now();
		return std::chrono::duration_cast<T>(now.time_since_epoch());
	}

	int sloth_copy(
		const std::filesystem::path& input_path,
		const std::filesystem::path& output_path)
	{
		try
		{
			std::ifstream input(input_path, std::ios::binary);
			input.exceptions(std::istream::badbit);

			if (!input)
			{
				std::cerr << "Failed to open: '" << input_path << "' for reading!" << std::endl;
				return EIO;
			}

			std::ofstream output(output_path, std::ios::binary);
			output.exceptions(std::istream::failbit | std::istream::badbit);

			if (!output)
			{
				std::cerr << "Failed to open: '" << output_path << "' for writing!" << std::endl;
				return EIO;
			}

			for (std::string line; std::getline(input, line, ' ');)
			{
				if (g_signal)
				{
					return ECANCELED;
				}

				const auto sleep_time = std::chrono::milliseconds(line.length() * 100);

				std::cout
						<< since_epoch<std::chrono::seconds>().count()
						<< " sleeping for "
						<< sleep_time.count() << "ms..."
						<< std::endl;

				std::this_thread::sleep_for(sleep_time);
				output << line << ' ';

				if (!output.flush())
				{
					std::cerr << "Failed to flush!" << std::endl;
					return EIO;
				}
			}

			return EXIT_SUCCESS;
		}
		catch (const std::ios::failure& e)
		{
			std::cerr << "An I/O excetion occurred: " << e.what() << std::endl;
		}

		return EIO;
	}
}

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		std::cerr << "Usage : " << argv[0]
			<< " <path/to/inputfile> <path/to/outputfile>" << std::endl;
		return EINVAL;
	}

	if (std::signal(SIGINT, signal_handler) == SIG_ERR)
	{
		std::cerr << "Cannot attach SIGINT handler!" << std::endl;
		return -1;
	}

	try
	{
		if (!std::filesystem::exists(argv[1]))
		{
			std::cout << "Input: '" << argv[1] << "' does not exist!" << std::endl;
			return ENOENT;
		}

		return sloth_copy(argv[1], argv[2]);

	} catch (const std::exception& e)
	{
		std::cerr << "An exception occurred: " << e.what() << std::endl;
	}

	return -1;
}
