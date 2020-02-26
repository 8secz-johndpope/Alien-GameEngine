#include "Application.h"
#include "Component.h"
#include "ComponentTransform.h"
#include "GameObject.h"
#include "ComponentBone.h"
#include "ComponentMaterial.h"
#include "ResourceMesh.h"
#include "ResourceBone.h"
#include "ComponentDeformableMesh.h"
#include "mmgr/mmgr.h"

ComponentDeformableMesh::ComponentDeformableMesh(GameObject* attach) : ComponentMesh(attach)
{
	type = ComponentType::DEFORMABLE_MESH;
	name = "Deformable Mesh";
}

ComponentDeformableMesh::~ComponentDeformableMesh()
{
	if (game_object_attached != nullptr && game_object_attached->HasComponent(ComponentType::MATERIAL))
	{
		static_cast<ComponentMaterial*>(game_object_attached->GetComponent(ComponentType::MATERIAL))->not_destroy = false;
	}
	if (mesh != nullptr && mesh->is_custom) {
		mesh->DecreaseReferences();
	}
	if (deformable_mesh)
	{
		delete deformable_mesh;
		deformable_mesh = nullptr;
	}
	//clear deformable mesh?
}

void ComponentDeformableMesh::AttachSkeleton(ComponentTransform* root)
{
	root_bone_id = root->game_object_attached->ID;

	//Duplicate mesh
	if (mesh)
		deformable_mesh = new ResourceMesh(mesh);
	
	AttachBone(root);
}

void ComponentDeformableMesh::AttachSkeleton()
{
	if (root_bone_id != 0)
		AttachSkeleton(App->objects->GetGameObjectByID(root_bone_id)->transform);
}

void ComponentDeformableMesh::AttachBone(ComponentTransform* bone_transform)
{
	ComponentBone* c_bone = (ComponentBone*)bone_transform->game_object_attached->GetComponent(ComponentType::BONE);

	if (c_bone)
		bones.push_back(c_bone);

	for each (GameObject* go in bone_transform->game_object_attached->GetChildren())
		AttachBone(go->transform);
}

void ComponentDeformableMesh::UpdateDeformableMesh()
{
	deformable_mesh->Reset();

	for (std::vector<ComponentBone*>::iterator it = bones.begin(); it != bones.end(); ++it)
	{
		ResourceBone* r_bone = (ResourceBone*)(*it)->GetBone();

		float4x4 matrix = (*it)->game_object_attached->transform->global_transformation;
		matrix = game_object_attached->transform->global_transformation.Inverted() * matrix;
		matrix = matrix * r_bone->matrix;

		for (uint i = 0; i < r_bone->num_weights; i++)
		{
			uint index = r_bone->vertex_ids[i];
			float3 original(&mesh->vertex[index * 3]);
			float3 vertex = matrix.TransformPos(original);

			deformable_mesh->vertex[index * 3] += vertex.x * r_bone->weights[i];
			deformable_mesh->vertex[index * 3 + 1] += vertex.y * r_bone->weights[i];
			deformable_mesh->vertex[index * 3 + 2] += vertex.z * r_bone->weights[i];

			if ((mesh->num_vertex) > 0)
			{
				vertex = matrix.TransformPos(float3(&mesh->normals[index*3]));
				deformable_mesh->normals[index * 3] += vertex.x * r_bone->weights[i];
				deformable_mesh->normals[index * 3 + 1] += vertex.y * r_bone->weights[i];
				deformable_mesh->normals[index * 3 + 2] += vertex.z * r_bone->weights[i];
			}
		}
	}
}

