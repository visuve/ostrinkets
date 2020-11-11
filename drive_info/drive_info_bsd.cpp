#include "drive_info.hpp"
#include <iostream>
#include <cinttypes>
#include <cstring>
#include <libgeom.h>
#include <memory>

namespace fstrinkets
{
	class geom_tree
	{
	public:
		geom_tree()
		{
			m_mesh = std::make_unique<gmesh>();

			if (geom_gettree(m_mesh.get()) != 0)
			{
				std::cerr << "Cannot get GEOM tree" << std::endl;
			}
		}

		~geom_tree()
		{
			geom_deletetree(m_mesh.get());
			m_mesh.reset();
		}

		gclass* find_geom_class(const std::string& name)
		{
			if (!m_mesh)
			{
				return nullptr;
			}

			for (gclass* iter = m_mesh->lg_class.lh_first;
				iter;
				iter = iter->lg_class.le_next)
			{
				if (name.compare(iter->lg_name) == 0)
				{
					return iter;
				}
			}

			std::cerr << "Class: " << name << " not found" << std::endl;
			return nullptr;
		}

	private:
		std::unique_ptr<gmesh> m_mesh;
	};

	void fill_drive_info(ggeom* disk_object, drive_info& di)
	{
		if (!disk_object)
		{
			std::cerr << "Invalid disk object!" << std::endl;
			return;
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

	void fill_partition_info(ggeom* partition_object, drive_info& di)
	{
		if (!partition_object)
		{
			std::cerr << "Invalid partition object!" << std::endl;
			return;
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

		gclass* disk_class = tree.find_geom_class("DISK");

		if (!disk_class)
		{
			return {};
		}

		std::vector<drive_info> drives;

		for (ggeom* disk_object = disk_class->lg_geom.lh_first;
			disk_object;
			disk_object = disk_object->lg_geom.le_next)
		{
			drive_info di;
			fill_drive_info(disk_object, di);
			drives.emplace_back(di);
		}

		gclass* partition_class = tree.find_geom_class("PART");

		if (!partition_class)
		{
			return {};
		}

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