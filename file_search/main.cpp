#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <regex>
#include <string_view>

constexpr size_t buffer_size = 0x1000; // 4kib

std::map<uint32_t, std::string> lines_containing(std::istream& input, std::string_view word)
{
	std::map<uint32_t, std::string> results;
	std::array<char, buffer_size> buffer = {};

	for (uint32_t line_number = 1; input.getline(buffer.data(), buffer_size, '\n'); ++line_number)
	{
		const std::streamsize bytes_read = input.gcount();

		if (bytes_read <= 0)
		{
			break;
		}

		const std::string_view view(buffer.data(), static_cast<size_t>(bytes_read));

		if (view.find(word) != std::string::npos)
		{
			results.emplace(line_number, view);
		}
	}

	return results;
}

std::map<uint32_t, std::string> lines_matching(std::istream& input, const std::regex& regex)
{
	std::map<uint32_t, std::string> results;
	std::array<char, buffer_size> buffer = {};

	for (uint32_t line_number = 1; input.getline(buffer.data(), buffer_size, '\n'); ++line_number)
	{
		const std::streamsize bytes_read = input.gcount();

		if (bytes_read <= 0)
		{
			break;
		}

		const std::string_view view(buffer.data(), static_cast<size_t>(bytes_read));

		if (std::regex_search(view.cbegin(), view.cend(), regex))
		{
			results.emplace(line_number, view);
		}
	}

	return results;
}

void search(
		const std::filesystem::path& path,
		const std::function<std::map<uint32_t, std::string>(std::istream&)>& search_function)
{
	for (const auto& iter : std::filesystem::recursive_directory_iterator(path))
	{
		const std::filesystem::path& file_path = iter.path();

		try
		{
			std::ifstream file(file_path);

			for (const auto& [line_number, line] : search_function(file))
			{
				std::cout << file_path << ':' << line_number << ':' << line << std::endl;
			}
		}
		catch (const std::exception& e)
		{
			std::cerr << "Failed to process: " << file_path << ": " << e.what() << std::endl;
		}
	}
}

void print_usage(const std::filesystem::path& executable)
{
	std::cout << "Usage: " << executable << " <folder> <mode> <expression>" << std::endl;
}

int main(int argc, char** argv)
{
	if (argc < 4)
	{
		print_usage(argv[0]);
		return EINVAL;
	}

	const std::filesystem::path path(argv[1]);
	const std::string mode(argv[2]);

	if (mode == "plain")
	{
		const std::string search_word(argv[3]);
		search(path, std::bind(lines_containing, std::placeholders::_1, search_word));
	}
	else if(mode == "regex")
	{
		const std::regex regex(argv[3], std::regex::grep | std::regex::icase);
		search(path, std::bind(lines_matching, std::placeholders::_1, regex));
	}
	else
	{
		print_usage(argv[0]);
		return EINVAL;
	}

	return 0;
}
