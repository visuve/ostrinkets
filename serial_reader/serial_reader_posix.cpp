#include "serial_reader.hpp"

#include <span>

#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h> 
#include <asm/termbits.h>
#include <unistd.h> 

namespace ostrinkets::serial
{
	class posix_serial
	{
	public:
		posix_serial(const options& opt) :
			_descriptor(open(opt.path, O_RDWR))
		{
			if (_descriptor < 0)
			{
				throw std::system_error(errno, std::system_category(), "open");
			}

			struct termios2 tty;

			if (ioctl(_descriptor, TCGETS2, &tty) != 0)
			{
				throw std::system_error(errno, std::system_category(), "ioctl");
			}

			tty.c_cflag &= ~PARENB;
			tty.c_cflag &= ~CSTOPB;
			tty.c_cflag &= ~CSIZE;
			tty.c_cflag |= CS8;
			tty.c_cflag &= ~CRTSCTS;
			tty.c_cflag |= CREAD | CLOCAL;
			tty.c_iflag &= ~(IXON | IXOFF | IXANY);
			tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
			tty.c_oflag &= ~OPOST;

			tty.c_cc[VTIME] = 0;
			tty.c_cc[VMIN] = 0;

			tty.c_cflag &= ~CBAUD;
			tty.c_cflag |= CBAUDEX;

			tty.c_ispeed = opt.baud_rate;
			tty.c_ospeed = opt.baud_rate;

			if (ioctl(_descriptor, TCSETS2, &tty) != 0)
			{
				throw std::system_error(errno, std::system_category(), "ioctl");
			}
		}

		~posix_serial()
		{
			if (_descriptor > 0)
			{
				close(_descriptor);
			}
		}

		operator bool() const
		{
			return _descriptor >= 0;
		}

		std::span<char> read(std::span<char> buffer)
		{
			int bytes_read = ::read(_descriptor, buffer.data(), buffer.size());

			if (bytes_read == -1)
			{
				throw std::system_error(errno, std::system_category(), "read");
			}

			return buffer.subspan(0, bytes_read);
		}

	private:
		int _descriptor = 0;
	};


	void read(const options& opt, std::ostream& os)
	{
		char buffer[0x1000] = {};

		posix_serial serial(opt);

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

