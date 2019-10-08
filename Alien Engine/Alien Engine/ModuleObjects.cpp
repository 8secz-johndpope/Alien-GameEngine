#include "Globals.h"
#include "ModuleObjects.h"
#include "Objects.h"
#include "Primitive.h"
#include "ModuleInput.h"
#include "glew/include/glew.h"
#include "Application.h"

#include "Cube.h"
#include "Sphere_Alien.h"
#include "Rock.h"
#include "Dodecahedron.h"
#include "Octahedron.h"
#include "Icosahedron.h"
#include "Torus_Alien.h"

ModuleObjects::ModuleObjects(bool start_enabled):Module(start_enabled)
{
	name.assign("ModuleObject");
}

ModuleObjects::~ModuleObjects()
{
}

bool ModuleObjects::Start()
{
	LOG("Starting Module Objects");
	bool ret = true;

	return ret;
}

update_status ModuleObjects::PreUpdate(float dt)
{
	std::vector<Object*>::iterator iter;
	for (iter = objects.begin(); iter != objects.end(); ++iter)
	{
		if ((*iter) != nullptr)
		{
			
		}
		else
			++iter;
	}

	return UPDATE_CONTINUE;
}

update_status ModuleObjects::Update(float dt)
{
	std::vector<Object*>::iterator iter;
	for (iter = objects.begin(); iter != objects.end(); ++iter)
	{
		if ((*iter) != nullptr)
		{

		}
		else
			++iter;
	}
	return UPDATE_CONTINUE;
}

update_status ModuleObjects::PostUpdate(float dt)
{
	std::vector<Object*>::iterator item;
	for (item = objects.begin(); item != objects.end(); ++item)
	{
		if ((*item) != nullptr)
		{
			if ((*item)->num_index == 0 || (*item)->num_vertex == 0 || !(*item)->IsEnabled())
				continue;

			if (!wireframe_mode) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
				glEnable(GL_POLYGON_OFFSET_FILL);
				glPolygonOffset(1.0f, 0.4f);

				glColor3f(0.75f, 0.75f, 0.75f);

				(*item)->Draw();
				glDisable(GL_POLYGON_OFFSET_FILL);
			}
			if (wireframe_mode || view_mesh_mode) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
				// draw model lines
				glColor3f(mesh_color.r, mesh_color.g, mesh_color.b);

				(*item)->Draw();
			}
		}

	}

	return UPDATE_CONTINUE;
}

bool ModuleObjects::CleanUp()
{
	std::vector<Object*>::iterator iter;
	for (iter=objects.begin(); iter!=objects.end(); ++iter)
	{
		if ((*iter) != nullptr)
		{
			delete (*iter);
			(*iter) = nullptr;
		}
	}

	objects.clear();

	return true;
}

Primitive* ModuleObjects::CreatePrimitive(const PrimitiveType& type)
{
	Primitive* ret = nullptr;
	switch (type)
	{
	case PrimitiveType::CUBE:
		ret = new Cube();
		ret->type = PrimitiveType::CUBE;
		break;
	case PrimitiveType::SPHERE_ALIEN:
		ret = new Sphere_Alien();
		ret->type = PrimitiveType::SPHERE_ALIEN;
		break;
	case PrimitiveType::ROCK:
		ret = new Rock();
		ret->type = PrimitiveType::ROCK;
		break;
	case PrimitiveType::DODECAHEDRON:
		ret = new Dodecahedron();
		ret->type = PrimitiveType::DODECAHEDRON;
		break;
	case PrimitiveType::OCTAHEDRON:
		ret = new Octahedron();
		ret->type = PrimitiveType::OCTAHEDRON;
		break;
	case PrimitiveType::ICOSAHEDRON:
		ret = new Icosahedron();
		ret->type = PrimitiveType::ICOSAHEDRON;
		break;
	case PrimitiveType::TORUS:
		ret = new Torus_Alien();
		ret->type = PrimitiveType::TORUS;
		break;
	}

	if (ret != nullptr)
	{
		static_cast<Object*>(ret)->type = ObjectType::PRIMITIVE;
		objects.push_back(ret);
	}

	return ret;
}

Primitive* ModuleObjects::CreatePrimitive(const PrimitiveType& type, const float& position_x, const float& position_y, const float& position_z, const uint & subdivisions = 5, const uint & seed = 0, const uint & slices = 5, const uint & slacks = 5, const float & radius = 0.5)
{
	Primitive* ret = nullptr;
	switch (type)
	{
	case PrimitiveType::CUBE:
		ret = new Cube(position_x, position_y, position_z);
		ret->type = PrimitiveType::CUBE;
		break;
	case PrimitiveType::SPHERE_ALIEN:
		ret = new Sphere_Alien(position_x, position_y, position_z, subdivisions);
		ret->type = PrimitiveType::SPHERE_ALIEN;
		break;
	case PrimitiveType::ROCK:
		ret = new Rock(position_x, position_y, position_z, seed, subdivisions);
		ret->type = PrimitiveType::ROCK;
		break;
	case PrimitiveType::DODECAHEDRON:
		ret = new Dodecahedron(position_x, position_y, position_z);
		ret->type = PrimitiveType::DODECAHEDRON;
		break;
	case PrimitiveType::OCTAHEDRON:
		ret = new Octahedron(position_x, position_y, position_z);
		ret->type = PrimitiveType::OCTAHEDRON;
		break;
	case PrimitiveType::ICOSAHEDRON:
		ret = new Icosahedron(position_x, position_y, position_z);
		ret->type = PrimitiveType::ICOSAHEDRON;
		break;
	case PrimitiveType::TORUS:
		ret = new Torus_Alien(position_x, position_y, position_z, slices, slacks, radius);
		ret->type = PrimitiveType::TORUS;
		break;
	}
	if (ret != nullptr)
	{
		static_cast<Object*>(ret)->type = ObjectType::PRIMITIVE;
		objects.push_back(ret);
	}

	return ret;
}

void ModuleObjects::ChangeWireframeMode()
{
	view_mesh_mode = true;
	wireframe_mode = !wireframe_mode;
}

void ModuleObjects::ChangeViewMeshMode()
{
	view_mesh_mode = !view_mesh_mode;
}

void ModuleObjects::DeleteAllObjects() 
{
	std::vector<Object*>::iterator iter;
	for (iter = objects.begin(); iter != objects.end(); ++iter)
	{
		if ((*iter) != nullptr)
		{
			delete (*iter);
			(*iter) = nullptr;
		}
	}

	objects.clear();
}

void ModuleObjects::LoadConfig(JSONfilepack*& config) 
{
	wireframe_mode = config->GetBoolean("Configuration.Renderer.Wireframe");
	view_mesh_mode = config->GetBoolean("Configuration.Renderer.MeshView");
	draw_vertex_normals = config->GetBoolean("Configuration.Renderer.VertexNormals");
	draw_face_normals = config->GetBoolean("Configuration.Renderer.FaceNormals");
}

void ModuleObjects::SaveConfig(JSONfilepack*& config)
{
	config->SetBoolean("Configuration.Renderer.Wireframe", wireframe_mode);
	config->SetBoolean("Configuration.Renderer.MeshView", view_mesh_mode);
	config->SetBoolean("Configuration.Renderer.VertexNormals", draw_vertex_normals);
	config->SetBoolean("Configuration.Renderer.FaceNormals", draw_face_normals);
}


