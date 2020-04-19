#include "Application.h"
#include "ComponentTransform.h"
#include "ComponentConvexHullCollider.h"
#include "Bullet/include/BulletCollision/CollisionShapes/btShapeHull.h"
#include "ComponentRigidBody.h"
#include "ComponentMesh.h"
#include "ResourceMesh.h"
#include "GameObject.h"
#include "mmgr/mmgr.h"

#include "ComponentPhysics.h"

ComponentConvexHullCollider::ComponentConvexHullCollider(GameObject* go) : ComponentCollider(go)
{
	name.assign("Convex Hull Collider");
	type = ComponentType::CONVEX_HULL_COLLIDER;
	prev_scale = transform->GetGlobalScale();

	shape = CreateConvexMesh(go);
	if(!shape)	// if convex mesh cook fail, create default cube
		shape = App->physx->CreateShape(PxBoxGeometry(.5f, .5f, .5f)); 

	App->SendAlienEvent(this, AlienEventType::COLLIDER_ADDED);
	
	InitCollider();

	// stores current gameobject scale
	prev_scale = float3(transform->GetGlobalScale());
}

PxShape* ComponentConvexHullCollider::CreateConvexMesh(const GameObject* go)
{
	// try to get mesh from gameobject
	const ComponentMesh* mesh = go->GetComponent<ComponentMesh>();
	if (!mesh)
		return nullptr;

	if (shape) // if we are re-creating actual shape
		BeginUpdateShape();

	uint nverts = mesh->mesh->num_vertex;
	uint ntris = mesh->mesh->num_faces;
	uint nindex = mesh->mesh->num_index;

	PxConvexMeshDesc convexDesc;
	convexDesc.flags = PxConvexFlag::eCOMPUTE_CONVEX;
	convexDesc.points.count = nverts;
	convexDesc.points.stride = sizeof(float) * 3;
	convexDesc.points.data = mesh->mesh->vertex;
	convexDesc.vertexLimit = (uint)vertex_limit;

	PxDefaultMemoryOutputStream buf;
	PxConvexMeshCookingResult::Enum result;

	if (!App->physx->px_cooking->cookConvexMesh(convexDesc, buf, &result))
		return nullptr;

	PxDefaultMemoryInputData input(buf.getData(), buf.getSize());
	PxConvexMesh* convexMesh = App->physx->px_physics->createConvexMesh(input);

	if (shape)	// if we are re-creating actual shape
	{
		shape->setGeometry(PxConvexMeshGeometry(convexMesh, PxMeshScale(F3_TO_PXVEC3(transform->GetGlobalScale())), PxConvexMeshGeometryFlag::eTIGHT_BOUNDS));
		EndUpdateShape();
		return shape;
	}
	else
		return App->physx->CreateShape(PxConvexMeshGeometry(convexMesh));
}


void ComponentConvexHullCollider::SaveComponent(JSONArraypack* to_save)
{
	ComponentCollider::SaveComponent(to_save);
	to_save->SetFloat3("Size", size);
	to_save->SetNumber("VertexLimit", vertex_limit);

}

void ComponentConvexHullCollider::LoadComponent(JSONArraypack* to_load)
{
	ComponentCollider::LoadComponent(to_load);
	vertex_limit = to_load->GetNumber("VertexLimit");
}

void ComponentConvexHullCollider::Update()
{
	/*float3 current_scale = transform->GetGlobalScale();

	float l1 = current_scale.LengthSq();
	float l2 = prev_scale.LengthSq();

	if(l1 != l2)
	{
		LOG_ENGINE("SCALE CHANGED");
		prev_scale = current_scale;
		CreateConvexMesh(game_object_attached);
	}*/
}


void ComponentConvexHullCollider::DrawScene()
{
	//shape->getGeometry().convexMesh().convexMesh.ge

	/*if (game_object_attached->IsSelected() && App->physics->debug_physics == false)
	{
		App->physics->DrawConvexCollider(this);
	}*/
}

void ComponentConvexHullCollider::DrawSpecificInspector()
{
	ImGui::Title("Vertex Limit", 1);
	if (ImGui::SliderInt("##vertexLimit", &vertex_limit, 4, 255))
	{
		Clamp(vertex_limit, 4, 255);
		CreateConvexMesh(game_object_attached);
	}
}
