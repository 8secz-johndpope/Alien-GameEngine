#include "Application.h"
#include "Component.h"
#include "ComponentDeformableMesh.h"
#include "ComponentTransform.h"
#include "GameObject.h"
#include "ComponentBone.h"
#include "ComponentMaterial.h"


#include "ResourceMesh.h"
#include "ResourceBone.h"
#include "ResourceTexture.h"
#include "ResourceShader.h"
#include "mmgr/mmgr.h"

#include "Optick/include/optick.h"

ComponentDeformableMesh::ComponentDeformableMesh(GameObject* attach) : ComponentMesh(attach)
{
	type = ComponentType::DEFORMABLE_MESH;
	name = "Deformable Mesh";

	//Change this to resource model so is only one new 
	bones_matrix = new float4x4[100];
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
	if (mesh != nullptr)
		deformable_mesh = new ResourceMesh(mesh);
	
	AttachBone(root);

	if (mesh->id_weights == 0)
	{
		SendWeightsAndID();
	}

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
	OPTICK_EVENT();
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

void ComponentDeformableMesh::SimulateShaderFuntion(ComponentCamera* camera)
{
	//float4x4 projection = camera->getviewmatrix4x4();
	//float4x4 view = transform->getglobalmatrix().transposed()
	//float4x4 model = camera->getviewmatrix4x4();

	for (int i=0; i< mesh->num_vertex*3; i+=3)
	{
		int vertex_id = i / 3;
		float3 vertex = float3(mesh->vertex[i], mesh->vertex[i + 1], mesh->vertex[i + 2]);
		/*float4x4 matrix = flo;*/
		for (int j = vertex_id * 4; j < (vertex_id * 4) + 4; j++)
		{
			if (bones_ID[j] != -1)
			{

			}
			

		}

	}

}

void ComponentDeformableMesh::UpdateBonesMatrix()
{
	uint i = 0;
	for (std::vector<ComponentBone*>::iterator it = bones.begin(); it != bones.end(); ++it, ++i)
	{
		ResourceBone* r_bone = (ResourceBone*)(*it)->GetBone();
		bones_matrix[i] = (*it)->game_object_attached->transform->global_transformation;
		bones_matrix[i] = game_object_attached->transform->global_transformation.Inverted() * bones_matrix[i];
		bones_matrix[i] = bones_matrix[i] * r_bone->matrix;
	}
}


void ComponentDeformableMesh::DrawPolygon(ComponentCamera* camera)
{
	OPTICK_EVENT();
	if (mesh == nullptr || mesh->id_index <= 0)
		return;
	UpdateBonesMatrix();

	ComponentTransform* transform = game_object_attached->transform;
	ComponentMaterial* material = (ComponentMaterial*)game_object_attached->GetComponent(ComponentType::MATERIAL);

	if (transform->IsScaleNegative())
		glFrontFace(GL_CW);

	// -------------------------- Actual Drawing -------------------------- 

	if (material->texture != nullptr)
		glBindTexture(GL_TEXTURE_2D, material->texture->id);

	material->used_shader->Bind();
	material->used_shader->UpdateUniforms();

	glBindVertexArray(mesh->vao);

	// Uniforms
	material->used_shader->SetUniformMat4f("view", camera->GetViewMatrix4x4()); // TODO: About in-game camera?
	material->used_shader->SetUniformMat4f("model", transform->GetGlobalMatrix().Transposed());
	material->used_shader->SetUniformMat4f("projection", camera->GetProjectionMatrix4f4());
	material->used_shader->SetUniformMat4f("gBones", bones_matrix[0], 100);
	//material->used_shader->SetUniform1f("time", Time::GetTimeSinceStart());

	glDrawElements(GL_TRIANGLES, mesh->num_index * 3, GL_UNSIGNED_INT, NULL);
	// --------------------------------------------------------------------- 
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	material->used_shader->Unbind();

	if (transform->IsScaleNegative())
		glFrontFace(GL_CCW);
}

void ComponentDeformableMesh::DrawOutLine()
{
	if (deformable_mesh == nullptr || deformable_mesh->id_index <= 0)
		return;


	if (!glIsEnabled(GL_STENCIL_TEST))
		return;
	if (game_object_attached->IsParentSelected() && !game_object_attached->selected)
	{
		glColor3f(App->objects->parent_outline_color.r, App->objects->parent_outline_color.g, App->objects->parent_outline_color.b);
		glLineWidth(App->objects->parent_line_width);
	}
	else
	{
		glColor3f(App->objects->no_child_outline_color.r, App->objects->no_child_outline_color.g, App->objects->no_child_outline_color.b);
		glLineWidth(App->objects->no_child_line_width);
	}

	glStencilFunc(GL_NOTEQUAL, 1, -1);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glPolygonMode(GL_FRONT, GL_LINE);

	glPushMatrix();
	ComponentTransform* transform = game_object_attached->transform;
	glMultMatrixf(transform->global_transformation.Transposed().ptr());

	glEnableClientState(GL_VERTEX_ARRAY);

	glBindBuffer(GL_ARRAY_BUFFER, deformable_mesh->id_vertex);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, deformable_mesh->id_index);
	glVertexPointer(3, GL_FLOAT, 0, 0);

	glDrawElements(GL_TRIANGLES, deformable_mesh->num_index * 3, GL_UNSIGNED_INT, 0);

	glDisable(GL_STENCIL_TEST);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisableClientState(GL_VERTEX_ARRAY);
	glLineWidth(1);

	glPopMatrix();
}

