#include "ComponentLightDirectional.h"
#include "glew/include/glew.h"
#include "GameObject.h"
#include "imgui/imgui.h"
#include "ComponentTransform.h"
#include "Application.h"
#include "ReturnZ.h"
#include "ModuleResources.h"
#include "ModuleRenderer3D.h"
#include "ComponentMesh.h"
#include "Gizmos.h"
#include "mmgr/mmgr.h"

ComponentLightDirectional::ComponentLightDirectional(GameObject* attach) : Component(attach)
{
	type = ComponentType::LIGHT_DIRECTIONAL;
	App->objects->directional_light_properites.push_back(&light_props);
	App->objects->AddNumOfDirLights();

#ifndef GAME_VERSION
	bulb = new ComponentMesh(game_object_attached);
	bulb->mesh = App->resources->light_mesh;
#endif
}

ComponentLightDirectional::~ComponentLightDirectional()
{
#ifndef GAME_VERSION
	delete bulb;
#endif

	App->objects->directional_light_properites.remove(&light_props);
	App->objects->ReduceNumOfDirLights();
}

void ComponentLightDirectional::LightLogic()
{
	ComponentTransform* transform=(ComponentTransform*)game_object_attached->GetComponent(ComponentType::TRANSFORM);
	light_props.position = float3(transform->GetGlobalPosition().x, transform->GetGlobalPosition().y, transform->GetGlobalPosition().z);
	light_props.direction = game_object_attached->transform->GetGlobalRotation().WorldZ();
#ifndef GAME_VERSION
	if (App->objects->printing_scene)
	{
		if (this->game_object_attached->IsSelected())
		{
			App->renderer3D->BeginDebugDraw(math::float4(0.0f, 1.0f, 0.0f, 1.0f));
			Gizmos::DrawLine(light_props.position, (light_props.position + light_props.direction * 3), Color::Green(), 2.0f);
			App->renderer3D->EndDebugDraw();
		}
		else
		{
			App->renderer3D->BeginDebugDraw(math::float4(0.0f, 1.0f, 0.0f, 1.0f));
			Gizmos::DrawLine(light_props.position, (light_props.position + light_props.direction * 3), Color::Green(), 0.1f);
			App->renderer3D->EndDebugDraw();
		}
	}
#endif
}

bool ComponentLightDirectional::DrawInspector()
{
	static bool en;
	ImGui::PushID(this);
	en = enabled;
	if (ImGui::Checkbox("##CmpActive", &en)) {
		ReturnZ::AddNewAction(ReturnZ::ReturnActions::CHANGE_COMPONENT, this);
		enabled = en;
		if (!enabled)
			OnDisable();
		else
			OnEnable();
	}
	ImGui::PopID();
	ImGui::SameLine();

	if (ImGui::CollapsingHeader("Light Directional", &not_destroy, ImGuiTreeNodeFlags_DefaultOpen))
	{
		RightClickMenu("Light Directional");

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::PushID("printiconlight");
		ImGui::Checkbox("Print Icon", &print_icon);
		ImGui::PopID();

		// Parameters ---------
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Settings:");
		ImGui::DragFloat("Intensity", &light_props.intensity, 0.01f, 0.0f, 2.0f);
		ImGui::ColorEdit3("Ambient", light_props.ambient.ptr());
		ImGui::ColorEdit3("Diffuse", light_props.diffuse.ptr());
		ImGui::ColorEdit3("Specular", light_props.specular.ptr());
		
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
	}
	else
		RightClickMenu("Light Directional");

	return true;
}

void ComponentLightDirectional::OnDisable()
{

}

void ComponentLightDirectional::Clone(Component* clone)
{
	clone->enabled = enabled;
	clone->not_destroy = not_destroy;
	ComponentLightDirectional* light = (ComponentLightDirectional*)clone;
	light->renderer_id = renderer_id;
	light->print_icon = print_icon;
}

void ComponentLightDirectional::Reset()
{
	print_icon = true;
}

void ComponentLightDirectional::SetComponent(Component* component)
{
	if (component->GetType() == type) {

		ComponentLightDirectional* light = (ComponentLightDirectional*)component;

		renderer_id = light->renderer_id;
		print_icon = light->print_icon;
	}
}

void ComponentLightDirectional::SaveComponent(JSONArraypack* to_save)
{
	to_save->SetNumber("Type", (int)type);
	to_save->SetBoolean("Enabled", enabled);
	to_save->SetString("ID", std::to_string(ID).data());
	to_save->SetBoolean("PrintIcon", print_icon);

	to_save->SetNumber("Intensity", float(light_props.intensity));
	to_save->SetFloat3("Position", float3(light_props.position));
	to_save->SetFloat3("Direction", float3(light_props.direction));
	to_save->SetFloat3("Ambient", float3(light_props.ambient));
	to_save->SetFloat3("Diffuse", float3(light_props.diffuse));
	to_save->SetFloat3("Specular", float3(light_props.specular));
}

void ComponentLightDirectional::LoadComponent(JSONArraypack* to_load)
{
	enabled = to_load->GetBoolean("Enabled");
	ID = std::stoull(to_load->GetString("ID"));
	print_icon = to_load->GetBoolean("PrintIcon");

	light_props.intensity = (float)to_load->GetNumber("Intensity");
	light_props.position = to_load->GetFloat3("Position");
	light_props.direction = to_load->GetFloat3("Direction");
	light_props.ambient = to_load->GetFloat3("Ambient");
	light_props.diffuse = to_load->GetFloat3("Diffuse");
	light_props.specular = to_load->GetFloat3("Specular");
}

void ComponentLightDirectional::DrawIconLight()
{
	if (bulb != nullptr && print_icon)
	{
		ComponentTransform* transform = (ComponentTransform*)game_object_attached->GetComponent(ComponentType::TRANSFORM);
		float3 pos = transform->GetGlobalPosition();
		float4x4 matrix = float4x4::FromTRS({ pos.x - 0.133f, pos.y, pos.z }, transform->GetGlobalRotation(), { 0.2f, 0.18f, 0.2f });
		glDisable(GL_LIGHTING);
		Gizmos::DrawPoly(bulb->mesh, matrix, Color(0.0f, 255.0f, 0.0f));
		glEnable(GL_LIGHTING);
	}
}