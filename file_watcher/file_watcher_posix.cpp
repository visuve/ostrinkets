#include "file_watcher.hpp"

#include <poll.h>
#include <sys/inotify.h>
#include <unistd.h>

#include <chrono>
#include <iostream>

using millisecond = std::chrono::duration<int, std::ratio<1, 1000>>;

class notification_descriptor
{
public:
	notification_descriptor(const std::filesystem::path& path)//  :
	{
		_inotify = inotify_init1(IN_NONBLOCK);

		if (_inotify == -1)
		{
			std::cerr << "inotify_init1 failed: 0x"
				<< std::hex << errno << std::endl;
			return;
		}

		_watch = inotify_add_watch(_inotify, path.c_str(), IN_CREATE | IN_DELETE);

		if (_watch == -1)
		{
			std::cerr << "inotify_add_watch failed: 0x"
				<< std::hex << errno << std::endl;
		}
	}

	bool is_valid() const
	{
		return _inotify != -1 && _watch != -1;
	}

	short wait(millisecond timeout)
	{
		pollfd poll_inotify = {};
		poll_inotify.fd = _inotify;
		poll_inotify.events = POLLIN;

		if (poll(&poll_inotify, 1, timeout.count()) == -1)
		{
			std::cerr << "poll failed: 0x"
				<< std::hex << errno << std::endl;
			return -1;
		}

		return poll_inotify.revents;
	}

	void refresh()
	{
		char buffer[0xFF] = {};
		read(_inotify, buffer, 0xFF);
		// TODO: make something usefull with the buffer
	}

	~notification_descriptor()
	{
		if (_watch != -1 && inotify_rm_watch(_inotify, _watch) == -1)
		{
			std::cerr << "inotify_rm_watch failed: 0x"
				<< std::hex << errno << std::endl;
		}

		if (_inotify != -1 && close(_inotify) == -1)
		{
			std::cerr << "close failed: 0x"
				<< std::hex << errno << std::endl;
		}
	}

private:
	int _inotify = -1;
	int _watch = -1;
};

file_watcher::file_watcher(const std::filesystem::path& path) :
	_path(path)
{
}

void file_watcher::start(const std::function<void()>& callback)
{
	notification_descriptor notification(_path);

	if (!notification.is_valid())
	{
		return;
	}

	while (_run)
	{
		short result = notification.wait(millisecond(1000));

		if (result < 0)
		{
			return;
		}

		if (result == 0)
		{
			continue;
		}

		if (result & POLLIN)
		{
			notification.refresh();
			callback();
		}
	}
}

void file_watcher::stop()
{
	_run = false;
}