void ComponentDeformableMesh::DrawMesh()
{
	OPTICK_EVENT();
	if (deformable_mesh == nullptr || deformable_mesh->id_index <= 0)
		return;

	ComponentTransform* transform = game_object_attached->transform;

	glPushMatrix();
	glMultMatrixf(transform->global_transformation.Transposed().ptr());

	glBindTexture(GL_TEXTURE_2D, 0);
	glEnableClientState(GL_VERTEX_ARRAY);

	glLineWidth(App->objects->mesh_line_width);
	glColor3f(App->objects->mesh_color.r, App->objects->mesh_color.g, App->objects->mesh_color.b);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glBindBuffer(GL_ARRAY_BUFFER, deformable_mesh->id_vertex);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, deformable_mesh->id_index);
	glVertexPointer(3, GL_FLOAT, 0, NULL);

	glDrawElements(GL_TRIANGLES, deformable_mesh->num_index * 3, GL_UNSIGNED_INT, NULL);

	glLineWidth(1);
	glDisableClientState(GL_VERTEX_ARRAY);

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

//TODO change this to execute this code only one time
//When loading resouce model
void ComponentDeformableMesh::SendWeightsAndID()
{

	int bone_id = 0;
	//Genereting array of weights and bones_ID
	if (weights == nullptr && bones_ID == nullptr)
	{
		weights = new float[mesh->num_vertex * 4];
		bones_ID = new int[mesh->num_vertex * 4];

		memset(weights, 0, sizeof(float) * mesh->num_vertex * 4);
		memset(bones_ID, -1, sizeof(int) * mesh->num_vertex * 4);
	}

	for (std::vector<ComponentBone*>::iterator component_bone = bones.begin();
		component_bone != bones.end(); component_bone++ , bone_id++)
	{
		FillWeights(bone_id, (*component_bone));
	}
	glBindVertexArray(mesh->vao);
	if (mesh->id_weights == 0)
	{
		glGenBuffers(1, &mesh->id_weights);
	}
	glBindBuffer(GL_ARRAY_BUFFER, mesh->id_weights);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh->num_vertex * 4, weights, GL_STATIC_DRAW);
	
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(3);

	if (mesh->id_bones == 0)
	{
		glGenBuffers(1, &mesh->id_bones);
	}
	glBindBuffer(GL_ARRAY_BUFFER, mesh->id_bones);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh->num_vertex * 4, bones_ID, GL_STATIC_DRAW);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(4);

	//if (weights)
	//{
	//	delete weights;
	//	weights = nullptr;
	//}
	//if (bones_ID)
	//{
	//	delete bones_ID;
	//	bones_ID = nullptr;
	//}
}

void ComponentDeformableMesh::FillWeights(int bone_ID, ComponentBone* component_bone)
{
	ResourceBone* bone = component_bone->GetBone();
	for (int i = 0; i < bone->num_weights; i++)
	{
		int vertex_id = bone->vertex_ids[i];
		for (int j = vertex_id * 4; j < (vertex_id * 4) + 4; j++)
		{
			if (bones_ID[j] == -1)
			{
				weights[j] = bone->weights[i];
				bones_ID[j] = bone_ID;
				break;
			}
		}
	}
}

