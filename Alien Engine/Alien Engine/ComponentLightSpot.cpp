#include "ComponentLightSpot.h"
#include "glew/include/glew.h"
#include "GameObject.h"
#include "imgui/imgui.h"
#include "ComponentTransform.h"
#include "Application.h"
#include "ReturnZ.h"
#include "ComponentMesh.h"
#include "Gizmos.h"
#include "mmgr/mmgr.h"

ComponentLightSpot::ComponentLightSpot(GameObject* attach) : Component(attach)
{
	type = ComponentType::LIGHT_SPOT;

#ifndef GAME_VERSION
	bulb = new ComponentMesh(game_object_attached);
	bulb->mesh = App->resources->light_mesh;
#endif
}

ComponentLightSpot::~ComponentLightSpot()
{
#ifndef GAME_VERSION
	delete bulb;
#endif

	App->objects->ReduceNumOfSpotLights();
}

void ComponentLightSpot::LightLogic()
{
	ComponentTransform* transform = (ComponentTransform*)game_object_attached->GetComponent(ComponentType::TRANSFORM);
	light_props.position = float3(transform->GetGlobalPosition().x, transform->GetGlobalPosition().y, transform->GetGlobalPosition().z);
}

bool ComponentLightSpot::DrawInspector()
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

	if (ImGui::CollapsingHeader("Light Spot", &not_destroy, ImGuiTreeNodeFlags_DefaultOpen))
	{
		RightClickMenu("Light Spot");

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::PushID("printiconlight");
		ImGui::Checkbox("Print Icon", &print_icon);
		ImGui::PopID();

		// Parameters ---------
		ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Settings:");
		ImGui::DragFloat3("Direction", light_props.direction.ptr(), 0.10f);
		ImGui::DragFloat3("Ambient", light_props.ambient.ptr(), 0.10f);
		ImGui::DragFloat3("Diffuse", light_props.diffuse.ptr(), 0.10f);
		ImGui::DragFloat3("Specular", light_props.specular.ptr(), 0.10f);
		ImGui::DragFloat3("Constant", light_props.constant.ptr(), 0.10f);
		ImGui::DragFloat3("Linear", light_props.linear.ptr(), 0.10f);
		ImGui::DragFloat3("Quadratic", light_props.quadratic.ptr(), 0.10f);
		ImGui::DragFloat3("Cut Off", light_props.cut_off.ptr(), 0.10f);
		ImGui::DragFloat3("Outer Cut Off", light_props.outer_cut_off.ptr(), 0.10f);

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
	}
	else
		RightClickMenu("Light Spot");

	return true;
}

void ComponentLightSpot::OnDisable()
{

}

void ComponentLightSpot::Clone(Component* clone)
{
	clone->enabled = enabled;
	clone->not_destroy = not_destroy;
	ComponentLightSpot* light = (ComponentLightSpot*)clone;
	light->renderer_id = renderer_id;
	light->print_icon = print_icon;
}

void ComponentLightSpot::Reset()
{
	print_icon = true;
}

void ComponentLightSpot::SetComponent(Component* component)
{
	if (component->GetType() == type) {

		ComponentLightSpot* light = (ComponentLightSpot*)component;

		renderer_id = light->renderer_id;
		print_icon = light->print_icon;
	}
}

void ComponentLightSpot::SaveComponent(JSONArraypack* to_save)
{
	to_save->SetNumber("Type", (int)type);
	to_save->SetBoolean("Enabled", enabled);
	to_save->SetString("ID", std::to_string(ID));
	to_save->SetBoolean("PrintIcon", print_icon);

	to_save->SetFloat3("Position", float3(light_props.position));
	to_save->SetFloat3("Direction", float3(light_props.direction));
	to_save->SetFloat3("Ambient", float3(light_props.ambient));
	to_save->SetFloat3("Diffuse", float3(light_props.diffuse));
	to_save->SetFloat3("Specular", float3(light_props.specular));
	to_save->SetFloat3("Constant", float3(light_props.constant));
	to_save->SetFloat3("Linear", float3(light_props.linear));
	to_save->SetFloat3("Quadratic", float3(light_props.quadratic));
	to_save->SetFloat3("Cut_Off", float3(light_props.cut_off));
	to_save->SetFloat3("Outer_Cut_Off", float3(light_props.outer_cut_off));
}

void ComponentLightSpot::LoadComponent(JSONArraypack* to_load)
{
	enabled = to_load->GetBoolean("Enabled");
	ID = std::stoull(to_load->GetString("ID"));
	print_icon = to_load->GetBoolean("PrintIcon");

	light_props.position = to_load->GetFloat3("Position");
	light_props.direction = to_load->GetFloat3("Direction");
	light_props.ambient = to_load->GetFloat3("Ambient");
	light_props.diffuse = to_load->GetFloat3("Diffuse");
	light_props.specular = to_load->GetFloat3("Specular");
	light_props.constant = to_load->GetFloat3("Constant");
	light_props.linear = to_load->GetFloat3("Linear");
	light_props.quadratic = to_load->GetFloat3("Quadratic");
	light_props.cut_off = to_load->GetFloat3("Cut_Off");
	light_props.outer_cut_off = to_load->GetFloat3("Outer_Cut_Off");
}

void ComponentLightSpot::DrawIconLight()
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