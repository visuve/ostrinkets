#include "drive_info.hpp"
#include <cassert>
#include <iostream>
#include <libudev.h> // apt install libudev-dev

namespace fstrinkets
{
	class udev_context
	{
	public:
		udev_context()
		{
			if (!m_context)
			{
				std::cerr << "Could not create udev context!" << std::endl;
				return;
			}
		}

		~udev_context()
		{
			for (udev_enumerate* enumerator : m_enumerators)
			{
				udev_enumerate_unref(enumerator);
			}

			if (m_context)
			{
				udev_unref(m_context);
			}
		}

		operator udev* () const
		{
			return m_context;
		}

		udev_enumerate* create_enumerator()
		{
			udev_enumerate* enumerator = udev_enumerate_new(m_context);

			if (enumerator)
			{
				return m_enumerators.emplace_back(enumerator);
			}

			std::cerr << "Could not create enumerator!" << std::endl;
			return nullptr;
		}

	private:
		udev* m_context = udev_new();
		std::vector<udev_enumerate*> m_enumerators;
	};

	class device
	{
	public:
		device(udev_context& context, udev_list_entry* device_list_entry) :
			m_context(context)
		{
			const char* path = udev_list_entry_get_name(device_list_entry);

			if (!path)
			{
				std::cerr << "Failed to get device path!" << std::endl;
				return;
			}

			m_device_path = path;
			m_device = udev_device_new_from_syspath(context, path);

			if (!m_device)
			{
				std::cerr << "Failed to get device from path: '"
					<< m_device_path << "'!" << std::endl;
				return;
			}
		}

		~device()
		{
			if (m_device)
			{
				udev_device_unref(m_device);
			}
		}

		operator udev_device* () const
		{
			return m_device;
		}

		std::string device_path() const
		{
			return m_device_path;
		}

		std::string device_type() const
		{
			const char* type = udev_device_get_devtype(m_device);

			if (!type)
			{
				std::cerr << "Failed to get device type from path: '"
					<< m_device_path << "'!" << std::endl;
				return {};
			}

			return type;
		}

		std::string system_name() const
		{
			const char* system_name = udev_device_get_sysname(m_device);

			if (!system_name)
			{
				std::cerr << "Failed to get system name from path: '"
					<< m_device_path << "'!" << std::endl;
				return {};
			}

			return system_name;
		}

		udev_list_entry* partitions()
		{
			udev_enumerate* enumerator = m_context.create_enumerator();
			udev_enumerate_add_match_parent(enumerator, m_device);
			udev_enumerate_add_match_property(enumerator, "DEVTYPE", "partition");
			udev_enumerate_scan_devices(enumerator);
			return udev_enumerate_get_list_entry(enumerator);
		}

		bool is_loop_device() const
		{
			return system_name().compare(0, 4, "loop") == 0;
		}

	private:
		udev_context& m_context;
		udev_device* m_device = nullptr;
		std::string m_device_path;
	};


	void fill_drive_info(const device& device, drive_info& di)
	{
		const char* path = udev_device_get_devnode(device);

		if (!path)
		{
			std::cerr << "Failed to get file system path from: '"
				<< device.device_path() << "'!" << std::endl;
		}
		else
		{
			di.path = path;
		}

		const char* description = udev_device_get_property_value(device, "ID_MODEL");

		if (!description)
		{
			std::cerr << "Failed to get ID_MODEL from: '"
				<< device.device_path() << "'!" << std::endl;
		}
		else
		{
			di.description = description;
		}


		// https://people.redhat.com/msnitzer/docs/io-limits.txt
		const char* size_str = udev_device_get_sysattr_value(device, "size");
		const char* block_size_str = udev_device_get_sysattr_value(device, "queue/logical_block_size");

		if (!size_str)
		{
			std::cerr << "Failed to get size from: '"
				<< device.device_path() << "'!" << std::endl;
		}
		else if (!block_size_str)
		{
			std::cerr << "Failed to get block size from: '"
				<< device.device_path() << "'!" << std::endl;
		}
		else
		{
			uint64_t size = strtoull(size_str, nullptr, 10);
			uint64_t block_size = strtoull(block_size_str, nullptr, 10);
			assert(size && block_size);
			di.capacity = size * block_size;
		}

	}

	std::vector<drive_info> get_drive_info()
	{
		udev_context context;
		udev_enumerate* enumerator = context.create_enumerator();

		if (!enumerator)
		{
			return {};
		}

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
