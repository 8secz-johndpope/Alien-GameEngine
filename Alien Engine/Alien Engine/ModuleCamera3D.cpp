#include "Globals.h"
#include "Application.h"
#include "ModuleCamera3D.h"
#include "ModuleObjects.h"
#include "ComponentTransform.h"

ModuleCamera3D::ModuleCamera3D(bool start_enabled) : Module(start_enabled)
{
	name.assign("Camera3D");

	CalculateViewMatrix();

	X = vec3(1.0f, 0.0f, 0.0f);
	Y = vec3(0.0f, 1.0f, 0.0f);
	Z = vec3(0.0f, 0.0f, 1.0f);

	Position = vec3(0.0f, 0.0f, 5.0f);
	Reference = vec3(0.0f, 0.0f, 0.0f);
}

ModuleCamera3D::~ModuleCamera3D()
{}

// -----------------------------------------------------------------
bool ModuleCamera3D::Start()
{
	LOG("Setting up the camera");
	bool ret = true;

	return ret;
}

// -----------------------------------------------------------------
bool ModuleCamera3D::CleanUp()
{
	LOG("Cleaning camera");

	SDL_FreeCursor(cursor);

	return true;
}

// -----------------------------------------------------------------
update_status ModuleCamera3D::Update(float dt)
{
	newPos = { 0,0,0 };

	speed = camera_speed * dt;
	zoom_speed = camera_zoom_speed * dt;
	mouse_speed = camera_mouse_speed * dt;

	if (App->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) 
	{
		speed = camera_speed * 2 * dt;
		zoom_speed = camera_zoom_speed * 2 * dt;
	}
	if (is_scene_hovered) 
	{
		Zoom();
		Rotation();
	}
	if (is_scene_hovered || is_scene_focused) 
	{
		Movement();
	}



	SetCenterOffset();

	Position += newPos;
	Reference += newPos;

	if (App->input->GetKey(SDL_SCANCODE_F) == KEY_DOWN)
		Focus();

	// Recalculate matrix -------------
	CalculateViewMatrix();

	return UPDATE_CONTINUE;
}

// -----------------------------------------------------------------
void ModuleCamera3D::Look(const vec3 &Position, const vec3 &Reference, bool RotateAroundReference)
{
	this->Position = Position;
	this->Reference = Reference;

	Z = normalize(Position - Reference);
	X = normalize(cross(vec3(0.0f, 1.0f, 0.0f), Z));
	Y = cross(Z, X);

	if(!RotateAroundReference)
	{
		this->Reference = this->Position;
		this->Position += Z * 0.05f;
	}

	CalculateViewMatrix();
}

// -----------------------------------------------------------------
void ModuleCamera3D::LookAt( const vec3 &Spot)
{
	Reference = Spot;

	Z = normalize(Position - Reference);
	X = normalize(cross(vec3(0.0f, 1.0f, 0.0f), Z));
	Y = cross(Z, X);

	CalculateViewMatrix();
}


// -----------------------------------------------------------------
void ModuleCamera3D::Move(const vec3 &Movement)
{
	Position += Movement;
	Reference += Movement;

	CalculateViewMatrix();
}

void ModuleCamera3D::Movement()
{
	if (App->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_REPEAT)
	{
		if (App->input->GetKey(SDL_SCANCODE_Z) == KEY_REPEAT) newPos.y += speed;
		if (App->input->GetKey(SDL_SCANCODE_X) == KEY_REPEAT) newPos.y -= speed;

		if (App->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) newPos -= Z * speed;
		if (App->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) newPos += Z * speed;

		if (App->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) newPos -= X * speed;
		if (App->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) newPos += X * speed;
	}

	if (App->input->GetMouseButton(SDL_BUTTON_MIDDLE) == KEY_REPEAT && App->input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT)
	{
		cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
		SDL_SetCursor(cursor);

		if (App->input->GetMouseXMotion() > -1) newPos -= X * mouse_speed;
		if (App->input->GetMouseXMotion() < 1) newPos += X * mouse_speed;

		if (App->input->GetMouseYMotion() < 1) newPos -= Y * mouse_speed*0.5f ;
		if (App->input->GetMouseYMotion() > -1) newPos += Y * mouse_speed*0.5f;
	}
}

