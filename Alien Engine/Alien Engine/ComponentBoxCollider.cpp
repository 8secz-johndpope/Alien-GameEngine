#include "ComponentBoxCollider.h"
#include "ComponentRigidBody.h"
#include "ComponentTransform.h"
#include "ComponentMesh.h"
#include "ModulePhysics.h"
#include "GameObject.h"
#include "imgui/imgui.h"

ComponentBoxCollider::ComponentBoxCollider(GameObject* go) : ComponentCollider(go)
{
	name.assign("Box Collider");
	type = ComponentType::BOX_COLLIDER;

	// More useful if its called externaly
	Init();
}

void ComponentBoxCollider::SetSize(float3 value)
{
	if (!value.Equals(size))
	{
		size = value;
		UpdateShape();
	}
}

void ComponentBoxCollider::DrawSpecificInspector()
{
	float3 current_size = size;

	ImGui::Title("Size", 1);			ImGui::DragFloat3("##size", current_size.ptr(), 0.1f, 0.01f, FLT_MAX);

	SetSize(current_size);
}

void ComponentBoxCollider::CreateShape() 
{
	if (!WrapMesh())
	{
		// Default values 
		SetSize(float3::one());
	}

	if (shape == nullptr) UpdateShape();
}

void ComponentBoxCollider::UpdateShape()
{
	if (shape == nullptr)
	{
		delete shape;
	}

	float3 final_size = CheckInvalidCollider(size.Mul(transform->GetGlobalScale().Abs())) * 0.5f;
	shape = new btBoxShape(ToBtVector3(final_size));
	aux_body->setCollisionShape(shape);

	if (rb != nullptr)  rb->UpdateCollider();
}

bool ComponentBoxCollider::WrapMesh()
{
	ComponentMesh* mesh = game_object_attached->GetComponent<ComponentMesh>();

	if (mesh != nullptr)
	{
		SetCenter(mesh->local_aabb.CenterPoint());
		SetSize(mesh->local_aabb.Size());
		return true;
	}

	return false;
}

void ComponentBoxCollider::Reset()
{
	ComponentCollider::Reset();
	SetSize(float3::one());
}

void ComponentBoxCollider::SaveComponent(JSONArraypack* to_save)
{
	ComponentCollider::SaveComponent(to_save);
	to_save->SetFloat3("Size", size);
}

void ComponentBoxCollider::LoadComponent(JSONArraypack* to_load)
{
	ComponentCollider::LoadComponent(to_load);
	SetSize(to_load->GetFloat3("Size"));
}

float3 ComponentBoxCollider::CheckInvalidCollider(float3 size)
{
	return size.Max(float3(0.01, 0.01, 0.01));
}
