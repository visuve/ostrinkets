#include "file_watcher.hpp"

#include <Windows.h>

#include <iostream>

using millisecond = std::chrono::duration<DWORD, std::ratio<1, 1000>>;

class notification_handle
{
public:
	notification_handle(const std::filesystem::path& path) :
		_handle(FindFirstChangeNotificationW(
			path.c_str(),
			true,
			FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME))
	{
		if (!is_valid())
		{
			throw std::system_error(
				GetLastError(),
				std::system_category(),
				"FindFirstChangeNotification failed");
		}
	}

	bool is_valid() const
	{
		return _handle != 0 && _handle != INVALID_HANDLE_VALUE;
	}

	DWORD wait(millisecond time)
	{
		return WaitForSingleObject(_handle, time.count());
	}

	void refresh()
	{
		if (!FindNextChangeNotification(_handle))
		{
			throw std::system_error(
				GetLastError(),
				std::system_category(),
				"FindNextChangeNotification failed");
		}
	}

	~notification_handle()
	{
		if (is_valid() && !FindCloseChangeNotification(_handle))
		{
			std::cerr << "FindCloseChangeNotification failed: 0x"
				<< std::hex << GetLastError() << std::endl;
		}
	}

private:
	HANDLE _handle = nullptr;
};

file_watcher::file_watcher(const std::filesystem::path& path) :
	_path(path)
{
}

void file_watcher::start(const std::function<void()>& callback)
{
	notification_handle handle(_path);

	while (_run)
	{
		DWORD result = handle.wait(millisecond(1000));

		switch (result)
		{
			case WAIT_OBJECT_0:
				handle.refresh();
				callback();
				break;
			case WAIT_TIMEOUT:
				break;
			case WAIT_FAILED:
				std::cerr << "WaitForSingleObject failed: 0x"
					<< std::hex << GetLastError() << std::endl;
				return;
			default:
				std::cerr << "WaitForSingleObject unexpected: 0x"
					<< std::hex << result << " / 0x" << std::hex << GetLastError() << std::endl;
				return;
		}
	}
}

void file_watcher::stop()
{
	_run = false;
}
