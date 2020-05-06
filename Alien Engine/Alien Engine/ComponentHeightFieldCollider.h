#pragma once

#include "ComponentCollider.h"
#include "MathGeoLib/include/Math/MathAll.h"

class GameObject;
class ComponentMesh;
class btShapeHull;

class __declspec(dllexport)  ComponentHeightFieldCollider : public ComponentCollider
{
	friend class GameObject;

public:

	ComponentHeightFieldCollider(GameObject* go);

private:

	//void Update();

	void DrawSpecificInspector();

	void SaveComponent(JSONArraypack* to_save);
	void LoadComponent(JSONArraypack* to_load);

	void Clone(Component* component);

	void ScaleChanged();


	PxShape* CreateHeightField();

	static bool CompareX(float3 a, float3 b);
	static bool CompareZ(float3 a, float3 b);
	static bool CompareY(float3 a, float3 b);

private:

	

};
