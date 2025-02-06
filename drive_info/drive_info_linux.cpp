#include "drive_info.hpp"
#include <cassert>
#include <iostream>
#include <system_error>
#include <libudev.h> // apt install libudev-dev

namespace ostrinkets
{
	class udev_context
	{
	public:
		udev_context()
		{
			if (!_context)
			{
				throw std::system_error(errno, std::system_category(), "udev_new failed");
			}
		}

		~udev_context()
		{
			for (udev_enumerate* enumerator : _enumerators)
			{
				udev_enumerate_unref(enumerator);
			}

			if (_context)
			{
				udev_unref(_context);
			}
		}

		operator udev* () const
		{
			return _context;
		}

		udev_enumerate* create_enumerator()
		{
			udev_enumerate* enumerator = udev_enumerate_new(_context);

			if (!enumerator)
			{
				throw std::system_error(errno, std::system_category(), "udev_enumerate_new failed");
				
			}

			return _enumerators.emplace_back(enumerator);
		}

	private:
		udev* _context = udev_new();
		std::vector<udev_enumerate*> _enumerators;
	};

	class device
	{
	public:
		device(udev_context& context, udev_list_entry* device_list_entry) :
			_context(context)
		{
			const char* path = udev_list_entry_get_name(device_list_entry);

			if (!path)
			{
				throw std::system_error(errno, std::system_category(), "udev_list_entry_get_name failed");
			}

			_device = udev_device_new_from_syspath(context, path);

			if (!_device)
			{
				throw std::system_error(errno, std::system_category(), "udev_device_new_from_syspath failed");
			}
		}

		~device()
		{
			if (_device)
			{
				udev_device_unref(_device);
			}
		}

		operator udev_device* () const
		{
			return _device;
		}

		std::string device_type() const
		{
			const char* type = udev_device_get_devtype(_device);

			if (!type)
			{
				throw std::system_error(errno, std::system_category(), "udev_device_get_devtype failed");
			}

			return type;
		}

		std::string system_name() const
		{
			const char* system_name = udev_device_get_sysname(_device);

			if (!system_name)
			{
				throw std::system_error(errno, std::system_category(), "udev_device_get_sysname failed");
			}

			return system_name;
		}

		udev_list_entry* partitions()
		{
			udev_enumerate* enumerator = _context.create_enumerator();
			udev_enumerate_add_match_parent(enumerator, _device);
			udev_enumerate_add_match_property(enumerator, "DEVTYPE", "partition");
			udev_enumerate_scan_devices(enumerator);
			return udev_enumerate_get_list_entry(enumerator);
		}

		bool is_loop_device() const
		{
			return system_name().compare(0, 4, "loop") == 0;
		}

	private:
		udev_context& _context;
		udev_device* _device = nullptr;
	};


	void fill_drive_info(const device& device, drive_info& di)
	{
		const char* path = udev_device_get_devnode(device);

		if (!path)
		{
			throw std::system_error(errno, std::system_category(), "udev_device_get_devnode failed");
		}

		di.path = path;

		const char* description = udev_device_get_property_value(device, "ID_MODEL");

		if (!description)
		{
			throw std::system_error(errno, std::system_category(), "udev_device_get_property_value failed");
		}

		di.description = description;

		// https://people.redhat.com/msnitzer/docs/io-limits.txt
		const char* size_str = udev_device_get_sysattr_value(device, "size");
		const char* block_size_str = udev_device_get_sysattr_value(device, "queue/logical_block_size");

		if (!size_str || !block_size_str)
		{
			throw std::system_error(errno, std::system_category(), "udev_device_get_sysattr_value failed");
		}

		uint64_t size = strtoull(size_str, nullptr, 10);
		uint64_t block_size = strtoull(block_size_str, nullptr, 10);
		assert(size && block_size);
		di.capacity = size * block_size;
	}

	std::vector<drive_info> get_drive_info()
	{
		udev_context context;
		udev_enumerate* enumerator = context.create_enumerator();
		udev_enumerate_add_match_subsystem(enumerator, "block");
		udev_enumerate_scan_devices(enumerator);

		udev_list_entry* device_list = udev_enumerate_get_list_entry(enumerator);

		std::vector<drive_info> result;

		for (udev_list_entry* device_iter = device_list;
			device_iter;
			device_iter = udev_list_entry_get_next(device_iter))
		{
			device dev(context, device_iter);

			if (dev.device_type() != "disk" || dev.is_loop_device())
			{
				continue;
			}

			drive_info di;
			fill_drive_info(dev, di);

			for (udev_list_entry* partition_iter = dev.partitions();
				partition_iter;
				partition_iter = udev_list_entry_get_next(partition_iter))
			{
				++di.partitions;
			}

			result.emplace_back(di);
		}

		return result;
	}
}
