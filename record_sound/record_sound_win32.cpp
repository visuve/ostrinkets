#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include "record_sound.hpp"

#include <Windows.h>
#include <Mmsystem.h>

#include <chrono>
#include <filesystem>
#include <iostream>
#include <thread>

namespace record_sound
{
	std::filesystem::path windows_multimedia_path()
	{
		uint32_t size = GetSystemDirectoryW(nullptr, 0);
		std::wstring buffer(size, 0);

		buffer.resize((GetSystemDirectoryW(buffer.data(), size)));

		if (!buffer.size())
		{
			throw std::system_error(
				GetLastError(),
				std::generic_category(),
				"GetSystemDirectory");
		}

		return std::filesystem::path(buffer) / L"winmm.dll";
	}

	template <typename T>
	T load_function_pointer_ex(HMODULE module, std::string_view function_name)
	{
		FARPROC result = GetProcAddress(module, function_name.data());

		if (!result)
		{
			throw std::system_error(
				GetLastError(),
				std::generic_category(),
				"GetProcAddress");
		}

		return reinterpret_cast<T>(result);
	}

#define load_function_pointer(module, function) load_function_pointer_ex<decltype(&function)>(module, #function)

	class windows_multimedia
	{
	public:
		using send_string_t = decltype(&mciSendStringA);

		windows_multimedia() :
			_module(LoadLibraryW(windows_multimedia_path().c_str()))
		{
			_send_string = load_function_pointer(_module, mciSendStringA);
		}

		~windows_multimedia()
		{
			if (_module)
			{
				FreeLibrary(_module);
				_module = nullptr;
			}
		}

		void send_string(std::string_view command) const
		{
			std::string message(MAXERRORLENGTH, 0);

			MCIERROR result = _send_string(
				command.data(),
				message.data(),
				static_cast<uint32_t>(message.size()),
				nullptr);

			if (result != 0)
			{
				throw std::system_error(
					result,
					std::generic_category(),
					message.front() != 0 ? message : "mciSendStringA");
			}
		}

	private:
		HMODULE _module;
		send_string_t _send_string;
	};

	void pause(std::string_view message)
	{
		std::cout << message << std::endl;
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}

	constexpr int32_t bits_per_sample = 16;
	constexpr int32_t channels = 2;
	constexpr int32_t samples_per_second = 44100;
	constexpr int32_t alignment = bits_per_sample * channels / 8;
	constexpr int32_t bytes_per_second = alignment * samples_per_second;
}

int record_sound::record(int argc, char** argv)
{
	try
	{
		if (argc <= 1)
		{
			return ERROR_BAD_ARGUMENTS;
		}

		const std::filesystem::path recording_path =
			std::filesystem::path(argv[0]).parent_path() /
			std::format("{:%Y%m%d_%H%M%OS}.wav", std::chrono::system_clock::now());

		const std::string quality = std::format(
			"bitspersample {} channels {} alignment {} samplespersec {} bytespersec {}",
			bits_per_sample,
			channels,
			alignment,
			samples_per_second,
			bytes_per_second);

		windows_multimedia winmm;
		winmm.send_string("open new type waveaudio alias derp");
		winmm.send_string("set derp " + quality);

		pause("Press enter to start recording...");

		// To avoid recording the enter key press echo :D
		std::this_thread::sleep_for(std::chrono::milliseconds(250));
		winmm.send_string("record derp");

		pause("Recording. Press enter to stop.");

		winmm.send_string("stop derp");
		winmm.send_string("save derp " + recording_path.string());
		winmm.send_string("delete derp");
		winmm.send_string("close derp");

		std::cout << "Saved: " << recording_path << std::endl;

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
