#include "process.hpp"
#include <sstream>

namespace
{
	std::wstring join(const argument_vector& v)
	{
		std::wstringstream stream;
		std::ostream_iterator<std::wstring, wchar_t> iter(stream, L" ");
		std::copy(v.begin(), v.end(), iter);
		return stream.str();
	}
}

process::process(std::filesystem::path&& executable, argument_vector&& arguments) :
	_executable(executable),
	_arguments({ executable })
{
	std::move(arguments.begin(), arguments.end(), std::back_inserter(_arguments));
}

process::~process()
{
	if (_process_info)
	{
		CloseHandle(_process_info->hThread);
		CloseHandle(_process_info->hProcess);
	}
}

void process::start()
{
	STARTUPINFOW startup_info = {};
	startup_info.cb = sizeof(startup_info);

	_process_info = std::make_unique<PROCESS_INFORMATION>();

	std::wstring arguments = join(_arguments);

	if (!CreateProcessW(
		_executable.c_str(),
		arguments.data(),
		nullptr,
		nullptr,
		FALSE,
		0,
		nullptr,
		nullptr,
		&startup_info,
		_process_info.get())
		)
	{
		_process_info = nullptr;
		throw std::runtime_error("Could not start process");
	}
}

void process::wait()
{
	if (!_process_info)
	{
		throw std::runtime_error("Process not started?");
	}

	if (WaitForSingleObject(_process_info->hProcess, INFINITE) != WAIT_OBJECT_0)
	{
		throw std::runtime_error("Failed to wait process");
	}
}

int process::exit_code() const
{
	if (!_process_info)
	{
		throw std::runtime_error("Process not started?");;
	}

	DWORD result = 0;

	if (!GetExitCodeProcess(_process_info->hProcess, &result))
	{
		throw std::runtime_error("Could not get process exit code");
	}

	return static_cast<int>(result);
}