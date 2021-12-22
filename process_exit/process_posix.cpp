#include "process.hpp"
#include <spawn.h>
#include <sys/wait.h>
#include <system_error>

process::process(std::filesystem::path&& executable, argument_vector&& arguments) :
	_executable(executable),
	_arguments({ executable })
{
	std::move(arguments.begin(), arguments.end(), std::back_inserter(_arguments));
}

process::~process()
{
}

void process::start()
{
	char** arguments = new char*[_arguments.size()];

	for (size_t i = 0; i < _arguments.size(); ++i)
	{
		arguments[i] = _arguments[i].data();
	}

	if (posix_spawn(&_pid, _executable.c_str(), nullptr, nullptr, arguments, nullptr) != 0)
	{
		delete[] arguments;
		throw std::runtime_error("Could not start process");
	}

	delete[] arguments;
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