#include "file_watcher.hpp"

#include <fcntl.h>
#include <sys/event.h>
// #include <stdio.h>
// #include <stdlib.h>
#include <unistd.h>

#include <iostream>

class file_desciptor
{
public:
	file_desciptor(const std::filesystem::path& path) :
		_descriptor(open(path.c_str(), O_RDONLY))
	{
		if (!is_valid())
		{
			std::cerr << "open failed: 0x"
				<< std::hex << errno << std::endl;
		}
	}

	bool is_valid() const
	{
		return _descriptor != -1;
	}

	int get() const
	{
		return _descriptor;
	}

	~file_desciptor()
	{
		if (is_valid())
		{
			close(_descriptor);
		}
	}

private:
	int _descriptor = -1;
};

file_watcher::file_watcher(const std::filesystem::path& path) :
	_path(path)
{
}

void file_watcher::start(const std::function<void()>& callback)
{
	file_desciptor fd(_path);

	if (!fd.is_valid())
	{
		return;
	}

	int kq = kqueue();

	if (kq == -1)
	{
		std::cerr << "kqueue failed: 0x"
			<< std::hex << errno << std::endl;
		return;
	}

	struct kevent event = {};

	EV_SET(&event, fd.get(), EVFILT_VNODE, EV_ADD | EV_CLEAR, NOTE_WRITE, 0, nullptr);

	if (kevent(kq, &event, 1, nullptr, 0, nullptr) == -1)
	{
		std::cerr << "kevent failed: 0x"
			<< std::hex << errno << std::endl;
		return;
	}

	if (event.flags & EV_ERROR)
	{
		std::cerr << "kevent failed: 0x"
			<< std::hex << event.data << std::endl;
	}

	struct kevent triggered = {};

	while (_run)
	{
		int result = kevent(kq, nullptr, 0, &triggered, 1, nullptr);

		if (result == -1)
		{
			std::cerr << "kevent poll failed: 0x"
				<< std::hex << errno << std::endl;
			return;
		}

		if (result > 0)
		{
			callback();
		}
	}
}

void file_watcher::stop()
{
	_run = false;
}
