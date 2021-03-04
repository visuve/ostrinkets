#include <atomic>
#include <array>
#include <chrono>
#include <csignal>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <span>
#include <string>
#include <thread>

namespace
{
	std::atomic<int> g_signal = 0;

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

	template <size_t N>
	std::streamsize read_random_number_of_bytes(std::ifstream& input, std::array<char, N>& buffer)
	{
		const auto bytes_to_read =
			random_numeric_value<std::streamsize>(size_t(1), buffer.size());

		input.read(buffer.data(), bytes_to_read);

		return input.gcount();
	}

	void sleep_for_random_time()
	{
		const auto sleep_time = random_numeric_value<std::chrono::milliseconds>(1, 20);

		std::cout << since_epoch<std::chrono::seconds>().count()
			<< " sleeping for " << sleep_time.count() << "ms..." << std::endl;

		std::this_thread::sleep_for(sleep_time);
	}

	bool write_and_flush(std::ofstream& output, std::span<char> buffer)
	{
		if (!output.write(buffer.data(), buffer.size_bytes()))
		{
			std::cerr << "Failed to write!" << std::endl;
			return false;
		}

		if (!output.flush())
		{
			std::cerr << "Failed to flush!" << std::endl;
			return false;
		}

		std::cout << since_epoch<std::chrono::seconds>().count()
			<< " wrote " << buffer.size_bytes() << "bytes" << std::endl;

		return true;
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

			std::array<char, 0x200> buffer;

			while (!g_signal && input)
			{
				const std::streamsize bytes = read_random_number_of_bytes(input, buffer);

				if (bytes <= 0)
				{
					return EIO;
				}

				if (g_signal)
				{
					return ECANCELED;
				}

				sleep_for_random_time();

				if (!write_and_flush(output, { buffer.begin(), static_cast<size_t>(bytes) }))
				{
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
		return EXIT_FAILURE;
	}

	try
	{
		if (!std::filesystem::exists(argv[1]))
		{
			std::cerr << "Input: '" << argv[1] << "' does not exist!" << std::endl;
			return ENOENT;
		}

		return sloth_copy(argv[1], argv[2]);
	}
	catch (const std::exception& e)
	{
		std::cerr << "An exception occurred: " << e.what() << std::endl;
	}

	return EXIT_FAILURE;
}
