#pragma once

#include <atomic>
#include <filesystem>
#include <functional>
#include <unordered_map>

enum class change_type : uint8_t
{
	created = 'c',
	modified = 'm',
	removed = 'r'
};

class file_watcher
{
public:
	explicit file_watcher(const std::filesystem::path& path);
	void start(const std::function<void()>& callback);
	void stop();

private:
	std::atomic<bool> _run = true;
	std::filesystem::path _path;
	std::unordered_map<std::string, std::filesystem::file_time_type> _paths;
};
