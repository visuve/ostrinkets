#include "file_info.hpp"

#include <Windows.h>

#include <iostream>

namespace fstrinkets
{
	struct auto_handle
	{
		auto_handle(HANDLE handle) :
			m_handle(handle)
		{
		}

		~auto_handle()
		{
			if (m_handle)
			{
				CloseHandle(m_handle);
			}
		}

		operator HANDLE()
		{
			return m_handle;
		}

	private:
		HANDLE m_handle;
	};

	std::wostream& operator << (std::wostream& out, const LARGE_INTEGER& large_integer)
	{
		return out << large_integer.QuadPart;
	}

	std::wostream& operator << (std::wostream& out, const ULARGE_INTEGER& large_integer)
	{
		return out << large_integer.QuadPart;
	}

	std::wostream& operator << (std::wostream& out, const FILETIME& file_time)
	{
		return out << ULARGE_INTEGER{ file_time.dwLowDateTime, file_time.dwHighDateTime };
	}

	std::wostream& operator << (std::wostream& out, const FILE_ID_128& file_id)
	{
		const std::ios::fmtflags before = out.flags();
		out << std::hex << std::setfill(L'0');

		size_t i = 0;

		while (i < 15)
		{
			out << std::setw(2) << file_id.Identifier[i++] << '-';
		}

		out << std::setw(2) << file_id.Identifier[i];
		out.flags(before);
		return out;
	}

	void print_file_info(const std::filesystem::path& path)
	{
		constexpr DWORD desired_access = GENERIC_READ;
		constexpr DWORD share_mode = 0;
		constexpr SECURITY_ATTRIBUTES* security_attributes = nullptr;
		constexpr DWORD creation_disposition = OPEN_EXISTING;
		constexpr DWORD flags_and_attributes = FILE_ATTRIBUTE_NORMAL;
		constexpr HANDLE template_file = nullptr;

		auto_handle handle(
			CreateFileW(
				path.c_str(),
				desired_access,
				share_mode,
				security_attributes,
				creation_disposition,
				flags_and_attributes,
				template_file));

		if (!handle)
		{
			std::wcerr << L"Failed to open '" << path << L"'." << std::endl;
			return;
		}

		std::wcout << path << ':' << std::endl;

		{
			BY_HANDLE_FILE_INFORMATION info = {};

			if (GetFileInformationByHandle(handle, &info))
			{
				std::wcout << L"\tFileInformationByHandle:" << std::endl;
				std::wcout << L"\t\tFileAttributes: " << info.dwFileAttributes << std::endl;
				std::wcout << L"\t\tCreationTime: " << info.ftCreationTime << std::endl;
				std::wcout << L"\t\tLastAccessTime: " << info.ftLastAccessTime << std::endl;
				std::wcout << L"\t\tLastWriteTime: " << info.ftLastWriteTime << std::endl;
				std::wcout << L"\t\tVolumeSerialNumber: " << info.dwVolumeSerialNumber << std::endl;
				std::wcout << L"\t\tFileSize: " <<
					ULARGE_INTEGER{ info.nFileSizeLow, info.nFileSizeHigh} << std::endl;
				std::wcout << L"\t\tFileIndex: " <<
					ULARGE_INTEGER{ info.nFileIndexLow, info.nFileIndexHigh } << std::endl;
				std::wcout << std::endl;
			}
		}
		{
			FILE_STANDARD_INFO info = {};

			if (GetFileInformationByHandleEx(
				handle,
				FILE_INFO_BY_HANDLE_CLASS::FileStandardInfo,
				&info,
				sizeof(FILE_STANDARD_INFO)))
			{
				std::wcout << L"\tFileStandardInfo:" << std::endl;
				std::wcout << L"\t\tAllocationSize: " << info.AllocationSize << std::endl;
				std::wcout << L"\t\tEndOfFile: " << info.EndOfFile << std::endl;
				std::wcout << L"\t\tNumberOfLinks: " << info.NumberOfLinks << std::endl;
				std::wcout << L"\t\tDeletePending: " << info.DeletePending << std::endl;
				std::wcout << L"\t\tDirectory: " << info.Directory << std::endl;
				std::wcout << std::endl;
			}
		}
		{
			FILE_ALIGNMENT_INFO info = {};

			if (GetFileInformationByHandleEx(
				handle,
				FILE_INFO_BY_HANDLE_CLASS::FileAlignmentInfo,
				&info,
				sizeof(FILE_ALIGNMENT_INFO)))
			{
				std::wcout << L"\tFileAlignmentInfo:" << std::endl;
				std::wcout << L"\t\tAlignmentRequirement: "
					<< info.AlignmentRequirement << std::endl;
				std::wcout << std::endl;
			}
		}
		{
			FILE_STORAGE_INFO info = {};

			if (GetFileInformationByHandleEx(
				handle,
				FILE_INFO_BY_HANDLE_CLASS::FileStorageInfo,
				&info,
				sizeof(FILE_STORAGE_INFO)))
			{

				std::wcout << L"\tFileStorageInfo:" << std::endl;

				std::wcout << L"\t\tLogicalBytesPerSector: "
					<< info.LogicalBytesPerSector << std::endl;

				std::wcout << L"\t\tPhysicalBytesPerSectorForAtomicity: "
					<< info.PhysicalBytesPerSectorForAtomicity << std::endl;

				std::wcout << L"\t\tPhysicalBytesPerSectorForPerformance: "
					<< info.PhysicalBytesPerSectorForPerformance << std::endl;

				std::wcout << L"\t\tFileSystemEffectivePhysicalBytesPerSectorForAtomicity: "
					<< info.FileSystemEffectivePhysicalBytesPerSectorForAtomicity << std::endl;

				std::wcout << L"\t\tFlags: "
					<< info.Flags << std::endl;

				std::wcout << L"\t\tByteOffsetForSectorAlignment: "
					<< info.ByteOffsetForSectorAlignment << std::endl;

				std::wcout << L"\t\tByteOffsetForPartitionAlignment: " <<
					info.ByteOffsetForPartitionAlignment << std::endl;

				std::wcout << std::endl;
			}
		}
		{
			FILE_ID_INFO info = {};

			if (GetFileInformationByHandleEx(
				handle,
				FILE_INFO_BY_HANDLE_CLASS::FileIdInfo,
				&info,
				sizeof(FILE_ID_INFO)))
			{
				std::wcout << L"\tFileIdInfo:" << std::endl;
				std::wcout << L"\t\tVolumeSerialNumber: " << info.VolumeSerialNumber << std::endl;
				std::wcout << L"\t\tFileId: " << info.FileId << std::endl;
				std::wcout << std::endl;
			}
		}
		{
			std::wstring buffer(MAX_PATH + 1, '\0');

			if (GetVolumeInformationByHandleW(handle,
				nullptr,
				0,
				nullptr,
				nullptr,
				nullptr,
				buffer.data(),
				MAX_PATH))
			{
				std::wcout << L"\tVolumeInformationByHandle:" << std::endl;
				std::wcout << L"\t\tFileSystemName: " << buffer << std::endl;
				std::wcout << std::endl;
			}
		}
	}
}