#include "mem_search.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <fcntl.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <unistd.h>

namespace mem_search
{
	struct memory_region
	{
		uint64_t address_start;
		uint64_t address_end;
		std::string permissions; // TODO: use std::filesystem::perms permissions
		uint64_t offset;
		std::string dev;
		uint32_t inode;
		std::filesystem::path path;

		friend std::istream& operator >> (std::istream& input, memory_region& region)
		{
			input >> std::hex >> region.address_start;
			input.get(); // Skip '-' marker between regions
			input >> std::hex >> region.address_end;
			input >> region.permissions;
			input >> std::hex >> region.offset;
			input >> region.dev;
			input >> std::dec >> region.inode;
			input >> region.path;
			return input;
		}

		bool is_readable() const
		{
			return permissions.starts_with('r');
		}

		int64_t size() const
		{
			return address_end - address_start;
		}
	};

	std::vector<memory_region> parse_process_memory_map(pid_t pid)
	{
		std::filesystem::path process_memory_map_path = "/proc/" + std::to_string(pid) + "/maps";
		std::ifstream process_memory_map(process_memory_map_path);

		std::vector<memory_region> result;
		memory_region region = {};

		while (process_memory_map >> region)
		{
			result.emplace_back(region);
		}

		return result;
	}

	pid_t find_pid_by_process_name(std::string_view needle)
	{
		for (const auto& iter : std::filesystem::directory_iterator("/proc"))
		{
			const std::filesystem::path status_file_path = iter.path() / "cmdline";

			std::ifstream status_file(status_file_path);
			std::string haystack;

			if (std::getline(status_file, haystack) && haystack.starts_with(needle))
			{
				return std::stoi(iter.path().stem());
			}
		}

		throw std::runtime_error("Process not found!");
	}

	class process_memory_descriptor
	{
	public:
		process_memory_descriptor(pid_t pid) :
			process_memory_descriptor("/proc/" + std::to_string(pid) + "/mem", pid)
		{
		}

		~process_memory_descriptor()
		{
			if (_process_handle != -1)
			{
				ptrace(PT_DETACH, _pid, nullptr, 0);
				close(_process_handle);
			}
		}

		operator int() const
		{
			return _process_handle;
		}

	private:
		process_memory_descriptor(const std::filesystem::path& file, pid_t pid) :
			_process_handle(open(file.c_str(), O_RDONLY)),
			_pid(pid)
		{
			if (_process_handle != -1)
			{
				ptrace(PT_ATTACHEXC, pid, nullptr, 0);
				waitpid(pid, nullptr, 0);
			}
		}

		int _process_handle = -1;
		pid_t _pid = -1;
	};

	uint64_t find_value_in_process(
		std::string_view process_name,
		std::string_view value_to_search)
	{
		const pid_t pid = find_pid_by_process_name(process_name);
		const auto process_memory_map = parse_process_memory_map(pid);

		std::string buffer;
		process_memory_descriptor pmd(pid);

		for (const auto& region : process_memory_map)
		{
			if (!region.is_readable())
			{
				continue;
			}

			buffer.resize(region.size());
			ssize_t bytes_read = pread(pmd, buffer.data(), region.size(), region.address_start);

			if (bytes_read <=0)
			{
				continue;
			}

			buffer.resize(bytes_read);

			uint64_t position = buffer.find(value_to_search);

			if (position != std::string::npos)
			{
				return region.address_start + position;
			}
		}

		return 0;
	}
}
