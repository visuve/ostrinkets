#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>

class resource
{
public:
	resource(const std::filesystem::path& path, bool is_disk);
	virtual ~resource();

	bool is_valid() const;
	bool is_empty() const;

	std::optional<std::byte> read_byte_at(uint64_t offset);
	bool write_byte_at(uint64_t offset, std::byte byte);
	bool flip_random_bit();
	bool flush();

private:
#ifdef _WIN32
	void* _handle = nullptr;
#else
	int _descriptor = -1;
#endif
	uint64_t _size = 0;
};
