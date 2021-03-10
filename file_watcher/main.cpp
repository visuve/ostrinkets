#include <atomic>
#include <chrono>
#include <csignal>
#include <filesystem>
#include <functional>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>

std::atomic<int> g_signal;

void signal_handler(int signal)
{
	g_signal = signal;
}

enum class change_type : uint8_t
{
	created = 'c',
	modified = 'm',
	removed = 'r'
};

class file_watcher
{
public:
	file_watcher(const std::filesystem::path& path, std::chrono::seconds delay) :
		m_path(path),
		m_delay(delay)
	{
		for (const auto& file : std::filesystem::recursive_directory_iterator(path))
		{
			m_paths[file.path().string()] = std::filesystem::last_write_time(file);
		}
	}

	void start(const std::function<void(std::filesystem::path, change_type)>& callback)
	{
		while (!g_signal)
		{
			std::this_thread::sleep_for(m_delay);

			for (auto it = m_paths.begin(); it != m_paths.cend();)
			{
				if (std::filesystem::exists(it->first))
				{
					++it;
					continue;
				}

				callback(it->first, change_type::removed);
				it = m_paths.erase(it);
			}

			for (const auto& file : std::filesystem::recursive_directory_iterator(m_path))
			{
				const std::string key = file.path().string();
				const std::filesystem::file_time_type time = std::filesystem::last_write_time(file);

				if (!m_paths.contains(key))
				{
					m_paths[key] = time;
					callback(file.path(), change_type::created);
					continue;
				}

				if (m_paths[key] != time)
				{
					m_paths[key] = time;
					callback(file.path(), change_type::modified);
					continue;
				}
			}
		}
	}

private:
	std::filesystem::path m_path;
	std::chrono::seconds m_delay;
	std::unordered_map<std::string, std::filesystem::file_time_type> m_paths;
};

void report_changes(const std::filesystem::path& path, change_type change)
{
	switch (change)
	{
		case change_type::created:
			std::cout << "File: " << path << " created." << std::endl;
			break;
		case change_type::modified:
			std::cout << "File: " << path << " modified." << std::endl;
			break;
		case change_type::removed:
			std::cout << "File: " << path << " removed." << std::endl;
			break;
		default:
			std::cout << "Failed to detect change type for: " << path << std::endl;
			break;
	}
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cout << "Usage: " << argv[0] << " <target>" << std::endl;
		return EINVAL;
	}

	if (std::signal(SIGINT, signal_handler) == SIG_ERR)
	{
		std::cerr << "Cannot attach SIGINT handler!" << std::endl;
		return EXIT_FAILURE;
	}

	file_watcher fw(argv[1], std::chrono::seconds(10));
	fw.start(report_changes);

	return 0;
}