#include "mem_search.hpp"

#include <Windows.h>
#include <TlHelp32.h>

namespace mem_search
{
	DWORD find_pid_by_process_name(std::string_view process_name)
	{
		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		PROCESSENTRY32 process = {};
		process.dwSize = sizeof(PROCESSENTRY32);
		DWORD pid = 0;

		if (Process32First(snapshot, &process))
		{
			do
			{
				if (process.szExeFile == process_name)
				{
					pid = process.th32ProcessID;
					break;
				}
			}
			while (Process32Next(snapshot, &process));
		}

		CloseHandle(snapshot);
		return pid;
	}

	constexpr bool is_readable(const MEMORY_BASIC_INFORMATION& memory_info)
	{
		return memory_info.State == MEM_COMMIT &&
			(memory_info.Type == MEM_MAPPED || memory_info.Type == MEM_PRIVATE);
	}

	uint64_t find_value_in_process(std::string_view process_name, std::string_view value_to_search)
	{	
		DWORD pid = find_pid_by_process_name(process_name);

		if (!pid)
		{
			return 0;
		}

		HANDLE process_handle = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, false, pid);

		if (!process_handle)
		{
			return 0;
		}

		SYSTEM_INFO system_info = {};
		GetSystemInfo(&system_info);

		MEMORY_BASIC_INFORMATION memory_info = {};
		std::string buffer;

		for (void* address = system_info.lpMinimumApplicationAddress;
			address < system_info.lpMaximumApplicationAddress;
			address = reinterpret_cast<void*>(reinterpret_cast<SIZE_T>(address) + memory_info.RegionSize))
		{
			if (VirtualQueryEx(
				process_handle,
				address,
				&memory_info,
				sizeof(MEMORY_BASIC_INFORMATION)) != sizeof(MEMORY_BASIC_INFORMATION))
			{
				break;
			}

			if (!is_readable(memory_info))
			{
				continue;
			}

			SIZE_T bytes_read = 0;
			buffer.resize(memory_info.RegionSize);

			if (!ReadProcessMemory(
				process_handle,
				address,
				buffer.data(),
				memory_info.RegionSize,
				&bytes_read))
			{
				continue;
			}

			buffer.resize(bytes_read);
			uint64_t position = buffer.find(value_to_search);

			if (position != std::string::npos)
			{
				return reinterpret_cast<uint64_t>(address) + position;
			}
		}

		CloseHandle(process_handle);
		return 0;
	}
}