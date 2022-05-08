#include <filesystem>
#include <fstream>
#include <iostream>

void print_usage(const std::filesystem::path& executable)
{
	std::cout << "Usage: " << executable
		<< " <file> <prefix[optional]> <postfix[optional]>" << std::endl;
}

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		print_usage(argv[0]);
		return EINVAL;
	}

	std::ifstream file(argv[1], std::ios::binary);
	std::string prefix(argc > 2 ? argv[2] : "0x");
	std::string postfix(argc > 3 ? argv[3] : ", ");

	std::cout.setf(std::ios::hex, std::ios::basefield);
	std::cout.setf(std::ios::uppercase);
	std::cout.fill('0');

	uint8_t byte;

	while (file >> byte)
	{
		std::cout << prefix << std::setw(2) << +byte;

		if (file.peek() != EOF)
		{
			std::cout << postfix;
		}
	}

	return 0;
}