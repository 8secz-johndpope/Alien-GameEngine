#pragma once

#include "Globals.h"
#include "Module.h"


#include "Assimp/include/assimp/cimport.h"
#include "Assimp/include/assimp/scene.h"
#include "Assimp/include/assimp/postprocess.h"
#include "Assimp/include/assimp/cfileio.h"
#include "Assimp/include/assimp/mesh.h"
#pragma comment (lib, "Assimp/libx86/assimp.lib")

#pragma comment (lib, "Devil/libx86/DevIL.lib")
#pragma comment (lib, "Devil/libx86/ILU.lib")
#pragma comment (lib, "Devil/libx86/ILUT.lib")

#include <vector>
#include "glew/include/glew.h"
#include "GameObject.h"
#include "ComponentMesh.h"
#include "Shapes.h"

#include "Devil/include/il.h"
#include "Devil/include/ilu.h"
#include "Devil/include/ilut.h"

class ResourceModel;
class ResourceMesh;
class ResourceTexture;

class ModuleImporter : public Module
{
public:
	ModuleImporter(bool start_enabled = true);
	~ModuleImporter();

	bool Init();
	bool Start();
	bool CleanUp();

	// models
	bool LoadModelFile(const char* path); // when dropped
	void LoadParShapesMesh(par_shapes_mesh* p_mesh, ResourceMesh* mesh);
	ResourceMesh* LoadEngineModels(const char* path);
	bool ReImportModel(ResourceModel* model); // when dropped
	void ReImportAnimations(ResourceModel* model, const aiScene* scene);
	void ReImportBones(const aiScene* scene);
	
	// textures
	ResourceTexture* LoadTextureFile(const char* path, bool has_been_dropped = false, bool is_custom = true); // when dropped
	ResourceTexture* LoadEngineTexture(const char* path);
	void LoadTextureToResource(const char* path, ResourceTexture* texture);
	void ApplyTextureToSelectedObject(ResourceTexture* texture);

private:
	
	// models
	void InitScene(const char* path, const aiScene* scene);

	void LoadAnimation(const aiAnimation* animation);
	void LoadBone(const aiBone* bone);
	void LoadMesh(const aiMesh* mesh);
	void LoadNode(const aiNode* node, const aiScene* scene, uint nodeNum);
	void LoadMaterials(const aiMaterial* material);

private:
	ResourceModel* model = nullptr;
};


