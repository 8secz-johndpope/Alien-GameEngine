#pragma once

#include "ComponentCollider.h"
#include "MathGeoLib/include/Math/MathAll.h"
#include "Bullet/include/btBulletDynamicsCommon.h"

class GameObject;
class ModulePhysics;


class __declspec(dllexport) ComponentBoxCollider : public ComponentCollider
{
	friend class ModulePhysics;
	friend class GameObject;

public:

	ComponentBoxCollider(GameObject* go);

	void SetSize(const float3 size);
	float3 GetSize() { return size; }

private:

	void DrawSpecificInspector();
	void SaveComponent(JSONArraypack* to_save);
	void LoadComponent(JSONArraypack* to_load);
	void Clone(Component* clone);
	void Reset();



private:

	float3 size = float3::one();
};

