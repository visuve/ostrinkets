#include <atomic>
#include <algorithm>
#include <csignal>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <set>
#include <vector>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#define _text(x) L ## x
#define text(x) _text(x)
#define put_string(x) _putws(text(x))
#define put_formatted(x, ...) wprintf_s(text(x), __VA_ARGS__)
#else
#define put_string(x) puts(x)
#define put_formatted(x, ...) printf(x, __VA_ARGS__)
#endif

namespace
{
	namespace fs = std::filesystem;

	std::atomic<int> g_signal_status = 0;

	void signal_handler(int signal)
	{
		g_signal_status = signal;
	}

	const std::set<std::filesystem::path> paths_to_ignore =
	{
		".bzr",
		".cvs",
		".git",
		".hg",
		".svn"
	};

	bool is_ignored(const fs::directory_entry& entry)
	{
		const auto equal_stem = [&](const fs::path& ignore)
		{
			return entry.path().stem() == ignore;
		};

		return std::any_of(paths_to_ignore.cbegin(), paths_to_ignore.cend(), equal_stem);
	}

	bool is_skipped(const fs::directory_entry& entry, const std::vector<fs::path>& unwanted)
	{
		const auto equal_filename = [&](const fs::path& to_remove)
		{
			return entry.path().filename() == to_remove;
		};

		return std::none_of(unwanted.cbegin(), unwanted.cend(), equal_filename);
	}

	uintmax_t remove_entry(const fs::directory_entry& entry)
	{
		try
		{
			switch (entry.status().type())
			{
				case fs::file_type::regular:
				{
					return fs::remove(entry) ? 1 : 0;
				}
				case fs::file_type::directory:
				{
					return fs::remove_all(entry);
				}
				default:
				{
					put_formatted(" -> skipped, invalid type: %d.\n",
						static_cast<int>(entry.status().type()));
				}
			}
		}
		catch (const fs::filesystem_error& e)
		{
			printf(" -> skipped, exception: %s\n", e.what());
		}

		return 0;
	}

	void clean(const fs::path& path, const std::vector<fs::path>& to_remove)
	{
		auto iter = fs::recursive_directory_iterator(path);
		uintmax_t removed_total = 0;

		for (auto& entry : iter)
		{
			if (g_signal_status)
			{
				put_formatted(" -> skipped, invalid type: %d.\n", g_signal_status.load());
				break;
			}

			// Optimization tweak
			const auto native_path = entry.path().native();
			fwrite(native_path.c_str(), sizeof(native_path.front()), native_path.size(), stdout);

			if (is_ignored(entry))
			{
				iter.disable_recursion_pending();
				++iter;
				put_string(" -> ignored.");
				continue;
			}

			if (is_skipped(entry, to_remove))
			{
				put_string(" -> skipped.");
				continue;
			}

			if ((entry.status().permissions() & fs::perms::owner_exec) == fs::perms::none)
			{
				put_string(" -> skipped, no perms.");
				continue;
			}

			const uintmax_t removed = remove_entry(entry);

			if (!removed)
			{
				put_string(" -> remove failed.");
				continue;
			}

			removed_total += removed;
			iter.disable_recursion_pending();
			put_string(" -> removed.");
		}

		put_formatted("Done. Removed: %zu items.\n", removed_total);
	}
}

#ifdef _WIN32
int wmain(int argc, wchar_t* argv[])
{
	_setmode(_fileno(stdout), _O_U16TEXT);
	_setmode(_fileno(stderr), _O_U16TEXT);
#else
int main(int argc, char* argv[])
{
#endif
	if (std::signal(SIGINT, signal_handler) == SIG_ERR)
	{
		put_string("Cannot attach SIGINT handler!");
		return EXIT_FAILURE;
	}

	if (argc < 3)
	{
		put_formatted(
			"Usage\n%s <starting-location> <file-or-folder-names-to-recursively-delete>\n",
			argv[0]);

		return EXIT_FAILURE;
	}

	try
	{
		const fs::path start_dir = fs::path(argv[1]);

		if (!fs::exists(start_dir))
		{
			put_formatted("'%s' does not exist!\n", start_dir.c_str());
			return EXIT_FAILURE;
		}

		clean(start_dir, { argv + 1, argv + argc });
	}
	catch (const std::exception& e)
	{
		printf("An exception occurred: %s\n", e.what());
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