void ComponentDeformableMesh::DrawPolygon()
{
	if(deformable_mesh)
		UpdateDeformableMesh();

	if (deformable_mesh == nullptr || mesh->id_index <= 0)
		return;

	if (game_object_attached->IsSelected() || game_object_attached->IsParentSelected()) {
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 1, -1);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	}

	ComponentTransform* transform = game_object_attached->transform;

	if (transform->IsScaleNegative())
		glFrontFace(GL_CW);

	//SKINNING STUFF
	glBindBuffer(GL_ARRAY_BUFFER, deformable_mesh->id_vertex);
	glBufferData(GL_ARRAY_BUFFER, deformable_mesh->num_vertex * sizeof(float) * 3, deformable_mesh->vertex, GL_DYNAMIC_DRAW);

	if (deformable_mesh->normals)
	{
		glBindBuffer(GL_ARRAY_BUFFER, deformable_mesh->id_normals);
		glBufferData(GL_ARRAY_BUFFER, deformable_mesh->num_vertex * sizeof(float) * 3, deformable_mesh->normals, GL_DYNAMIC_DRAW);
	}
	/////////

	glPushMatrix();
	glMultMatrixf(transform->global_transformation.Transposed().ptr());

	glEnableClientState(GL_VERTEX_ARRAY);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, 0.1f);

	glBindBuffer(GL_ARRAY_BUFFER, deformable_mesh->id_vertex);
	glVertexPointer(3, GL_FLOAT, 0, 0);

	if (mesh->uv_cords != nullptr) {
		glBindBuffer(GL_ARRAY_BUFFER, deformable_mesh->id_uv);
		glTexCoordPointer(3, GL_FLOAT, 0, NULL);
	}

	if (mesh->normals != nullptr) {
		glEnableClientState(GL_NORMAL_ARRAY);
		glBindBuffer(GL_ARRAY_BUFFER, deformable_mesh->id_normals);
		glNormalPointer(GL_FLOAT, 0, 0);
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, deformable_mesh->id_index);
	glDrawElements(GL_TRIANGLES, deformable_mesh->num_index * 3, GL_UNSIGNED_INT, 0);

	if (transform->IsScaleNegative())
		glFrontFace(GL_CCW);

	glDisable(GL_TEXTURE_2D);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glBindTexture(GL_TEXTURE_2D, 0);

	glPopMatrix();
}

void ComponentDeformableMesh::SaveComponent(JSONArraypack* to_save)
{
	to_save->SetNumber("Type", (int)type);
	to_save->SetBoolean("ViewMesh", view_mesh);
	to_save->SetBoolean("Wireframe", wireframe);
	to_save->SetBoolean("ViewVertexNormals", view_vertex_normals);
	to_save->SetBoolean("ViewFaceNormals", view_face_normals);
	to_save->SetBoolean("DrawAABB", draw_AABB);
	to_save->SetBoolean("DrawOBB", draw_OBB);
	to_save->SetString("ID", std::to_string(ID));
	to_save->SetString("MeshID", mesh ? std::to_string(mesh->GetID()) : std::to_string(0));
	to_save->SetString("RootBoneID", root_bone_id != 0 ? std::to_string(root_bone_id) : std::to_string(0));
	to_save->SetBoolean("Enabled", enabled);
}

void ComponentDeformableMesh::LoadComponent(JSONArraypack* to_load)
{
	view_mesh = to_load->GetBoolean("ViewMesh");
	wireframe = to_load->GetBoolean("Wireframe");
	view_vertex_normals = to_load->GetBoolean("ViewVertexNormals");
	view_face_normals = to_load->GetBoolean("ViewFaceNormals");
	draw_AABB = to_load->GetBoolean("DrawAABB");
	draw_OBB = to_load->GetBoolean("DrawOBB");
	enabled = to_load->GetBoolean("Enabled");
	root_bone_id = std::stoull(to_load->GetString("RootBoneID"));
	ID = std::stoull(to_load->GetString("ID"));
	u64 mesh_ID = std::stoull(to_load->GetString("MeshID"));
	if (mesh_ID != 0)
	{
		mesh = (ResourceMesh*)App->resources->GetResourceWithID(mesh_ID);
		if (mesh != nullptr)
			mesh->IncreaseReferences();
	}

	GenerateAABB();
	RecalculateAABB_OBB();
}

