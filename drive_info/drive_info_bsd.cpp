#include "drive_info.hpp"
#include <iostream>
#include <system_error>
#include <cinttypes>
#include <cstring>
#include <libgeom.h>
#include <memory>

namespace ostrinkets
{
	class geom_tree
	{
	public:
		geom_tree() :
			_result(geom_gettree(_mesh.get()))
		{
			if (_result != 0)
			{
				throw std::system_error(_result, std::system_category(), "geom_gettree failed");
			}
		}

		~geom_tree()
		{
			if (_result == 0)
			{
				geom_deletetree(_mesh.get());
				_mesh.reset();
			}
		}

		gclass* find_geom_class(std::string_view name)
		{
			for (gclass* iter = _mesh->lg_class.lh_first;
				iter;
				iter = iter->lg_class.le_next)
			{
				if (name.compare(iter->lg_name) == 0)
				{
					return iter;
				}
			}

			const std::string message = "Class: " + std::string(name) + " not found";
			throw std::invalid_argument(message);
		}

	private:
		std::unique_ptr<gmesh> _mesh = std::make_unique<gmesh>();
		int _result;
	};

	void fill_drive_info(const ggeom* disk_object, drive_info& di)
	{
		if (!disk_object)
		{
			throw std::invalid_argument("invalid disk object");
		}

		for (gprovider* provider = disk_object->lg_provider.lh_first;
			provider;
			provider = provider->lg_provider.le_next)
		{
			di.path =  provider->lg_name ;
			di.capacity = static_cast<uint64_t>(provider->lg_mediasize);

			for (gconfig* config = provider->lg_config.lh_first;
				config;
				config = config->lg_config.le_next)
			{
				if (std::string("descr").compare(config->lg_name) == 0)
				{
					di.description = config->lg_val;
				}
			}
		}
	}

	void fill_partition_info(const ggeom* partition_object, drive_info& di)
	{
		if (!partition_object)
		{
			throw std::invalid_argument("invalid partition object");
		}

		if (di.path.compare(partition_object->lg_name) != 0)
		{
			return;
		}

		for (gprovider* provider = partition_object->lg_provider.lh_first;
			provider;
			provider = provider->lg_provider.le_next)
		{
			++di.partitions;
		}

		di.path = "/dev/" + di.path; // Here be dragons...
	}

	std::vector<drive_info> get_drive_info()
	{
		geom_tree tree;
		const gclass* disk_class = tree.find_geom_class("DISK");

		std::vector<drive_info> drives;

		for (ggeom* disk_object = disk_class->lg_geom.lh_first;
			disk_object;
			disk_object = disk_object->lg_geom.le_next)
		{
			drive_info di;
			fill_drive_info(disk_object, di);
			drives.emplace_back(di);
		}

		const gclass* partition_class = tree.find_geom_class("PART");

		for (ggeom* partition_object = partition_class->lg_geom.lh_first;
			partition_object;
			partition_object = partition_object->lg_geom.le_next)
		{
			for (drive_info& di : drives)
			{
				fill_partition_info(partition_object, di);
			}
		}

		return drives;
	}
}
