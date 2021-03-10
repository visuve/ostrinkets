#include "resource.hpp"

#include <atomic>
#include <bitset>
#include <cassert>
#include <csignal>
#include <cstddef>
#include <chrono>
#include <iostream>
#include <random>
#include <thread>

#ifdef  _WIN32
#define NOMINMAX
#include <Windows.h>
#define last_error GetLastError()
#else
#include <errno.h>
#define last_error errno
#endif

namespace
{
	std::atomic<int> g_signal;

	void signal_handler(int signal)
	{
		g_signal = signal;
	}

	template<typename T, typename N>
	T random_numeric_value(
		N min = std::numeric_limits<N>::min(),
		N max = std::numeric_limits<N>::max())
	{
		thread_local std::random_device device;
		thread_local std::mt19937_64 engine(device());
		thread_local std::uniform_int_distribution<N> distribution(min, max);

		return T(distribution(engine));
	}

	template <typename T>
	T since_epoch()
	{
		auto now = std::chrono::system_clock::now();
		return std::chrono::duration_cast<T>(now.time_since_epoch());
	}
}

bool resource::flip_random_bit()
{
	const uint64_t offset = random_numeric_value<uint64_t>(uint64_t(0), m_size - 1);
	std::optional<std::byte> random_source_byte = read_byte_at(offset);

	if (!random_source_byte.has_value())
	{
		std::cout << since_epoch<std::chrono::seconds>().count()
			<< " Failed to read @ " << offset << std::endl;
		return false;
	}

	const auto random_source_byte_value = std::to_integer<uint32_t>(random_source_byte.value());

	std::bitset<8> random_bits(random_source_byte_value);
	const size_t random_bit_index = random_numeric_value<size_t>(0, 7);
	random_bits.flip(random_bit_index);

	const uint32_t shuffled_value = random_bits.to_ulong();
	std::byte shuffled_byte = static_cast<std::byte>(shuffled_value);

	if (!write_byte_at(offset, shuffled_byte))
	{
		std::cout << since_epoch<std::chrono::seconds>().count()
			<< " Failed to write @ " << offset << std::endl;

		return false;
	}

	if (!flush())
	{
		return false;
	}

	std::cout << since_epoch<std::chrono::seconds>().count();
	printf(" Flipped 0x%.2X -> 0x%.2X @ ",
		random_source_byte_value,
		shuffled_value);
	std::cout << offset << std::endl;

	return true;
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

	resource target_resource(argv[1], false); // TODO: remove hardcoded value

	if (!target_resource.is_valid())
	{
		std::cout << "Failed to open: " << argv[1] << std::endl;
		return last_error;
	}

	if (target_resource.is_empty())
	{
		std::cout << argv[1] << " appears empty!" << std::endl;
		return last_error;
	}

	while (!g_signal)
	{
		if (!target_resource.flip_random_bit())
		{
			return last_error;
		}

		const auto sleep_time = random_numeric_value<std::chrono::seconds>(1, 1000);

		std::cout << since_epoch<std::chrono::seconds>().count()
			<< " Sleeping for " << sleep_time.count() << "s..." << std::endl;

		std::this_thread::sleep_for(sleep_time);
	}

	return 0;
}
