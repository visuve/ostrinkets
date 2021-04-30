#pragma once

#include <atomic>
#include <filesystem>
#include <functional>

class file_watcher
{
public:
	explicit file_watcher(const std::filesystem::path& path);
	void start(const std::function<void()>& callback);
	void stop();

private:
	std::atomic<bool> _run = true;
	std::filesystem::path _path;
};
