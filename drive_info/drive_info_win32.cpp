#include "drive_info.hpp"

#include <any>
#include <cassert>
#include <cinttypes>
#include <iostream>
#include <system_error>
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
			VariantInit(&_value);
		}

		~com_variant()
		{
			VariantClear(&_value);
		}

		operator VARIANT* ()
		{
			return &_value;
		}

		VARIANT* operator -> ()
		{
			return &_value;
		}

	private:
		VARIANT _value;
	};

	std::string to_utf8(std::wstring_view unicode)
	{
		std::string utf8;

		int required = WideCharToMultiByte(
			CP_UTF8,
			0,
			unicode.data(),
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
				unicode.data(),
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
			_result = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

			if (FAILED(_result))
			{
				throw std::system_error(_result, std::system_category(), "CoInitializeEx failed");
			}

			_result = CoInitializeSecurity(
				nullptr,
				-1,
				nullptr,
				nullptr,
				RPC_C_AUTHN_LEVEL_CONNECT,
				RPC_C_IMP_LEVEL_IMPERSONATE,
				nullptr,
				EOAC_NONE,
				0);

			if (FAILED(_result))
			{
				throw std::system_error(_result, std::system_category(), "CoInitializeSecurity failed");
			}

			_result = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_ALL, IID_PPV_ARGS(&_locator));

			if (FAILED(_result))
			{
				throw std::system_error(_result, std::system_category(), "CoCreateInstance failed");
			}

			_result = _locator->ConnectServer(
				BSTR(L"root\\CIMV2"),
				nullptr,
				nullptr,
				nullptr,
				WBEM_FLAG_CONNECT_USE_MAX_WAIT,
				nullptr,
				nullptr,
				&_service);

			if (FAILED(_result))
			{
				throw std::system_error(_result, std::system_category(), "IWbemLocator::ConnectServer failed");
			}
		}

		// https://docs.microsoft.com/en-us/windows/win32/cimwin32prov/win32-diskdrive
		std::vector<drive_info> list_drives()
		{
			if (FAILED(_result))
			{
				throw std::logic_error("wmi_drive_info not initialized properly.");
			}

			com_ptr<IEnumWbemClassObject> enumerator;

			_result = _service->ExecQuery(
				BSTR(L"WQL"),
				BSTR(L"SELECT Caption, DeviceID, Partitions, Size FROM Win32_DiskDrive"),
				WBEM_FLAG_FORWARD_ONLY,
				nullptr,
				&enumerator);

			if (FAILED(_result))
			{
				throw std::system_error(_result, std::system_category(), "IWbemServices::ExecQuery failed");
			}

			std::vector<drive_info> drives;

			while (SUCCEEDED(_result))
			{
				com_ptr<IWbemClassObject> class_object;
				ULONG count = 0;
				_result = enumerator->Next(WBEM_INFINITE, 1, &class_object, &count);

				if (FAILED(_result))
				{
					throw std::system_error(_result, std::system_category(), "IEnumWbemClassObject::Next failed");
				}

				if (_result == WBEM_S_FALSE)
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

			_result = classObject->Get(name, 0, variant, nullptr, nullptr);

			if (FAILED(_result) || variant->vt == VT_NULL)
			{
				const std::string message("IWbemClassObject::Get(" + to_utf8(name) + ')');
				throw std::system_error(_result, std::system_category(), message);
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

		HRESULT _result;
		com_ptr<IWbemLocator> _locator;
		com_ptr<IWbemServices> _service;
	};

	std::vector<drive_info> get_drive_info()
	{
		return wmi_drive_info().list_drives();
	}
}

