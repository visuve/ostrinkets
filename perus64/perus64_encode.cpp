#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>

constexpr char base64(uint32_t index)
{
	switch (index)
	{
		case 0x00: return 'A';
		case 0x01: return 'B';
		case 0x02: return 'C';
		case 0x03: return 'D';
		case 0x04: return 'E';
		case 0x05: return 'F';
		case 0x06: return 'G';
		case 0x07: return 'H';
		case 0x08: return 'I';
		case 0x09: return 'J';
		case 0x0A: return 'K';
		case 0x0B: return 'L';
		case 0x0C: return 'M';
		case 0x0D: return 'N';
		case 0x0E: return 'O';
		case 0x0F: return 'P';
		case 0x10: return 'Q';
		case 0x11: return 'R';
		case 0x12: return 'S';
		case 0x13: return 'T';
		case 0x14: return 'U';
		case 0x15: return 'V';
		case 0x16: return 'W';
		case 0x17: return 'X';
		case 0x18: return 'Y';
		case 0x19: return 'Z';

		case 0x1A: return 'a';
		case 0x1B: return 'b';
		case 0x1C: return 'c';
		case 0x1D: return 'd';
		case 0x1E: return 'e';
		case 0x1F: return 'f';
		case 0x20: return 'g';
		case 0x21: return 'h';
		case 0x22: return 'i';
		case 0x23: return 'j';
		case 0x24: return 'k';
		case 0x25: return 'l';
		case 0x26: return 'm';
		case 0x27: return 'n';
		case 0x28: return 'o';
		case 0x29: return 'p';
		case 0x2A: return 'q';
		case 0x2B: return 'r';
		case 0x2C: return 's';
		case 0x2D: return 't';
		case 0x2E: return 'u';
		case 0x2F: return 'v';
		case 0x30: return 'w';
		case 0x31: return 'x';
		case 0x32: return 'y';
		case 0x33: return 'z';

		case 0x34: return '0';
		case 0x35: return '1';
		case 0x36: return '2';
		case 0x37: return '3';
		case 0x38: return '4';
		case 0x39: return '5';
		case 0x3A: return '6';
		case 0x3B: return '7';
		case 0x3C: return '8';
		case 0x3D: return '9';

		case 0x3E: return '+';
		case 0x3F: return '/';
	}

	throw std::out_of_range("Index needs to be between 0 and 63");
}

void perus64_encode(std::istream& in, std::ostream& out)
{
	while (!in.eof())
	{
		std::array<char, 3> triplet;
		in.read(triplet.data(), triplet.size());
		uint32_t baseIndex = 0;

		switch (in.gcount())
		{
			case 1:
				baseIndex = triplet[0] << 16;
				out.put(base64(baseIndex >> 18));
				out.put(base64(baseIndex >> 12 & 63));
				out.put('=');
				out.put('=');
				continue;
			case 2:
				baseIndex = triplet[0] << 16 | triplet[1] << 8;
				out.put(base64(baseIndex >> 18));
				out.put(base64(baseIndex >> 12 & 63));
				out.put(base64(baseIndex >> 6 & 63));
				out.put('=');
				continue;
			case 3:
				baseIndex = triplet[0] << 16 | triplet[1] << 8 | triplet[2];
				out.put(base64(baseIndex >> 18));
				out.put(base64(baseIndex >> 12 & 63));
				out.put(base64(baseIndex >> 6 & 63));
				out.put(base64(baseIndex & 63));
				continue;
		}
	}
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cout << "Perus64 encoder!\nUsage:" << std::endl;
		std::cout << '\t' << argv[0] << " <path/to/encode>" << std::endl;
		return -1;
	}

	const std::filesystem::path filePath(argv[1]);

	if (!std::filesystem::exists(filePath))
	{
		std::cerr << '"' << filePath << "\" does not exist!" << std::endl;
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
		perus64_encode(file, std::cout);
	}
	catch (const std::fstream::failure& e)
	{
		std::cerr << "An I/O exception occurred: " << e.what()<< std::endl;
		return -1;
	}
	catch (const std::exception& e)
	{
		std::cerr << "An exception occurred: " << e.what() << std::endl;
		return -1;
	}

	return 0;
}
