#include "window_manager.hpp"

#include <stdexcept>

#include <X11/Xutil.h>
#include <X11/Xatom.h>

window_manager::window_manager() :
	_display(XOpenDisplay(nullptr))
{
	if (!_display)
	{
		throw std::runtime_error("XQueryTree failed!");
	}

	_root = XDefaultRootWindow(_display);

	_clients = client_list("_NET_CLIENT_LIST");

	if (_clients.empty() || !_clients.data())
	{
		_clients = client_list("_WIN_CLIENT_LIST");
	}

	if (_clients.empty() || !_clients.data())
	{
		throw std::runtime_error("Failed to list clients!");
	}
}

window_manager::~window_manager()
{
	if (_display)
	{
		XCloseDisplay(_display);
	}
}

bool window_manager::close_by_pid(int pid)
{
	for (Window window : _clients)
	{
		if (window_pid(window) == pid)
		{
			close_window(window);
			return true;
		}
	}

	return false;
}

std::vector<Window> window_manager::client_list(std::string_view property_name)
{
	Atom property = XInternAtom(_display, property_name.data(), false);
	Atom type;
	int32_t format;
	unsigned long count;
	unsigned long remaining;
	uint8_t* data;

	Status status = XGetWindowProperty(
		_display,
		_root,
		property,
		0,
		1024,
		false,
		XA_WINDOW,
		&type,
		&format,
		&count,
		&remaining,
		&data);

	std::vector<Window> result;

	if (status == Success)
	{
		auto cast = reinterpret_cast<Window*>(data);
		result = { cast, cast + count };
	}

	if (data)
	{
		XFree(data);
	}

	return result;
}

int window_manager::window_pid(Window window)
{
	Atom property = XInternAtom(_display, "_NET_WM_PID", true);
	Atom type;
	int32_t format;
	unsigned long count;
	unsigned long remaining;
	uint8_t* data;

	Status status = XGetWindowProperty(
		_display,
		window,
		property,
		0,
		1024,
		false,
		XA_CARDINAL,
		&type,
		&format,
		&count,
		&remaining,
		&data);

	int pid = -1;

	if (status == Success && count > 0)
	{
		pid = *reinterpret_cast<int*>(data);
	}

	if (data)
	{
		XFree(data);
	}

	return pid;
}

void window_manager::close_window(Window id)
{
	constexpr long mask = SubstructureRedirectMask | SubstructureNotifyMask;

	XEvent e = {};
	e.xclient.format = 32;
	e.xclient.message_type = XInternAtom(_display, "_NET_CLOSE_WINDOW", false);
	e.xclient.send_event = true;
	e.xclient.type = ClientMessage;
	e.xclient.window = id;

	if (!XSendEvent(_display, _root, false, mask, &e))
	{
		throw std::runtime_error("XSendEvent failed!");
	}

	XSync(_display, false);
}