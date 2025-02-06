#pragma once

#include <filesystem>
#include <string_view>

class memory_mapped_file_impl;

class memory_mapped_file
{
public:
	memory_mapped_file(const std::filesystem::path& path);
	~memory_mapped_file();

	std::string_view data() const;
	void close();

private:
	memory_mapped_file(const memory_mapped_file&) = delete;
	memory_mapped_file(memory_mapped_file&&) = delete;
	memory_mapped_file& operator = (const memory_mapped_file&) = delete;
	memory_mapped_file& operator = (memory_mapped_file&&) = delete;

	memory_mapped_file_impl* _impl;
};
