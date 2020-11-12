#include "drive_info.hpp"

#include <any>
#include <cassert>
#include <cinttypes>
#include <iostream>
#include <WbemCli.h>
#include <Windows.h>
#include <wrl/client.h>

namespace fstrinkets
{
	template<typename T>
	using com_ptr = Microsoft::WRL::ComPtr<T>;

	class com_variant
	{
	public:
		com_variant()
		{
			VariantInit(&m_value);
		}

		~com_variant()
		{
			VariantClear(&m_value);
		}

		operator VARIANT* ()
		{
			return &m_value;
		}

		VARIANT* operator -> ()
		{
			return &m_value;
		}

	private:
		VARIANT m_value;
	};

	std::string to_utf8(const std::wstring& unicode)
	{
		std::string utf8;

		int required = WideCharToMultiByte(
			CP_UTF8,
			0, unicode.c_str(),
			static_cast<int>(unicode.length()),
			nullptr, 0,
			nullptr,
			nullptr);

		if (required > 0)
		{
			utf8.resize(static_cast<size_t>(required));
			int result = WideCharToMultiByte(
				CP_UTF8,
				0,
				unicode.c_str(),
				static_cast<int>(unicode.length()),
				&utf8.front(),
				required,
				nullptr,
				nullptr);

			if (result != required)
			{
				utf8.resize(static_cast<size_t>(result));
			}
		}

		return utf8;
	}

	class wmi_drive_info
	{
	public:
		wmi_drive_info()
		{
			m_result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

			if (FAILED(m_result))
			{
				std::cerr << "CoInitializeEx failed: 0x"
					<< std::hex << m_result << std::endl;
				return;
			}

			m_result = CoInitializeSecurity(
				nullptr,
				-1,
				nullptr,
				nullptr,
				RPC_C_AUTHN_LEVEL_CONNECT,
				RPC_C_IMP_LEVEL_IMPERSONATE,
				nullptr,
				EOAC_NONE,
				0);

			if (FAILED(m_result))
			{
				std::cerr << "CoInitializeSecurity failed: 0x"
					<< std::hex << m_result << std::endl;
				return;
			}

			m_result = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&m_locator));

			if (FAILED(m_result))
			{
				std::cerr << "CoCreateInstance failed: 0x"
					<< std::hex << m_result << std::endl;
				return;
			}

			m_result = m_locator->ConnectServer(
				BSTR(L"root\\CIMV2"),
				nullptr,
				nullptr,
				nullptr,
				WBEM_FLAG_CONNECT_USE_MAX_WAIT,
				nullptr,
				nullptr,
				&m_service);

			if (FAILED(m_result))
			{
				std::cerr << "IWbemLocator::ConnectServer failed: 0x"
					<< std::hex << m_result << std::endl;
				return;
			}
		}

		// https://docs.microsoft.com/en-us/windows/win32/cimwin32prov/win32-diskdrive
		std::vector<drive_info> list_drives()
		{
			if (FAILED(m_result))
			{
				return {};
			}

			com_ptr<IEnumWbemClassObject> enumerator;

			m_result = m_service->ExecQuery(
				BSTR(L"WQL"),
				BSTR(L"SELECT Caption, DeviceID, Partitions, Size FROM Win32_DiskDrive"),
				WBEM_FLAG_FORWARD_ONLY,
				nullptr,
				&enumerator);

			if (FAILED(m_result))
			{
				std::cerr << "IWbemServices::ExecQuery failed: 0x"
					<< std::hex << m_result << std::endl;
				return {};
			}

			std::vector<drive_info> drives;

			while (SUCCEEDED(m_result))
			{
				com_ptr<IWbemClassObject> class_object;
				ULONG count = 0;
				m_result = enumerator->Next(WBEM_INFINITE, 1, &class_object, &count);

				if (FAILED(m_result))
				{
					std::cerr << "IEnumWbemClassObject::Next failed: 0x"
						<< std::hex << m_result << std::endl;
					return drives;
				}

				if (m_result == WBEM_S_FALSE)
				{
					return drives;
				}

				drive_info drive;
				drive.description = query_value<std::string>(class_object.Get(), L"Caption", VT_BSTR);
				drive.path = query_value<std::string>(class_object.Get(), L"DeviceID", VT_BSTR);
				drive.partitions = query_value<uint32_t>(class_object.Get(), L"Partitions", VT_I4);
				drive.capacity = query_value<uint64_t>(class_object.Get(), L"Size", VT_BSTR);
				drives.emplace_back(drive);
			}

			return drives;
		}

	private:
		template <typename T, size_t N>
		T query_value(
			IWbemClassObject* classObject,
			const wchar_t(&name)[N],
			VARTYPE type)
		{
			std::any result;
			com_variant variant;

			m_result = classObject->Get(name, 0, variant, nullptr, nullptr);

			if (FAILED(m_result) || variant->vt == VT_NULL)
			{
				std::wcerr << L"Fecthing " << name << L" failed. HRESULT: 0x"
					<< std::hex << m_result << std::endl;

				return T();
			}

			assert(type == variant->vt);

			switch (type)
			{
				case VT_BOOL:
					result = variant->boolVal ? true : false;
					break;
				case VT_I4:
					result = variant->uintVal;
					break;
				case VT_BSTR:
					if (typeid(T) == typeid(std::string))
					{
						result = to_utf8(variant->bstrVal);
					}
					else
					{
						// Sometimes large integer values are actually returned as strings.
						// Thanks again Microsoft ,,|,
						result = wcstoumax(variant->bstrVal, nullptr, 10);
					}
					break;
				default:
					std::cerr << type << " not handled!" << std::endl;
					break;
			}

			return std::any_cast<T>(result);
		}

		HRESULT m_result;
		com_ptr<IWbemLocator> m_locator;
		com_ptr<IWbemServices> m_service;
	};

	std::vector<drive_info> get_drive_info()
	{
		return wmi_drive_info().list_drives();
	}
}

