#include <algorithm>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <regex>
#include <string_view>

// TODO: this will fail if the searched item is between two buffer windows

void replace(
	std::ifstream& input_stream,
	std::ofstream& output_stream,
	const std::function<void(std::string&)>& replace_function)
{
	std::string buffer(0x1000, '\0'); // 4kib

	do
	{
		input_stream.read(buffer.data(), buffer.size());

		const std::streamsize bytes_read = input_stream.gcount();

		if (bytes_read <= 0)
		{
			break;
		}

		if (static_cast<size_t>(bytes_read) < buffer.size())
		{
			buffer.resize(static_cast<size_t>(bytes_read));
		}

		replace_function(buffer);

		output_stream << buffer;
	} while (input_stream);
}

void replace(
		const std::filesystem::path& file_path,
		const std::function<void(std::string&)>& replace_function)
{
	try
	{
		std::filesystem::path tmp_file_path(file_path.string() + ".tmp");

		std::ifstream input_stream(file_path);
		std::ofstream output_stream(tmp_file_path, std::ios::trunc);

		input_stream.exceptions(std::ios::badbit);
		output_stream.exceptions(std::istream::failbit | std::istream::badbit);

		replace(input_stream, output_stream, replace_function);

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
