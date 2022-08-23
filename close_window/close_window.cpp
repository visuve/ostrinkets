#include <iostream>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

bool find_and_close(Display* display, Window rootwin, int pid)
{
	Window parent;
	Window* children = nullptr;
	uint32_t no_of_children = 0;

	Status status = XQueryTree(display, rootwin, &rootwin, &parent, &children, &no_of_children);

	if (!status || !no_of_children)
	{
		puts("XQueryTree failed!");
		return false;
	}

	for (uint32_t i = 0; i < no_of_children; i++)
	{
		Window child = children[i];

		XTextProperty text_data;
		Atom atom = XInternAtom(display, "_NET_WM_PID", true);
		status = XGetTextProperty(display, child, &text_data, atom);

		if (!status)
		{
			puts("XGetTextProperty failed!");
			continue;
		}

		if (!text_data.nitems)
		{
			continue;
		}

		int window_pid = text_data.value[1] * 256;
		window_pid += text_data.value[0];

		if (window_pid != pid)
		{
			continue;
		}

		XEvent e;
		e.xclient.type = ClientMessage;
		e.xclient.window = child;
		e.xclient.message_type = XInternAtom(display, "WM_PROTOCOLS", true);
		e.xclient.format = 32;
		e.xclient.data.l[0] = XInternAtom(display, "WM_DELETE_WINDOW", false);
		e.xclient.data.l[1] = CurrentTime;

		status = XSendEvent(display, child, false, NoEventMask, &e);

		if (!status)
		{
			puts("XSendEvent failed!");
			break;
		}

		return true;
	}

	return false;
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cerr << "Usage: " << argv[0] << "<pid>" << std::endl;
		return EINVAL;
	}

	int pid = std::stoi(argv[1]);

	Display* display = XOpenDisplay(nullptr);

	if (!display)
	{
		puts("No display!");
		return -1;
	}

	Window root = XDefaultRootWindow(display);

	if (!root)
	{
		puts("No root window!");
		return -1;
	}

	if (find_and_close(display, root, pid))
	{
		puts("Closed!");
	}
	else
	{
		puts("Did not close :(");
	}

	return 0;
}