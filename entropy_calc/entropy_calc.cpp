#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <vector>

namespace
{
	double entropy(std::basic_ifstream<uint8_t>& input_stream)
	{
		std::vector<uint8_t> buffer(0x10000);
		std::map<uint8_t, uint64_t> frequencies;
		std::streamsize total_bytes_read = 0;

		while (input_stream.read(buffer.data(), buffer.size()))
		{
			std::streamsize bytes_read = input_stream.gcount();

			for (std::streamsize i = 0; i < bytes_read; ++i)
			{
				uint8_t character = buffer[i];
				++frequencies[character];
			}

			total_bytes_read += bytes_read;
		}

		double entropy = 0;

		for (const auto& [key, value] : frequencies)
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

		std::basic_ifstream<uint8_t> input_stream(input_path, std::ios::binary);

		if (!input_stream)
		{
			std::cerr << "Failed to open: '" << input_path << "'!" << std::endl;
			return ENOENT;
		}


		std::cout << input_path << '\t' << entropy(input_stream) << std::endl;
		return 0;
	}
	catch (const std::ios::failure& e)
	{
		std::cerr << "An I/O excetion occurred: " << e.what() << std::endl;
	}
	catch (const std::exception& e)
	{
		std::cerr << "An exception occurred: " << e.what() << std::endl;
	}

	return EIO;
}
