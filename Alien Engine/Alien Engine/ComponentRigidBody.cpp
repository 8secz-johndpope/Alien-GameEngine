#include "Application.h"
#include "ComponentRigidBody.h"
#include "ComponentTransform.h"
#include "ComponentCollider.h"
#include "GameObject.h"
#include "Time.h"
#include "ModuleInput.h"
#include "Event.h"
#include "mmgr/mmgr.h"

ComponentRigidBody::ComponentRigidBody(GameObject* go) : ComponentBasePhysic(go)
{
	// GameObject Components 
	type = ComponentType::RIGID_BODY;

	// Default values 

	for (uint i = 0; i < (uint)ForceMode::MAX; ++i)
	{
		force_to_apply[i] = float3::zero();
		torque_to_apply[i] = float3::zero();
	}

	SetMass(1.0f);
	SetDrag(0.f);
	SetAngularDrag(0.f);

	App->SendAlienEvent(this, AlienEventType::RIGIDBODY_ADDED);
}

ComponentRigidBody::~ComponentRigidBody()
{
	App->SendAlienEvent(this, AlienEventType::RIGIDBODY_DELETED);
}

void ComponentRigidBody::Update()
{
	// Set Go Transform ---------------------------

	if (enabled == false) 
		return;

	if (Time::IsPlaying() != false)
	{
		
	}
	else
	{

	}

	btVector3 freeze_p((float)!freeze_position[0], (float)!freeze_position[1], (float)!freeze_position[2]);
	btVector3 freeze_r((float)!freeze_rotation[0], (float)!freeze_rotation[1], (float)!freeze_rotation[2]);

	//	// Apply Forces ----------------------

	for (uint i = 0; i < (uint)ForceMode::MAX; ++i)
	{
		if (!force_to_apply[i].Equals(float3::zero()))
		{
			switch ((ForceMode)i)
			{
			case ForceMode::FORCE:
				//body->applyCentralForce(ToBtVector3(force_to_apply[i]));
				break;
			case ForceMode::IMPULSE:
				//body->applyCentralImpulse(ToBtVector3(force_to_apply[i]));
				break;
			}
			force_to_apply[i] = float3::zero();
		}
	}

	for (uint i = 0; i < (uint)ForceMode::MAX; ++i)
	{
		if (!torque_to_apply[i].Equals(float3::zero()))
		{
			switch ((ForceMode)i)
			{
			case ForceMode::FORCE:
				//body->applyTorque(ToBtVector3(torque_to_apply[i]));
				break;
			case ForceMode::IMPULSE:
				//body->applyTorqueImpulse(ToBtVector3(torque_to_apply[i]));
				break;
			}

			torque_to_apply[i] = float3::zero();
		}
	}
}

void ComponentRigidBody::OnEnable()
{
}

void ComponentRigidBody::OnDisable()
{
}

bool ComponentRigidBody::DrawInspector()
{

	static bool check;
	check = enabled;

	ImGui::PushID(this);

	if (ImGui::Checkbox("##CmpActive", &check)) {
		enabled = check;
		if (!enabled)
			OnDisable();
		else
			OnEnable();
	}

	bool current_is_kinematic = is_kinematic;
	float current_mass = mass;
	float current_drag = drag;
	float current_angular_drag = angular_drag;

	ImGui::SameLine();

	// RigidBody Config --------------------------------------

	if (ImGui::CollapsingHeader("Rigid Body", &not_destroy))
	{
		ImGui::Spacing();
		ImGui::Title("Is Kinematic", 1);	if (ImGui::Checkbox("##is_kinematic", &current_is_kinematic)) { SetIsKinematic(current_is_kinematic); }
		ImGui::Title("Mass", 1);			if (ImGui::DragFloat("##mass", &current_mass, 0.01f, 0.00f, FLT_MAX)) { SetMass(current_mass); }
		ImGui::Title("Drag", 1);			if (ImGui::DragFloat("##drag", &current_drag, 0.01f, 0.00f, FLT_MAX)) { SetDrag(current_drag); }
		ImGui::Title("Angular Drag", 1);	if (ImGui::DragFloat("##angular_drag", &current_angular_drag, 0.01f, 0.00f, FLT_MAX)) { SetAngularDrag(current_angular_drag); }

		ImGui::Title("Freeze", 1);	ImGui::Text("");
		ImGui::Spacing();
		ImGui::PushID(freeze_position);
		ImGui::Title("Position", 2);
		ImGui::Checkbox("X", &freeze_position[0]); ImGui::SameLine();
		ImGui::Checkbox("Y", &freeze_position[1]); ImGui::SameLine();
		ImGui::Checkbox("Z", &freeze_position[2]);
		ImGui::PopID();
		ImGui::PushID(freeze_rotation);
		ImGui::Title("Rotation", 2);
		ImGui::Checkbox("X", &freeze_rotation[0]); ImGui::SameLine();
		ImGui::Checkbox("Y", &freeze_rotation[1]); ImGui::SameLine();
		ImGui::Checkbox("Z", &freeze_rotation[2]);
		ImGui::PopID();
		ImGui::Spacing();
	}

	ImGui::PopID();

	return true;
}

