#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>

constexpr uint32_t index(char x)
{
	switch (x)
	{
		case 'A': return 0x00;
		case 'B': return 0x01;
		case 'C': return 0x02;
		case 'D': return 0x03;
		case 'E': return 0x04;
		case 'F': return 0x05;
		case 'G': return 0x06;
		case 'H': return 0x07;
		case 'I': return 0x08;
		case 'J': return 0x09;
		case 'K': return 0x0A;
		case 'L': return 0x0B;
		case 'M': return 0x0C;
		case 'N': return 0x0D;
		case 'O': return 0x0E;
		case 'P': return 0x0F;
		case 'Q': return 0x10;
		case 'R': return 0x11;
		case 'S': return 0x12;
		case 'T': return 0x13;
		case 'U': return 0x14;
		case 'V': return 0x15;
		case 'W': return 0x16;
		case 'X': return 0x17;
		case 'Y': return 0x18;
		case 'Z': return 0x19;

		case 'a': return 0x1A;
		case 'b': return 0x1B;
		case 'c': return 0x1C;
		case 'd': return 0x1D;
		case 'e': return 0x1E;
		case 'f': return 0x1F;
		case 'g': return 0x20;
		case 'h': return 0x21;
		case 'i': return 0x22;
		case 'j': return 0x23;
		case 'k': return 0x24;
		case 'l': return 0x25;
		case 'm': return 0x26;
		case 'n': return 0x27;
		case 'o': return 0x28;
		case 'p': return 0x29;
		case 'q': return 0x2A;
		case 'r': return 0x2B;
		case 's': return 0x2C;
		case 't': return 0x2D;
		case 'u': return 0x2E;
		case 'v': return 0x2F;
		case 'w': return 0x30;
		case 'x': return 0x31;
		case 'y': return 0x32;
		case 'z': return 0x33;

		case '0': return 0x34;
		case '1': return 0x35;
		case '2': return 0x36;
		case '3': return 0x37;
		case '4': return 0x38;
		case '5': return 0x39;
		case '6': return 0x3A;
		case '7': return 0x3B;
		case '8': return 0x3C;
		case '9': return 0x3D;

		case '+': return 0x3E;
		case '/': return 0x3F;
	}

	throw std::out_of_range("Only base64 characters allowed");
}

void perus64_decode(std::istream& in, std::ostream& out)
{
	while (!in.eof())
	{
		std::array<char, 4> quartet;
		in.read(quartet.data(), quartet.size());

		std::streamsize bytesRead = in.gcount();

		if (bytesRead > 1)
		{
			out.put(index(quartet[0]) << 2 | index(quartet[1]) >> 4);
		}

		if (bytesRead > 2 && quartet[2] != '=')
		{
			out.put((index(quartet[1]) & 0xF) << 4 | index(quartet[2]) >> 2);
		}

		if (bytesRead > 3 && quartet[3] != '=')
		{
			out.put((index(quartet[2]) & 0x3) << 6 | index(quartet[3]));
		}
	}
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cout << "Perus64 decoder!\nUsage:" << std::endl;
		std::cout << '\t' << argv[0] << " <path/to/decode>" << std::endl;
		return -1;
	}

	const std::filesystem::path filePath(argv[1]);

	if (!std::filesystem::exists(filePath))
	{
		std::cerr << '"' << filePath << "\" does not exist!" << std::endl;
		return -1;
	}

	if (std::filesystem::file_size(filePath) % 4)
	{
		std::cerr << '"' << filePath << "\" does not appear to be base64 encoded!" << std::endl;
		return -1;
	}

	std::ifstream file(filePath, std::ios_base::binary);

	if (!file.is_open())
	{
		std::cerr << "Failed to open \"" << filePath << '"' << std::endl;
		return -1;
	}

	try
	{
		file.exceptions(std::istream::badbit);
		perus64_decode(file, std::cout);
	}
	catch (const std::fstream::failure& e)
	{
		std::cerr << "An I/O exception occurred: " << e.what() << std::endl;
		return -1;
	}
	catch (const std::exception& e)
	{
		std::cerr << "An exception occurred: " << e.what() << std::endl;
		return -1;
	}

	return 0;
}
