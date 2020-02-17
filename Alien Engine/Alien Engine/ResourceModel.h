#pragma once

#include "Resource_.h"
#include <vector>

class ResourceMesh;
class ResourceAnimation;

class ResourceModel : public Resource {

	friend class ModuleImporter;

public:

	ResourceModel();
	virtual ~ResourceModel();

	// meta data
	bool CreateMetaData(const u64& force_id = 0);
	bool ReadBaseInfo(const char* assets_file_path);
	void ReadLibrary(const char* meta_data);
	bool LoadMemory();
	bool DeleteMetaData();

	// create GameObjects
	void ConvertToGameObjects();

private:

	// sort
	static bool SortByFamilyNumber(const ResourceMesh* mesh1, const ResourceMesh* mesh2);

public:
	std::vector<ResourceMesh*> meshes_attached;
	std::vector<ResourceAnimation*> animation_attached;

};