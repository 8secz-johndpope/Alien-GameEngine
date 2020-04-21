#pragma once

#include "Component.h"
#include "Color.h"
#include "ComponentMesh.h"
#include "ModuleObjects.h"

struct __declspec(dllexport) DirLightProperties
{
	float intensity = 1.0f;
	float3 position = float3::zero();
	float3 direction = float3::zero();
	float3 ambient = float3::zero();
	float3 diffuse = float3::one();
	float3 specular = float3::one();

	ComponentLightDirectional* light = nullptr;
	uint depthMap = 0;
	uint depthMapFBO;
	float4x4 viewMat;
	float4x4 projMat;
};

class __declspec(dllexport) ComponentLightDirectional : public Component {
	friend class GameObject;
	friend class ComponentMesh;
public:
	ComponentLightDirectional(GameObject* attach);
	virtual ~ComponentLightDirectional();

	void PostUpdate();
private:
	void LightLogic();

	bool DrawInspector();
	void OnDisable();

	void Clone(Component* clone);

	void Reset();
	void SetComponent(Component* component);

	void SaveComponent(JSONArraypack* to_save);
	void LoadComponent(JSONArraypack* to_load);

	void DrawIconLight();
	void DrawLightFrustrum();

private:
	ComponentMesh* bulb = nullptr;
	bool print_icon = true;

	float sizefrustrum = 10.f;
	float distance_far_plane = 100.f;
	uint renderer_id = 0;

	DirLightProperties light_props;
};
