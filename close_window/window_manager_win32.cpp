#include "window_manager.hpp"

#include <Windows.h>

#include <stdexcept>

window_manager::window_manager()
{
}

window_manager::~window_manager()
{
}

bool window_manager::close_by_pid(int pid)
{
	struct window_data
	{
		HWND window = nullptr;
		DWORD pid = 0;
	};

	window_data data;
	data.pid = static_cast<DWORD>(pid);

	auto callback = [](HWND window, LPARAM lParam)->BOOL
	{
		auto data = reinterpret_cast<window_data*>(lParam);

		if (!data)
		{
			throw std::runtime_error("Failed to cast window data!");
		}

		DWORD window_pid = 0;

		if (GetWindowThreadProcessId(window, &window_pid) && window_pid == data->pid)
		{
			data->window = window;
			return FALSE; // Stop enumerating
		}

		return TRUE; // Continue
	};

	EnumWindows(callback, reinterpret_cast<LPARAM>(&data));

	if (!data.window)
	{
		throw std::runtime_error("Failed to find window!");
	}

	return PostMessage(data.window, WM_QUIT, 0, 0);
}