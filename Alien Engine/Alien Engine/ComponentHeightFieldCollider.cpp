#include "Application.h"
#include "ComponentPhysics.h"
#include "ComponentHeightFieldCollider.h"
#include "ComponentTransform.h"
#include "ComponentMesh.h"
#include "ResourceMesh.h"
#include "GameObject.h"
#include "imgui/imgui.h"

#include "Maths.h"

#include <limits>

#include "mmgr/mmgr.h"


ComponentHeightFieldCollider::ComponentHeightFieldCollider(GameObject* go) : ComponentCollider(go)
{
	name.assign("HeightField Collider");
	type = ComponentType::HEIGHTFIELD_COLLIDER;


	shape = CreateHeightField();
	if (!shape)
		shape = App->physx->CreateShape(PxBoxGeometry(0.5f, 0.5f, 0.5f), *material);
	App->SendAlienEvent(this, AlienEventType::COLLIDER_ADDED);
	InitCollider();
}

void ComponentHeightFieldCollider::ScaleChanged()
{
}

PxShape* ComponentHeightFieldCollider::CreateHeightField()
{
	const ComponentMesh* cmesh = GetMesh();
	if (!cmesh)
		return nullptr;

	ResourceMesh* mesh = cmesh->mesh;
	if (!mesh)
		return nullptr;

	// * ------------ Terrain mesh ------------- * //
	// get mesh vertices to make own copy
	uint nverts = mesh->num_vertex;
	uint nindices = mesh->num_index;
	uint ntris = mesh->num_faces;
	float* vertices = mesh->vertex;

	// * -------- NOTE: all assumptions consider this geometry as a plane ---------- * //
	if (ntris % 2 != 0) {
		LOG_ENGINE("Error creating heightfield, num vertices not even: %i", nverts);
		return nullptr;
	}

	// create vectors to sort vertices 
	std::vector<float3> sorted_vertices;
	for (uint i = 0; i < nverts * 3; i += 3) {
		sorted_vertices.push_back(float3(vertices[i], vertices[i + 1], vertices[i + 2]));
	}
	
	// sort by x then z
	std::stable_sort(sorted_vertices.begin(), sorted_vertices.end(), CompareX);
	std::stable_sort(sorted_vertices.begin(), sorted_vertices.end(), CompareZ);
	
	// find duplicates on x and z to extract columns and rows
	std::map<float, int> columnCount;
	for (uint i = 0; i < sorted_vertices.size(); ++i)
	{
		auto inserted = columnCount.insert(std::pair<float, int>(sorted_vertices[i].x, 1));
		if (inserted.second == false)
			inserted.first->second++;
	}
	uint numCols = columnCount.size() - 1;
	uint numRows = columnCount.begin()->second - 1;

	float3 minvalue = sorted_vertices[sorted_vertices.size() - 1 - numCols];
	float3 maxvalue = sorted_vertices[numCols];

	float width = maxvalue.x - minvalue.x;
	float height = maxvalue.z - minvalue.z;

	float heightScale = 1.0f;
	float rowScale = sorted_vertices[1].x - sorted_vertices[0].x;
	float columnScale = sorted_vertices[0].z - sorted_vertices[numRows + 1].z;

	PxHeightFieldSample* samples = new PxHeightFieldSample[sizeof(PxHeightFieldSample) * (numRows * numCols)];

	for (uint i = 0; i < numCols*numRows; ++i)
	{
		samples[i].height = Maths::Map(sorted_vertices[i].y, FLT_MIN, FLT_MAX, SDL_MIN_SINT16, SDL_MAX_SINT16);
		//LOG_ENGINE("BLABLa");

	}

	PxHeightFieldDesc hfDesc;
	hfDesc.format = PxHeightFieldFormat::eS16_TM;
	hfDesc.nbColumns = numCols;
	hfDesc.nbRows = numRows;
	hfDesc.samples.data = samples;
	hfDesc.samples.stride = sizeof(PxHeightFieldSample);
	//hfDesc.flags = PxHeightFieldFlag::eNO_BOUNDARY_EDGES;

	PxHeightField* aHeightField = App->physx->px_cooking->createHeightField(hfDesc, App->physx->px_physics->getPhysicsInsertionCallback());
	if (!aHeightField)
	{
		LOG_ENGINE("Error creating heightfield, cooking heightfield");
		return nullptr;
	}



	return App->physx->CreateShape(PxHeightFieldGeometry(aHeightField, PxMeshGeometryFlags(), heightScale, rowScale, columnScale), *material);

}

void ComponentHeightFieldCollider::DrawSpecificInspector()
{
	//ImGui::Title("Size", 1);
	//if (ImGui::DragFloat3("##size", size.ptr(), 0.1f, 0.01f, FLT_MAX)) {
	//	//ReCreateBoxShape();
	//};
}

void ComponentHeightFieldCollider::Clone(Component* clone)
{
	ComponentHeightFieldCollider* heightfield_clone = (ComponentHeightFieldCollider*)clone;
}

void ComponentHeightFieldCollider::SaveComponent(JSONArraypack* to_save)
{
	ComponentCollider::SaveComponent(to_save);
	//to_save->SetFloat3("Size", size);
}

void ComponentHeightFieldCollider::LoadComponent(JSONArraypack* to_load)
{
	ComponentCollider::LoadComponent(to_load);
	/*size = to_load->GetFloat3("Size");*/

}

bool ComponentHeightFieldCollider::CompareX(float3 a, float3 b)
{
	return a.x < b.x;
}

bool ComponentHeightFieldCollider::CompareZ(float3 a, float3 b)
{
	return a.z > b.z;
}