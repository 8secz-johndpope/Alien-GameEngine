#include "ComponentCurve.h"
#include "imgui/imgui.h"
#include "ReturnZ.h"
#include "Maths.h"
#include "glew/include/glew.h"

#include "Application.h"
#include "ModuleInput.h"

ComponentCurve::ComponentCurve(GameObject* attach) : Component(attach)
{
	type = ComponentType::CURVE;

	curve = Curve(float3{ -10,0,0 }, float3{ 10,0,0 });
}

ComponentCurve::~ComponentCurve()
{
}

bool ComponentCurve::DrawInspector()
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

	if (ImGui::CollapsingHeader("Curve", &not_destroy, ImGuiTreeNodeFlags_DefaultOpen))
	{
		RightClickMenu("Curve");


	}
	else {
		RightClickMenu("Curve");
	}

	return true;
}

void ComponentCurve::Clone(Component* clone)
{
}

void ComponentCurve::SaveComponent(JSONArraypack* to_save)
{
}

void ComponentCurve::LoadComponent(JSONArraypack* to_load)
{
}

void ComponentCurve::DrawScene()
{
	glPointSize(9);
	glBegin(GL_POINTS);
	for (uint i = 0; i < curve.GetControlPoints().size(); ++i) {
		glVertex3f(curve.GetControlPoints()[i].x, curve.GetControlPoints()[i].y, curve.GetControlPoints()[i].z);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (float f = 0.0F; f <= 1; f += 0.001) {
		auto res = curve.ValueAt(f);
		glVertex3f(res[0], res[1], res[2]);
	}
	glEnd();
}

Curve::Curve(const float3& begin, const float3& end)
{
	control_points.push_back(begin);
	control_points.push_back(begin + float3(5,10,0));
	control_points.push_back(end + float3(-5, 10, 0));
	control_points.push_back(end);

	for (float t = 0; t < 1; t += detail) {
		curve_points.push_back(bezier(t, control_points[0], control_points[1], control_points[2], control_points[3]));
	}
}

float3 Curve::ValueAt(float at)
{
	if (at < 0 || at > 1 || curve_points.empty() || control_points.empty()) {
		return float3::zero();
	}

	uint index = Maths::Map(at, 0, 1, 0, curve_points.size() - 1);
	return curve_points[index];
}

const std::vector<float3>& Curve::GetControlPoints()
{
	return control_points;
}

float3 Curve::bezier(float t, const float3& p0, const float3& p1, const float3& p2, const float3& p3)
{
	float3 res = float3::zero();
	res[0] = pow((1 - t), 3) * p0[0] + 3 * t * pow((1 - t), 2) * p1[0] + 3 * pow(t, 2) * (1 - t) * p2[0] + pow(t, 3) * p3[0];
	res[1] = pow((1 - t), 3) * p0[1] + 3 * t * pow((1 - t), 2) * p1[1] + 3 * pow(t, 2) * (1 - t) * p2[1] + pow(t, 3) * p3[1];
	res[2] = pow((1 - t), 3) * p0[2] + 3 * t * pow((1 - t), 2) * p1[2] + 3 * pow(t, 2) * (1 - t) * p2[2] + pow(t, 3) * p3[2];
	return res;
}