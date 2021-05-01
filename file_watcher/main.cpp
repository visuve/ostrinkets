#include "file_watcher.hpp"

#include <chrono>
#include <csignal>
#include <iostream>
#include <memory>

std::unique_ptr<file_watcher> fw;

void signal_handler(int)
{
	if (fw)
	{
		fw->stop();
	}
}

void report_changes()
{
	const auto now = std::chrono::system_clock::now();
	const auto tt = std::chrono::system_clock::to_time_t(now);
	std::cout << tt << " Changed..." << std::endl;
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cout << "Usage: " << argv[0] << " <target>" << std::endl;
		return EINVAL;
	}

	if (std::signal(SIGINT, signal_handler) == SIG_ERR)
	{
		std::cerr << "Cannot attach SIGINT handler!" << std::endl;
		return EXIT_FAILURE;
	}

	try
	{
		fw = std::make_unique<file_watcher>(argv[1]);
		fw->start(report_changes);
	}
	catch (const std::system_error& sys_error)
	{
		std::cerr << "An exception occurred:" << std::endl;
		std::cerr << sys_error.what() << std::endl;
		std::cerr << "Error code: " << sys_error.code().value() << std::endl;
		return sys_error.code().value();
	}
	catch (const std::exception& e)
	{
		std::cerr << "An exception occurred:" << std::endl;
		std::cerr << e.what();
		return EXIT_FAILURE;
	}

	return 0;
}
