#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <regex>
#include <string_view>

constexpr size_t buffer_size = 0x1000; // 4kib

void replace(
		const std::filesystem::path& file_path,
		const std::function<void(std::string&)>& replace_function)
{
	try
	{
		std::array<char, buffer_size> buffer = {};
		std::filesystem::path tmp_file_path(file_path.string() + ".tmp");

		std::ifstream input_stream(file_path);
		std::ofstream output_stream(tmp_file_path, std::ios::trunc);

		input_stream.exceptions(std::ios::badbit);
		output_stream.exceptions(std::istream::failbit | std::istream::badbit);

		std::string chunk;

		do
		{
			input_stream.read(buffer.data(), buffer_size);

			const std::streamsize bytes_read = input_stream.gcount();

			if (bytes_read <= 0)
			{
				break;
			}

			chunk.assign(buffer.data(), static_cast<size_t>(bytes_read));

			replace_function(chunk);

			output_stream.write(chunk.c_str(), chunk.size());
		} while (input_stream);

		input_stream.close();
		output_stream.close();

		if (std::filesystem::remove(file_path))
		{
			std::filesystem::rename(tmp_file_path, file_path);
		}

	}
	catch (const std::exception& e)
	{
		std::cerr << "An exception occurred: " << e.what() << std::endl;
	}
}

void print_usage(const std::filesystem::path& executable)
{
	std::cout << "Usage: " << executable << " <file> <mode> <search_expression> <replacement>" << std::endl;
}

int main(int argc, char** argv)
{
	if (argc < 5)
	{
		print_usage(argv[0]);
		return EINVAL;
	}

	const std::filesystem::path path(argv[1]);
	const std::string mode(argv[2]);
	const std::string search_expression(argv[3]);
	const std::string replacement(argv[4]);

	if (mode == "plain")
	{
		replace(path, [=](std::string& text)
		{
			for (size_t iter = text.find(search_expression);
				 iter != std::string::npos;
				 iter = text.find(search_expression, iter + search_expression.size()))
			{
				text.replace(iter, search_expression.size(), replacement);
			}
		});
	}
	else if(mode == "regex")
	{
		const std::regex regex(argv[3], std::regex::grep | std::regex::icase);

		replace(path, [=](std::string& text)
		{
			text = std::regex_replace(text, regex, replacement);
		});
	}
	else
	{
		print_usage(argv[0]);
		return EINVAL;
	}

	return 0;
}
