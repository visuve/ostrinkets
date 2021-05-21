#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>
#include <vector>

constexpr size_t buffer_size = 0x10000; // 64kib

std::vector<size_t> lines_containing(std::istream& input, const std::string& word)
{
	std::vector<size_t> results;
	std::string buffer(buffer_size, '\0');

	for (size_t line_number = 1; input.getline(buffer.data(), buffer_size, '\n'); ++line_number)
	{
		const std::streamsize bytes_read = input.gcount();

		if (bytes_read <= 0)
		{
			break;
		}

		const std::string_view view(buffer.data(), static_cast<size_t>(bytes_read));

		if (view.find(word) != std::string::npos)
		{
			results.push_back(line_number);
		}
	}

	return results;
}

int main(int argc, char** argv)
{
	if (argc < 3)
	{
		std::cout << "Usage: " << argv[0] << " <target>" << std::endl;
		return EINVAL;
	}

	const std::filesystem::path path(argv[1]);
	const std::string word(argv[2]);

	for (auto file_path : std::filesystem::recursive_directory_iterator(path))
	{
		try
		{
			std::ifstream file(file_path.path());

			for (size_t line_number : lines_containing(file, word))
			{
				std::cout << file_path << ':' << line_number << std::endl;
			}
		}
		catch (const std::exception& e)
		{
			std::cerr << "An exception occurred: " << e.what() << std::endl;
		}
	}

	return 0;
}
