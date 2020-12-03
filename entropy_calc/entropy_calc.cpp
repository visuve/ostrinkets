#include <array>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>

namespace
{
	double entropy(std::basic_istream<std::byte>& input_stream)
	{
		std::vector<std::byte> buffer(0x10000);
		std::array<uint64_t, 0xFF> frequencies = {};
		std::streamsize total_bytes_read = 0;

		while (input_stream)
		{
			input_stream.read(buffer.data(), buffer.size());

			std::streamsize bytes_read = input_stream.gcount();

			for (std::streamsize i = 0; i < bytes_read; ++i)
			{
				std::byte character = buffer[i];
				++frequencies[static_cast<size_t>(character)];
			}

			total_bytes_read += bytes_read;
		}

		double entropy = 0;

		for (uint64_t value : frequencies)
		{
			double frequency = value / double(total_bytes_read);
			entropy -= frequency * std::log2(frequency);
		}

		return entropy;
	}
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << "<path/to/inputfile>" << std::endl;
		return EINVAL;
	}

	try
	{
		const std::filesystem::path input_path(argv[1]);

		if (!std::filesystem::exists(input_path))
		{
			std::cerr << "Input: '" << input_path << "' does not exist!" << std::endl;
			return ENOENT;
		}

		std::basic_ifstream<std::byte> input_stream(input_path, std::ios::binary);

		if (!input_stream)
		{
			std::cerr << "Failed to open: '" << input_path << "'!" << std::endl;
			return ENOENT;
		}

		std::cout << input_path << '\t' << entropy(input_stream) << std::endl;
	}
	catch (const std::ios::failure& e)
	{
		std::cerr << "An I/O excetion occurred: " << e.what() << std::endl;
		return EIO;
	}
	catch (const std::exception& e)
	{
		std::cerr << "An exception occurred: " << e.what() << std::endl;
		return -1;
	}

	return 0;
}
