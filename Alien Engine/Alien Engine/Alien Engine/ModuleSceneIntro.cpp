#include "Globals.h"
#include "Application.h"
#include "ModuleSceneIntro.h"

#ifdef NDEBUG //no debug
#pragma comment (lib, "MathGeoLib/libx86/Release/MathGeoLib.lib") 
#else
#pragma comment (lib, "MathGeoLib/libx86/Debug/MathGeoLib.lib") 
#endif

#include "RandomHelper.h"

ModuleSceneIntro::ModuleSceneIntro(Application* app, bool start_enabled) : Module(app, start_enabled)
{
}

ModuleSceneIntro::~ModuleSceneIntro()
{
}

// Load assets
bool ModuleSceneIntro::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;

	App->camera->Move(vec3(1.0f, 1.0f, 0.0f));
	App->camera->LookAt(vec3(0, 0, 0));


	return ret;
}

// Load assets
bool ModuleSceneIntro::CleanUp()
{
	LOG("Unloading Intro scene");

	
	return true;
}

// Update
update_status ModuleSceneIntro::Update(float dt)
{

	Sphere s({ 0,0,0 }, 4);
	Sphere f({ 0,0,0 }, 7);
	if (f.Intersects(s)) {
		LOG("INTERSECTION DETECTED!!!");
	}
	
	LOG("RAND %i",GetRandomIntBetweenTwo(-100, 100));
	
	return UPDATE_CONTINUE;
}


