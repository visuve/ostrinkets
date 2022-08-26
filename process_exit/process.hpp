#pragma once

#include <filesystem>
#include <memory>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
using argument_vector = std::vector<std::wstring>;
#else
#include <sys/types.h>

#endif

class process
{
	using value_type = std::filesystem::path::value_type;
	using string_type = std::filesystem::path::string_type;
	using argument_vector = std::vector<string_type>;

public:
	process(
		std::filesystem::path&& executable,
		argument_vector&& arguments,
		value_type** environment);

	virtual ~process();

	int start();
	void wait();
	int exit_code() const;

private:
	std::filesystem::path _executable;
	argument_vector _arguments;
	value_type** _environment;

#ifdef _WIN32
	std::unique_ptr<PROCESS_INFORMATION> _process_info;
#else
	pid_t _pid = -1;
	int _status = -1;
#endif
};