void ModuleCamera3D::Zoom()
{
	if (App->input->GetMouseZ() > 0)
	{
		newPos -= Z * zoom_speed;

	}
	else if (App->input->GetMouseZ() < 0)
	{
		newPos += Z * zoom_speed;
	}
}

void ModuleCamera3D::Rotation()
{
	if (App->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_REPEAT && App->input->GetKey(SDL_SCANCODE_LALT)==KEY_REPEAT)
	{
		int dx = -App->input->GetMouseXMotion();
		int dy = -App->input->GetMouseYMotion();

		float Sensitivity = 0.25f;

		Position -= Reference;

		cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
		SDL_SetCursor(cursor);

		if (dx != 0)
		{
			float DeltaX = (float)dx * Sensitivity;

			X = rotate(X, DeltaX, vec3(0.0f, 1.0f, 0.0f));
			Y = rotate(Y, DeltaX, vec3(0.0f, 1.0f, 0.0f));
			Z = rotate(Z, DeltaX, vec3(0.0f, 1.0f, 0.0f));
		}

		if (dy != 0)
		{
			float DeltaY = (float)dy * Sensitivity;

			Y = rotate(Y, DeltaY, X);
			Z = rotate(Z, DeltaY, X);

			if (Y.y < 0.0f)
			{
				Z = vec3(0.0f, Z.y > 0.0f ? 1.0f : -1.0f, 0.0f);
				Y = cross(Z, X);
			}
		}

		Position = Reference + Z * length(Position);
	}
}

void ModuleCamera3D::Focus()
{
	if (App->objects->GetSelectedObject() != nullptr)
	{
		ComponentTransform* tr = (ComponentTransform*)App->objects->GetSelectedObject()->GetComponent(ComponentType::TRANSFORM);
		LookAt({ tr->GetGlobalPosition().x, tr->GetGlobalPosition().y, tr->GetGlobalPosition().z });
		/*Scaling(App->objects->GetSelectedObject());*/
	}
	else
	{
		for (std::vector<GameObject*>::iterator iter = App->objects->base_game_object->children.begin(); iter != App->objects->base_game_object->children.end(); ++iter)
		{
			if (iter == App->objects->base_game_object->children.begin() || (*iter) != looking_at)
			{
				looking_at=(*iter);
				ComponentTransform* tr = (ComponentTransform*)(*iter)->GetComponent(ComponentType::TRANSFORM);
				LookAt({ tr->GetGlobalPosition().x, tr->GetGlobalPosition().y, tr->GetGlobalPosition().z });
				/*Scaling((*iter));*/
			}
		}
	}
}

// -----------------------------------------------------------------
float* ModuleCamera3D::GetViewMatrix()
{
	return &ViewMatrix;
}

void ModuleCamera3D::SetCenterOffset()
{
	if (last_width != center_offset_w)
	{
		// You skip the frist frame
		if (last_width != 0)
		{
			// Camera should look at the center of the object (or mantain its position)
			if ((last_width - center_offset_w) > 0)
				newPos -= X * mouse_speed * 0.4f;

			if ((last_width - center_offset_w) < 0)
				newPos += X * mouse_speed * 0.4f;
		}
	}

	last_width = center_offset_w;
}

// -----------------------------------------------------------------
void ModuleCamera3D::CalculateViewMatrix()
{
	ViewMatrix = mat4x4(X.x, Y.x, Z.x, 0.0f, X.y, Y.y, Z.y, 0.0f, X.z, Y.z, Z.z, 0.0f, -dot(X, Position), -dot(Y, Position), -dot(Z, Position), 1.0f);
	ViewMatrixInverse = inverse(ViewMatrix);
}

