#include "process.hpp"
#include <algorithm>
#include <ranges>
#include <spawn.h>
#include <sys/wait.h>
#include <system_error>

process::process(
	std::filesystem::path&& executable,
	argument_vector&& arguments,
	value_type** environment) :
	_executable(executable),
	_arguments({ executable }),
	_environment(environment)
{
#ifdef __APPLE__
	std::copy(arguments.cbegin(), arguments.cend(), std::back_inserter(_arguments));
#else
	std::ranges::move(arguments, std::back_inserter(_arguments));
#endif
}

process::~process()
{
}

int process::start()
{
	std::vector<char*> arguments(_arguments.size() + 1, nullptr);

	std::transform(
		_arguments.begin(),
		_arguments.end(),
		arguments.begin(),
		[](std::string& s)->char*
		{
			return s.data();
		});

	if (posix_spawn(
		&_pid,
		_executable.c_str(),
		nullptr,
		nullptr,
		arguments.data(),
		_environment) != 0)
	{
		throw std::runtime_error("Could not start process");
	}

	return _pid;
}

void process::wait()
{
	if (_pid <= 0)
	{
		throw std::runtime_error("Process not started?");
	}

	if (waitpid(_pid, &_status, 0) == -1)
	{
		throw std::runtime_error("Failed to wait process");
	}

	if (!WIFEXITED(_status))
	{
		throw std::runtime_error("Failed to wait process");
	}
}

int process::exit_code() const
{
	return WEXITSTATUS(_status);
}
