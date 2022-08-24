#include <iostream>
#include <stdexcept>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

class xdisplay
{
public:
	xdisplay() :
		_display(XOpenDisplay(nullptr))
	{
		if (!_display)
		{
			throw std::runtime_error("XQueryTree failed!");
		}

		_root = XDefaultRootWindow(_display);
	}

	~xdisplay()
	{
		if (_display)
		{
			XCloseDisplay(_display);
		}
	}

	void close_by_pid(int pid)
	{
		Window parent;
		Window* children = nullptr;
		uint32_t child_count = 0;

		Status status = XQueryTree(_display, _root, &_root, &parent, &children, &child_count);

		if (!status)
		{
			throw std::runtime_error("XQueryTree failed!");
		}

		for (uint32_t i = 0; i < child_count; i++)
		{
			Window child = children[i];

			XTextProperty text;
			Atom atom = XInternAtom(_display, "_NET_WM_PID", true);
			status = XGetTextProperty(_display, child, &text, atom);

			if (!status)
			{
				continue;
			}

			if (!text.nitems)
			{
				continue;
			}

			int window_pid = text.value[1] * 256;
			window_pid += text.value[0];

			if (window_pid != pid)
			{
				continue;
			}

			close_window(child);
			return;
		}

		std::cerr << "No window found with PID " << pid << std::endl;
	}

	void close_window(Window id)
	{
		constexpr long mask = SubstructureRedirectMask | SubstructureNotifyMask;

		XEvent e = {};
		e.xclient.format = 32;
		e.xclient.message_type = XInternAtom(_display, "_NET_CLOSE_WINDOW", false);
		e.xclient.send_event = true;
		e.xclient.type = ClientMessage;
		e.xclient.window = id;

		std::cout << "Closing window: " << std::hex << id << std::endl;

		if (!XSendEvent(_display, _root, false, mask, &e))
		{
			throw std::runtime_error("XSendEvent failed!");
		}

		XSync(_display, false);
	}

private:
	Display* _display = nullptr;
	Window _root;
};

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << "<pid>" << std::endl;
		return EINVAL;
	}

	try
	{
		int pid = std::stoi(argv[1]);

		xdisplay().close_by_pid(pid);
	}
	catch (const std::exception& e)
	{
		std::cerr << "An exception occurred: " << e.what() << std::endl;
	}

	std::cout << "KTHXBYE!" << std::endl;
	return 0;
}