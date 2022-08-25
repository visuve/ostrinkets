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

#ifdef __cpp_lib_format 
#include <format>
#endif

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

	std::streamsize read_random_number_of_bytes(std::ifstream& input, std::span<char> buffer)
	{
		const auto bytes_to_read =
			random_numeric_value<std::streamsize>(size_t(1), buffer.size_bytes());

		input.read(buffer.data(), bytes_to_read);

		return input.gcount();
	}

	void sleep_for_random_time()
	{
		const auto sleep_time = random_numeric_value<std::chrono::milliseconds>(1, 20);

#ifdef __cpp_lib_format 
		std::cout << std::format("{} sleeping for {}",
			std::chrono::system_clock::now(),
			sleep_time) << std::endl;
#else
		std::cout << std::chrono::system_clock::now().time_since_epoch().count()
			<< " sleeping for " << sleep_time.count() << std::endl;
#endif

		std::this_thread::sleep_for(sleep_time);
	}

	void write_and_flush(std::ofstream& output, std::span<char> buffer)
	{
		if (!output.write(buffer.data(), buffer.size_bytes()))
		{
			throw std::ios_base::failure("Failed to write output");
		}

		if (!output.flush())
		{
			throw std::ios_base::failure("Failed to flush output");
		}

#ifdef __cpp_lib_format
		std::cout << std::format("{} wrote {} bytes",
			std::chrono::system_clock::now(),
			buffer.size_bytes()) << std::endl;
#else
		std::cout << std::chrono::system_clock::now().time_since_epoch().count()
			<< " wrote " << buffer.size_bytes() << " bytes" << std::endl;
#endif
	}

	void sloth_copy(
		const std::filesystem::path& input_path,
		const std::filesystem::path& output_path)
	{
		std::ifstream input(input_path, std::ios::binary);
		input.exceptions(std::istream::badbit);

		std::ofstream output(output_path, std::ios::binary);
		output.exceptions(std::istream::failbit | std::istream::badbit);

		std::array<char, 0x200> buffer;

		while (!g_signal && input)
		{
			const std::streamsize bytes = read_random_number_of_bytes(input, buffer);

			if (bytes <= 0)
			{
				throw std::ios_base::failure("No data available");
			}

			if (g_signal)
			{
				return;
			}

			sleep_for_random_time();

			write_and_flush(output, { buffer.begin(), static_cast<size_t>(bytes) });
		}
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

		sloth_copy(argv[1], argv[2]);

		return EXIT_SUCCESS;
	}
	catch (const std::system_error& sys_error)
	{
		std::cerr << "An exception occurred:" << std::endl;
		std::cerr << sys_error.what() << std::endl;
		std::cerr << "Error code: " << sys_error.code().value() << std::endl;
		return sys_error.code().value();
	}
	catch (const std::exception& e)
	{
		std::cerr << "An exception occurred:" << std::endl;
		std::cerr << e.what();
	}

	return EXIT_FAILURE;
}
