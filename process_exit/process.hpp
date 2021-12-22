#pragma once

#include <filesystem>
#include <memory>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
using argument_vector = std::vector<std::wstring>;
#else
#include <sys/types.h>
using argument_vector = std::vector<std::string>;
#endif

class process
{
public:
	process(std::filesystem::path&& executable, argument_vector&& arguments);
	virtual ~process();

	void start();
	void wait();
	int exit_code() const;

private:
	std::filesystem::path _executable;
	argument_vector _arguments;

#ifdef _WIN32
	std::unique_ptr<PROCESS_INFORMATION> _process_info;
#else
	pid_t _pid = -1;
	int _status = -1;
#endif
};