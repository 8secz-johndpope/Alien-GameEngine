#pragma once

#include "Component.h"
#include "Color.h"
#include "ComponentMesh.h"
#include "ModuleObjects.h"

class __declspec(dllexport) ComponentLightPoint : public Component {
	friend class GameObject;
	friend class ComponentMesh;
public:
	ComponentLightPoint(GameObject* attach);
	virtual ~ComponentLightPoint();

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

private:
	ComponentMesh* bulb = nullptr;
	bool print_icon = true;

	uint renderer_id = 0;
};
