#include "serial_reader.hpp"

#include <Windows.h>

#include <span>

namespace ostrinkets::serial
{
	class win32_serial
	{
	public:
		win32_serial(const options& opt) :
			_handle(CreateFileA(opt.path, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr))
		{
			DCB dcb = {};

			if (!GetCommState(_handle, &dcb))
			{
				throw std::system_error(GetLastError(), std::system_category(), "GetCommState");
			}

			dcb.BaudRate = opt.baud_rate;
			dcb.StopBits = 1;
			dcb.Parity = 0;
			dcb.ByteSize = 8;

			dcb.fOutX = 0;
			dcb.fInX = 0;
			dcb.fRtsControl = RTS_CONTROL_ENABLE;
			dcb.fDtrControl = DTR_CONTROL_ENABLE;

			if (!SetCommState(_handle, &dcb))
			{
				throw std::system_error(GetLastError(), std::system_category(), "SetCommState");
			}
		}

		~win32_serial()
		{
			if (_handle)
			{
				CloseHandle(_handle);
			}
		}

		operator bool() const
		{
			return _handle != nullptr && _handle != INVALID_HANDLE_VALUE;
		}

		std::span<char> read(std::span<char> buffer)
		{
			DWORD bytes_read = 0;

			if (!ReadFile(_handle, buffer.data(), static_cast<DWORD>(buffer.size()), &bytes_read, nullptr))
			{
				throw std::system_error(GetLastError(), std::system_category(), "ReadFile");
			}

			return buffer.subspan(0, bytes_read);
		}

	private:
		HANDLE _handle;
		
	};


	void read(const options& opt, std::ostream& os)
	{
		char buffer[0x1000] = {};

		win32_serial serial(opt);

		while (serial)
		{
			auto result = serial.read(buffer);

			if (result.size())
			{
				os.write(result.data(), result.size());
			}
		}
	}
}