#pragma once

#ifdef _WIN32

#else
#include <X11/Xlib.h>
#include <string_view>
#include <vector>
#endif

class window_manager
{
public:
	window_manager();
	~window_manager();

	bool close_by_pid(int pid);

private:
#ifdef _WIN32
#else
	std::vector<Window> client_list(std::string_view property_name);
	int window_pid(Window window);
	void close_window(Window id);

	Display* _display = nullptr;
	Window _root;
	std::vector<Window> _clients;
#endif
};