void ComponentRigidBody::SaveComponent(JSONArraypack* to_save)
{
	to_save->SetBoolean("Enabled", enabled);
	to_save->SetNumber("Type", (int)type);

	to_save->SetNumber("IsKinematic", is_kinematic);
	to_save->SetNumber("Mass", mass);
	to_save->SetNumber("Drag", drag);
	to_save->SetNumber("AngularDrag", angular_drag);

	to_save->SetBoolean("FreezePosX", freeze_position[0]);
	to_save->SetBoolean("FreezePosY", freeze_position[1]);
	to_save->SetBoolean("FreezePosZ", freeze_position[2]);

	to_save->SetBoolean("FreezeRotX", freeze_rotation[0]);
	to_save->SetBoolean("FreezeRotY", freeze_rotation[1]);
	to_save->SetBoolean("FreezeRotZ", freeze_rotation[2]);
}

void ComponentRigidBody::LoadComponent(JSONArraypack* to_load)
{
	enabled = to_load->GetBoolean("Enabled");
	if (enabled == false)
	{
		OnDisable();
	}

	SetIsKinematic(to_load->GetNumber("IsKinematic"));
	SetMass(to_load->GetNumber("Mass"));
	SetDrag(to_load->GetNumber("Drag"));
	SetAngularDrag(to_load->GetNumber("AngularDrag"));

	freeze_position[0] = to_load->GetBoolean("FreezePosX");
	freeze_position[1] = to_load->GetBoolean("FreezePosY");
	freeze_position[2] = to_load->GetBoolean("FreezePosZ");

	freeze_rotation[0] = to_load->GetBoolean("FreezeRotX");
	freeze_rotation[1] = to_load->GetBoolean("FreezeRotY");
	freeze_rotation[2] = to_load->GetBoolean("FreezeRotZ");
}

// Forces Functions ------------------------------

void ComponentRigidBody::AddForce(const float3 force, ForceMode mode, Space space)
{
	float3 final_force = force;

	if (space == Space::Local)
	{
		final_force = transform->global_transformation.RotatePart().Mul(final_force);
	}

	force_to_apply[(uint)mode] += final_force;
}

void ComponentRigidBody::AddTorque(const float3 force, ForceMode mode, Space space)
{
	float3 final_force = force;

	if (space == Space::Local)
	{
		final_force = transform->global_transformation.RotatePart().Mul(final_force);
	}

	torque_to_apply[(uint)mode] += final_force;
}

// Rigid Body Values ----------------------------

void ComponentRigidBody::SetPosition(const float3 new_position)
{
	//btTransform new_trans = body->getCenterOfMassTransform();
	//new_trans.setOrigin( (collider)
	//	? ToBtVector3(new_position + collider->GetWorldCenter())
	//	: ToBtVector3(new_position));
	//
	//body->setCenterOfMassTransform(new_trans);

	//btQuaternion rotation = new_trans.getRotation();
	//btVector3 position = (collider) ? new_trans.getOrigin() - ToBtVector3(collider->GetWorldCenter()) : new_trans.getOrigin();

	//transform->SetGlobalPosition(float3(position));
	//transform->SetGlobalRotation(math::Quat(rotation));
}

void ComponentRigidBody::SetRotation(const Quat new_rotation)
{
	/*btTransform new_trans = body->getCenterOfMassTransform();
	new_trans.setRotation(ToBtQuaternion(new_rotation));

	body->setCenterOfMassTransform(new_trans);

	btQuaternion rotation = new_trans.getRotation();
	btVector3 position = (collider) ? new_trans.getOrigin() - ToBtVector3(collider->GetWorldCenter()) : new_trans.getOrigin();

	transform->SetGlobalPosition(float3(position));
	transform->SetGlobalRotation(math::Quat(rotation));*/
}

void ComponentRigidBody::SetTransform(const float3 new_position, const Quat new_rotation)
{
	/*btTransform new_trans = ToBtTransform((collider)
		? new_position + collider->GetWorldCenter()
		: new_position, new_rotation);

	body->setCenterOfMassTransform(new_trans);

	btQuaternion rotation = new_trans.getRotation();
	btVector3 position = (collider) ? new_trans.getOrigin() - ToBtVector3(collider->GetWorldCenter()) : new_trans.getOrigin();

	transform->SetGlobalPosition(float3(position));
	transform->SetGlobalRotation(math::Quat(rotation));*/
}

float3 ComponentRigidBody::GetPosition()
{
	return float3();
}

Quat ComponentRigidBody::GetRotation()
{
	return Quat();
}

float3 ComponentRigidBody::GetPosition() const
{
	//btTransform trans = body->getCenterOfMassTransform();
	//btVector3 pos = trans.getOrigin();
	//return float3(pos.x(), pos.y(), pos.z());
	return float3();
}

Quat ComponentRigidBody::GetRotation() const
{
	//btTransform trans = body->getCenterOfMassTransform();
	//btQuaternion rot = trans.getRotation();
	//return Quat(rot.x(), rot.y(), rot.z(), rot.w());
	return Quat();
}

void ComponentRigidBody::SetIsKinematic(const bool value)
{
}

void ComponentRigidBody::SetMass(const float value)
{

}

void ComponentRigidBody::SetDrag(const float value)
{

}

void ComponentRigidBody::SetAngularDrag(const float value)
{

}



void ComponentRigidBody::SetVelocity(float3 velocity)
{
}

float3 ComponentRigidBody::GetVelocity()
{
	return float3();
}

void ComponentRigidBody::SetAngularVelocity(float3 velocity)
{
}

float3 ComponentRigidBody::GetAngularVelocity()
{
	return float3();
}

void ComponentRigidBody::SetBodyTranform(const float3& pos, const Quat& rot)
{
}

void ComponentRigidBody::HandleAlienEvent(const AlienEvent& e)
{

}
