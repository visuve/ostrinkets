#include <functional>
#include <fstream>
#include <iostream>
#include <regex>

#include "memory_mapped_file.hpp"

struct match
{
	size_t begin = 0;
	size_t end = 0;
};

std::vector<match> find_all_plain(std::string_view haystack, std::string_view needle)
{
	std::vector<match> result;

	auto searcher = std::boyer_moore_horspool_searcher(needle.cbegin(), needle.cend());

	auto it = std::search(haystack.cbegin(), haystack.cend(), searcher);

	while (it != haystack.cend())
	{
		auto match_begin = it - haystack.cbegin();
		auto match_end = match_begin + needle.size();
		result.emplace_back(match_begin, match_end);

		it = std::search(it + needle.size(), haystack.cend(), searcher);
	}

	return result;
}

std::vector<match> find_all_regex(std::string_view haystack, const std::string needle)
{
	std::vector<match> result;

	const std::regex regex(needle, std::regex::grep);

	std::regex_token_iterator<std::string_view::iterator> it(haystack.cbegin(), haystack.cend(), regex);
	const std::regex_token_iterator<std::string_view::iterator> end;

	while (it != end)
	{
		const std::sub_match match = *it;

		auto match_begin = match.first - haystack.cbegin();
		auto match_end =  match_begin + match.length();
		result.emplace_back(match_begin, match_end);

		++it;
	}

	return result;
}

size_t replace_all(
		const std::filesystem::path& file_path,
		std::function<std::vector<match>(std::string_view)> search_function,
		std::string_view replacement)
{
	size_t replaced_count = 0;

	try
	{
		std::filesystem::path wrk_file_path(file_path.string() + ".tmp");
		std::ofstream output_stream(wrk_file_path, std::ios::binary | std::ios::trunc);
		output_stream.exceptions(std::istream::failbit | std::istream::badbit);

		memory_mapped_file mmf(file_path);

		std::string_view contents = mmf.data();

		std::vector<match> matches = search_function(contents);

		auto match = matches.cbegin();

		size_t offset = 0;

		while (offset < contents.size())
		{
			if (match != matches.cend())
			{
				const auto [match_begin, match_end] = *match;
				const size_t match_size = match_end - match_begin;
				const size_t chunk_size = std::max(match_begin, offset) - std::min(match_begin, offset);

				if (chunk_size)
				{
					auto chunk = contents.substr(offset, chunk_size);
					output_stream.write(chunk.data(), chunk_size);
					offset += chunk_size;
				}

				output_stream.write(replacement.data(), replacement.size());
				offset += match_size;

				++match;
				++replaced_count;
			}
			else
			{
				size_t size = contents.size() - offset;
				auto chunk = contents.substr(offset, size);
				output_stream.write(chunk.data(), chunk.size());

				offset += size;
			}

#ifdef _DEBUG
			output_stream.flush();
#endif
		}

		output_stream.close();
		mmf.close();

		std::filesystem::rename(file_path, file_path.string() + ".bak");
		std::filesystem::rename(wrk_file_path, file_path);
	}
	catch (const std::exception& e)
	{
		std::cerr << "\nAn exception occurred: " << e.what() << std::endl;
	}

	return replaced_count;
}

void print_usage(const std::filesystem::path& executable)
{
	std::cout << "Usage: " << executable << " <file> <search mode> <search expression> <replacement>" << std::endl;
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

	const auto begin = std::chrono::high_resolution_clock::now();
	size_t count = 0;

	if (mode == "plain")
	{
		const auto find_function = std::bind(find_all_plain, std::placeholders::_1, search_expression);

		 count = replace_all(path, find_function, replacement);
	}
	else if (mode == "regex")
	{
		const auto find_function = std::bind(find_all_regex, std::placeholders::_1, search_expression);

		count = replace_all(path, find_function, replacement);
	}
	else
	{
		print_usage(argv[0]);
		return EINVAL;
	}

	const auto end = std::chrono::high_resolution_clock::now();
	const auto diff = end - begin;

	std::cout << "Replaced " << count << " occurrences. Took: " << std::format("{:%T}\n", diff);

	return 0;
}
