#include "Component.h"
#include "Globals.h"
#include "ComponentCamera.h"
#include "MathGeoLib/include/MathGeoLib.h"
#include "MathGeoLib/include/MathBuildConfig.h"
#include "ModuleResources.h"
#include "ModuleUI.h"
#include "ModuleCamera3D.h"
#include "ComponentTransform.h"
#include "ModuleObjects.h"
#include "Gizmos.h"
#include "ModuleFileSystem.h"
#include "ResourceTexture.h"
#include "Application.h"
#include <gl/GL.h>
#include <gl/GLU.h>
#include "Maths.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "FileNode.h"
#include "ReturnZ.h"
#include "ModuleRenderer3D.h"
#include "ComponentMesh.h"
#include "Skybox.h"
#include "ResourceShader.h"
#include "mmgr/mmgr.h"
#include "Viewport.h"

#include "Optick/include/optick.h"

ComponentCamera::ComponentCamera(GameObject* attach): Component(attach)
{
	type = ComponentType::CAMERA;

	frustum.type = FrustumType::PerspectiveFrustum;

	frustum.pos = float3::zero();
	frustum.front = float3::unitZ();
	frustum.up = float3::unitY();

	frustum.nearPlaneDistance = near_plane;
	frustum.farPlaneDistance = far_plane;
	frustum.verticalFov = Maths::Deg2Rad() * vertical_fov;
	AspectRatio(16, 9);

	camera_color_background = Color(0.1f, 0.1f, 0.1f, 1.0f);

	if (attach != nullptr)
	{
		if (App->renderer3D->actual_game_camera == nullptr)
		{
			App->renderer3D->actual_game_camera = this;
			App->objects->game_viewport->SetCamera(this);
		}
		App->objects->game_cameras.push_back(this);
	}
	
#ifndef GAME_VERSION
	mesh_camera = new ComponentMesh(game_object_attached);
	mesh_camera->mesh = App->resources->camera_mesh;
#endif

	/* Create skybox */
	cubemap = new Cubemap();
	skybox = new Skybox();

#ifndef GAME_VERSION

	// This is the default skybox

	/*cubemap->pos_x.assign(LIBRARY_TEXTURES_FOLDER"6647353476053033927.dds");
	cubemap->neg_x.assign(LIBRARY_TEXTURES_FOLDER"16365362325777703218.dds");
	cubemap->pos_y.assign(LIBRARY_TEXTURES_FOLDER"13945118401434491814.dds");
	cubemap->neg_y.assign(LIBRARY_TEXTURES_FOLDER"13294645959885894553.dds");
	cubemap->neg_z.assign(LIBRARY_TEXTURES_FOLDER"882162789730207050.dds");
	cubemap->pos_z.assign(LIBRARY_TEXTURES_FOLDER"13059454142476507694.dds");*/

	/*cubemap->neg_x.assign(LIBRARY_TEXTURES_FOLDER"6647353476053033927.dds");
	cubemap->pos_x.assign(LIBRARY_TEXTURES_FOLDER"16365362325777703218.dds");
	cubemap->pos_y.assign(LIBRARY_TEXTURES_FOLDER"13945118401434491814.dds");
	cubemap->neg_y.assign(LIBRARY_TEXTURES_FOLDER"13294645959885894553.dds");
	cubemap->pos_z.assign(LIBRARY_TEXTURES_FOLDER"882162789730207050.dds");
	cubemap->neg_z.assign(LIBRARY_TEXTURES_FOLDER"13059454142476507694.dds");*/

	cubemap->neg_x.assign(LIBRARY_TEXTURES_FOLDER"6647353476053033927.dds");
	cubemap->pos_x.assign(LIBRARY_TEXTURES_FOLDER"16365362325777703218.dds");
	cubemap->pos_y.assign(LIBRARY_TEXTURES_FOLDER"13945118401434491814.dds");
	cubemap->neg_y.assign(LIBRARY_TEXTURES_FOLDER"13294645959885894553.dds");
	cubemap->pos_z.assign(LIBRARY_TEXTURES_FOLDER"882162789730207050.dds");
	cubemap->neg_z.assign(LIBRARY_TEXTURES_FOLDER"13059454142476507694.dds");

	auto faces = cubemap->ToVector();
	skybox_texture_id = skybox->LoadCubeMapFromLibraryFiles(faces);

#endif

	skybox->SetBuffers();

	skybox_shader = App->resources->skybox_shader;
	if (skybox_shader != nullptr)
		skybox_shader->IncreaseReferences();

	skybox_shader->Bind();
	skybox_shader->SetUniform1i("skybox", 0);
	skybox_shader->Unbind();
}

ComponentCamera::~ComponentCamera()
{
	std::vector<ComponentCamera*>::iterator item = App->objects->game_cameras.begin();
	for (; item != App->objects->game_cameras.end(); ++item) {
		if (*item != nullptr && *item == this) {
			App->objects->game_cameras.erase(item);
			if (App->renderer3D->actual_game_camera == this)
			{
				if (!App->objects->game_cameras.empty())
				{
					App->renderer3D->actual_game_camera = App->objects->game_cameras.front();
					App->objects->game_viewport->SetCamera(App->objects->game_cameras.front());
					#ifndef GAME_VERSION
						App->ui->actual_name = App->renderer3D->actual_game_camera->game_object_attached->GetName();
					#endif
				}
				else {
					App->renderer3D->actual_game_camera = nullptr;
					App->objects->game_viewport->SetCamera(nullptr);
				}
			}
			#ifndef GAME_VERSION
			if (App->renderer3D->selected_game_camera == this)
			{
				App->renderer3D->selected_game_camera = nullptr;
				App->camera->selected_viewport->SetCamera(nullptr);
			}
			#endif

			if (App->objects->game_viewport->GetCamera() == this) {
				if (!App->objects->game_cameras.empty()) {
					App->objects->game_viewport->SetCamera(App->objects->game_cameras.front());
				}
				else {
					App->objects->game_viewport->SetCamera(nullptr);
				}
			}
			break;
		}
	}
#ifndef GAME_VERSION
	delete mesh_camera;
#endif

	glDeleteTextures(1, &skybox_texture_id);

	RELEASE(skybox);
	RELEASE(cubemap);
}

bool ComponentCamera::DrawInspector()
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

	if (ImGui::CollapsingHeader("Camera", &not_destroy, ImGuiTreeNodeFlags_DefaultOpen))
	{
		RightClickMenu("Camera");
		static bool cntrl_z = true;
		ImGui::Spacing();
		static Color col;
		col = camera_color_background;
		if (ImGui::ColorEdit3("Background Color", &col, ImGuiColorEditFlags_Float)) {
			if (cntrl_z)
				ReturnZ::AddNewAction(ReturnZ::ReturnActions::CHANGE_COMPONENT, this);
			cntrl_z = false;
			camera_color_background = col;
		}
		else if (!cntrl_z && ImGui::IsMouseReleased(0)) {
			cntrl_z = true;
		}
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		static float sup;
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.25f);
		sup = near_plane;
		if (ImGui::DragFloat("Near Plane", &sup, 1, 0.1f, far_plane - 0.1f, "%.1f"))
		{
			if (cntrl_z)
				ReturnZ::AddNewAction(ReturnZ::ReturnActions::CHANGE_COMPONENT, this);
			cntrl_z = false;
			near_plane = sup;
			frustum.nearPlaneDistance = near_plane;
			App->renderer3D->UpdateCameraMatrix(this);
		}
		else if (!cntrl_z && ImGui::IsMouseReleased(0)) {
			cntrl_z = true;
		}
		ImGui::SameLine();

		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.25f);
		sup = far_plane;
		if (ImGui::DragFloat("Far Plane", &sup, 1, near_plane + 0.1f, 1000, "%.1f"))
		{
			if (cntrl_z)
				ReturnZ::AddNewAction(ReturnZ::ReturnActions::CHANGE_COMPONENT, this);
			cntrl_z = false;
			far_plane = sup;
			frustum.farPlaneDistance = far_plane;
			App->renderer3D->UpdateCameraMatrix(this);
		}
		else if (!cntrl_z && ImGui::IsMouseReleased(0)) {
			cntrl_z = true;
		}
		ImGui::Spacing();
		ImGui::Spacing();
		
		if (is_fov_horizontal!=0)
		{
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.4f);
			sup = horizontal_fov;
			if (ImGui::DragFloat("FOV ", &sup, 1, 1, 163, "%.1f"))
			{
				if (cntrl_z)
					ReturnZ::AddNewAction(ReturnZ::ReturnActions::CHANGE_COMPONENT, this);
				cntrl_z = false;
				horizontal_fov = sup;
				frustum.horizontalFov = horizontal_fov * Maths::Deg2Rad();
				AspectRatio(16, 9, true);
				//vertical_fov = frustum.verticalFov * Maths::Rad2Deg();
				App->renderer3D->UpdateCameraMatrix(this);
			}
			else if (!cntrl_z && ImGui::IsMouseReleased(0)) {
				cntrl_z = true;
			}
		}
		else
		{
			ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.4f);
			sup = vertical_fov;
			if (ImGui::DragFloat("FOV", &sup, 1, 1, 150, "%.1f"))
			{
				if (cntrl_z)
					ReturnZ::AddNewAction(ReturnZ::ReturnActions::CHANGE_COMPONENT, this);
				cntrl_z = false;
				vertical_fov = sup;
				frustum.verticalFov = vertical_fov * Maths::Deg2Rad();
				AspectRatio(16, 9);
				//horizontal_fov = frustum.horizontalFov * Maths::Rad2Deg();
				App->renderer3D->UpdateCameraMatrix(this);
			}
			else if (!cntrl_z && ImGui::IsMouseReleased(0)) {
				cntrl_z = true;
			}
		}

		ImGui::SameLine();


		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.4f);
		ImGui::Combo("## cool fov combp", &is_fov_horizontal, "Vertical\0Horizontal\0");

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		
		ImGui::Checkbox("Active Fog", &activeFog);
		if (activeFog)
		{
			ImGui::DragFloat("Density", &fogDensity, 0.001f, 0.0f, 10.f);
			ImGui::DragFloat("Gradient", &fogGradient, 0.02f, 0.0f, 10.f);
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::PushID("vcnsdbiobsdifnidsofnionew");
		ImGui::Checkbox("Print Icon", &print_icon);
		ImGui::PopID();
		ImGui::SameLine();
		ImGui::Text("|");
		ImGui::SameLine();
		ImGui::PushID("fdgdfdgdgserwfew");
		ImGui::ColorEdit4("Icon Color", &camera_icon_color, ImGuiColorEditFlags_Float);
		ImGui::PopID();
		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Skybox settings:");
		
		std::string path_pos_x = App->file_system->GetBaseFileName(cubemap->pos_x.c_str());
		ResourceTexture* tex_pos_x = (ResourceTexture*)App->resources->GetResourceWithID(std::stoull(path_pos_x));
		ImGui::Image((ImTextureID)tex_pos_x->id, ImVec2(100.0f, 100.0f));
		
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::GetDragDropPayload();
			if (payload != nullptr && payload->IsDataType(DROP_ID_PROJECT_NODE))
			{
				FileNode* node = *(FileNode * *)payload->Data;
				if (node->type == FileDropType::TEXTURE && ImGui::AcceptDragDropPayload(DROP_ID_PROJECT_NODE, ImGuiDragDropFlags_SourceNoDisableHover))
				{
					// drop texture
					if (node != nullptr && node->type == FileDropType::TEXTURE)
					{
						std::string path = App->file_system->GetPathWithoutExtension(node->path + node->name);
						path += "_meta.alien";

						u64 ID = App->resources->GetIDFromAlienPath(path.data());
						ResourceTexture* texture_dropped = (ResourceTexture*)App->resources->GetResourceWithID(ID);

						if (texture_dropped != nullptr)
						{
							if (texture_dropped->references == 0)
								texture_dropped->IncreaseReferences();

							cubemap->pos_x.assign(texture_dropped->GetLibraryPath());
							skybox->ChangePositiveX(skybox_texture_id, texture_dropped->id, texture_dropped->width, texture_dropped->height);
						}
					}

					ImGui::ClearDragDrop();
				}
			}

			ImGui::EndDragDropTarget();
		}
		ImGui::SameLine();
		ImGui::Text("Positive X: "); ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), cubemap->pos_x.c_str());

		std::string path_neg_x = App->file_system->GetBaseFileName(cubemap->neg_x.c_str());
		ResourceTexture* tex_neg_x = (ResourceTexture*)App->resources->GetResourceWithID(std::stoull(path_neg_x));
		ImGui::Image((ImTextureID)tex_neg_x->id, ImVec2(100.0f, 100.0f));
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::GetDragDropPayload();
			if (payload != nullptr && payload->IsDataType(DROP_ID_PROJECT_NODE))
			{
				FileNode* node = *(FileNode * *)payload->Data;
				if (node->type == FileDropType::TEXTURE && ImGui::AcceptDragDropPayload(DROP_ID_PROJECT_NODE, ImGuiDragDropFlags_SourceNoDisableHover))
				{
					// drop texture
					if (node != nullptr && node->type == FileDropType::TEXTURE)
					{
						std::string path = App->file_system->GetPathWithoutExtension(node->path + node->name);
						path += "_meta.alien";

						u64 ID = App->resources->GetIDFromAlienPath(path.data());
						ResourceTexture* texture_dropped = (ResourceTexture*)App->resources->GetResourceWithID(ID);

						if (texture_dropped != nullptr)
						{
							if (texture_dropped->references == 0)
								texture_dropped->IncreaseReferences();

							cubemap->neg_x.assign(texture_dropped->GetLibraryPath());
							skybox->ChangeNegativeX(skybox_texture_id, texture_dropped->id, texture_dropped->width, texture_dropped->height);
						}
					}

					ImGui::ClearDragDrop();
				}
			}

			ImGui::EndDragDropTarget();
		}
		ImGui::SameLine();
		ImGui::Text("Negative X: "); ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), cubemap->neg_x.c_str());


		std::string path_pos_y = App->file_system->GetBaseFileName(cubemap->pos_y.c_str());
		ResourceTexture* tex_pos_y = (ResourceTexture*)App->resources->GetResourceWithID(std::stoull(path_pos_y));
		ImGui::Image((ImTextureID)tex_pos_y->id, ImVec2(100.0f, 100.0f));
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::GetDragDropPayload();
			if (payload != nullptr && payload->IsDataType(DROP_ID_PROJECT_NODE))
			{
				FileNode* node = *(FileNode * *)payload->Data;

				if (node->type == FileDropType::TEXTURE && ImGui::AcceptDragDropPayload(DROP_ID_PROJECT_NODE, ImGuiDragDropFlags_SourceNoDisableHover))
				{
					// drop texture
					if (node != nullptr && node->type == FileDropType::TEXTURE)
					{
						std::string path = App->file_system->GetPathWithoutExtension(node->path + node->name);
						path += "_meta.alien";

						u64 ID = App->resources->GetIDFromAlienPath(path.data());
						ResourceTexture* texture_dropped = (ResourceTexture*)App->resources->GetResourceWithID(ID);

						if (texture_dropped != nullptr)
						{
							if (texture_dropped->references == 0)
								texture_dropped->IncreaseReferences();

							cubemap->pos_y.assign(texture_dropped->GetLibraryPath());
							skybox->ChangePositiveY(skybox_texture_id, texture_dropped->id, texture_dropped->width, texture_dropped->height);
						}
					}

					ImGui::ClearDragDrop();
				}
			}

			ImGui::EndDragDropTarget();
		}
		ImGui::SameLine();
		ImGui::Text("Positive Y: "); ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), cubemap->pos_y.c_str());
		
		std::string path_neg_y = App->file_system->GetBaseFileName(cubemap->neg_y.c_str());
		ResourceTexture* tex_neg_y = (ResourceTexture*)App->resources->GetResourceWithID(std::stoull(path_neg_y));
		ImGui::Image((ImTextureID)tex_neg_y->id, ImVec2(100.0f, 100.0f));
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::GetDragDropPayload();
			if (payload != nullptr && payload->IsDataType(DROP_ID_PROJECT_NODE))
			{
				FileNode* node = *(FileNode * *)payload->Data;

				if (node->type == FileDropType::TEXTURE && ImGui::AcceptDragDropPayload(DROP_ID_PROJECT_NODE, ImGuiDragDropFlags_SourceNoDisableHover))
				{
				// drop texture
					if (node != nullptr && node->type == FileDropType::TEXTURE)
					{
						std::string path = App->file_system->GetPathWithoutExtension(node->path + node->name);
						path += "_meta.alien";

						u64 ID = App->resources->GetIDFromAlienPath(path.data());
						ResourceTexture* texture_dropped = (ResourceTexture*)App->resources->GetResourceWithID(ID);

						if (texture_dropped != nullptr)
						{
							if (texture_dropped->references == 0)
								texture_dropped->IncreaseReferences();

							cubemap->neg_y.assign(texture_dropped->GetLibraryPath());
							skybox->ChangeNegativeY(skybox_texture_id, texture_dropped->id, texture_dropped->width, texture_dropped->height);
						}
					}

					ImGui::ClearDragDrop();
				}
			}

			ImGui::EndDragDropTarget();
		}
		ImGui::SameLine();
		ImGui::Text("Negative Y: "); ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), cubemap->neg_y.c_str());
		
		std::string path_pos_z = App->file_system->GetBaseFileName(cubemap->pos_z.c_str());
		ResourceTexture* tex_pos_z = (ResourceTexture*)App->resources->GetResourceWithID(std::stoull(path_pos_z));
		ImGui::Image((ImTextureID)tex_pos_z->id, ImVec2(100.0f, 100.0f));
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::GetDragDropPayload();
			if (payload != nullptr && payload->IsDataType(DROP_ID_PROJECT_NODE))
			{
				FileNode* node = *(FileNode * *)payload->Data;

				if (node->type == FileDropType::TEXTURE && ImGui::AcceptDragDropPayload(DROP_ID_PROJECT_NODE, ImGuiDragDropFlags_SourceNoDisableHover))
				{
					// drop texture
					if (node != nullptr && node->type == FileDropType::TEXTURE)
					{
						std::string path = App->file_system->GetPathWithoutExtension(node->path + node->name);
						path += "_meta.alien";

						u64 ID = App->resources->GetIDFromAlienPath(path.data());
						ResourceTexture* texture_dropped = (ResourceTexture*)App->resources->GetResourceWithID(ID);

						if (texture_dropped != nullptr)
						{
							if (texture_dropped->references == 0)
								texture_dropped->IncreaseReferences();

							cubemap->pos_z.assign(texture_dropped->GetLibraryPath());
							skybox->ChangePositiveZ(skybox_texture_id, texture_dropped->id, texture_dropped->width, texture_dropped->height);
						}
					}

					ImGui::ClearDragDrop();
				}
			}

			ImGui::EndDragDropTarget();
		}
		ImGui::SameLine();
		ImGui::Text("Positive Z: "); ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), cubemap->pos_z.c_str());


		std::string path_neg_z = App->file_system->GetBaseFileName(cubemap->neg_z.c_str());
		ResourceTexture* tex_neg_z = (ResourceTexture*)App->resources->GetResourceWithID(std::stoull(path_neg_z));
		ImGui::Image((ImTextureID)tex_neg_z->id, ImVec2(100.0f, 100.0f));
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* payload = ImGui::GetDragDropPayload();
			if (payload != nullptr && payload->IsDataType(DROP_ID_PROJECT_NODE))
			{
				FileNode* node = *(FileNode * *)payload->Data;

				if (node->type == FileDropType::TEXTURE && ImGui::AcceptDragDropPayload(DROP_ID_PROJECT_NODE, ImGuiDragDropFlags_SourceNoDisableHover))
				{
				// drop texture
					if (node != nullptr && node->type == FileDropType::TEXTURE)
					{
						std::string path = App->file_system->GetPathWithoutExtension(node->path + node->name);
						path += "_meta.alien";

						u64 ID = App->resources->GetIDFromAlienPath(path.data());
						ResourceTexture* texture_dropped = (ResourceTexture*)App->resources->GetResourceWithID(ID);

						if (texture_dropped != nullptr)
						{
							if (texture_dropped->references == 0)
								texture_dropped->IncreaseReferences();

							cubemap->neg_z.assign(texture_dropped->GetLibraryPath());
							skybox->ChangeNegativeZ(skybox_texture_id, texture_dropped->id, texture_dropped->width, texture_dropped->height);
						}
					}

					ImGui::ClearDragDrop();
				}
			}

			ImGui::EndDragDropTarget();
		}
		ImGui::SameLine();
		ImGui::Text("Negative Z: "); ImGui::SameLine(); ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), cubemap->neg_z.c_str());

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();
	}

	else
		RightClickMenu("Camera");

	
	return true;
}

void ComponentCamera::Reset()
{
	camera_color_background = { 0.05f, 0.05f, 0.05f, 1.0f };

	near_plane = 0.1f;
	far_plane = 200.f;
	frustum.nearPlaneDistance = near_plane;
	frustum.farPlaneDistance = far_plane;

	// This is the default skybox/cubemap
	cubemap->neg_z.assign(TEXTURES_FOLDER"Skybox/sky_2/negz.png");
	cubemap->pos_z.assign(TEXTURES_FOLDER"Skybox/sky_2/posz.png");
	cubemap->pos_y.assign(TEXTURES_FOLDER"Skybox/sky_2/posy.png");
	cubemap->neg_y.assign(TEXTURES_FOLDER"Skybox/sky_2/negy.png");
	cubemap->pos_x.assign(TEXTURES_FOLDER"Skybox/sky_2/posx.png");
	cubemap->neg_x.assign(TEXTURES_FOLDER"Skybox/sky_2/negx.png");

	vertical_fov = 60.0f;
	frustum.verticalFov = Maths::Deg2Rad() * vertical_fov;
	AspectRatio(16, 9);
	horizontal_fov = frustum.horizontalFov * Maths::Rad2Deg();
	print_icon = true;
	activeFog = false;
}

void ComponentCamera::SetComponent(Component* component)
{
	if (component->GetType() == type) {

		ComponentCamera* camera = (ComponentCamera*)component;
		
		camera_color_background = camera->camera_color_background;
		near_plane = camera->near_plane;
		far_plane = camera->far_plane;
		frustum = camera->frustum;
		vertical_fov = camera->vertical_fov;
		horizontal_fov = camera->horizontal_fov;
		print_icon = camera->print_icon;
		is_fov_horizontal = camera->is_fov_horizontal;
		camera_icon_color = camera->camera_icon_color;
	}
}

void ComponentCamera::AspectRatio(int width_ratio, int height_ratio, bool fov_type)
{
	if (fov_type == 0)
	{
		frustum.horizontalFov = (2.f * atanf(tanf(frustum.verticalFov * 0.5f) * ((float)width_ratio / (float)height_ratio)));
	}
	else
	{
		frustum.verticalFov = (2.f * atanf(tanf(frustum.horizontalFov * 0.5f) * ((float)height_ratio) / (float)width_ratio));
	}
	vertical_fov = frustum.verticalFov * Maths::Rad2Deg();
	horizontal_fov = frustum.horizontalFov * Maths::Rad2Deg();
}

void ComponentCamera::Look(const float3& position_to_look)
{
	if (position_to_look.IsFinite()) {
		float3 direction = position_to_look - frustum.pos;

		float3x3 matrix = float3x3::LookAt(frustum.front, direction.Normalized(), frustum.up, float3::unitY());

		frustum.front = matrix.MulDir(frustum.front).Normalized();
		frustum.up = matrix.MulDir(frustum.up).Normalized();
	}
}

float* ComponentCamera::GetProjectionMatrix() const
{
	return (float*)frustum.ProjectionMatrix().Transposed().v;
}

float4x4 ComponentCamera::GetProjectionMatrix4f4() const
{
	return frustum.ProjectionMatrix().Transposed();
}

float* ComponentCamera::GetViewMatrix() const
{
	return (float*)static_cast<float4x4>(frustum.ViewMatrix()).Transposed().v;
}

float4x4 ComponentCamera::GetViewMatrix4x4() const
{
	return float4x4(frustum.ViewMatrix()).Transposed();
}

void ComponentCamera::SetVerticalFov(const float& vertical_fov)
{
	this->vertical_fov = vertical_fov;
	frustum.verticalFov = Maths::Deg2Rad() * vertical_fov;
	AspectRatio(16, 9);
}

float ComponentCamera::GetVerticalFov() const
{
	return vertical_fov;
}

void ComponentCamera::SetHorizontalFov(const float& horizontal_fov)
{
	this->horizontal_fov = horizontal_fov;
	frustum.horizontalFov = Maths::Deg2Rad() * horizontal_fov;
	AspectRatio(16, 9, true);
}

float ComponentCamera::GetHorizontalFov() const
{
	return horizontal_fov;
}

void ComponentCamera::SetFarPlane(const float& far_plane)
{
	this->far_plane = far_plane;
	frustum.farPlaneDistance = far_plane;
}

void ComponentCamera::SetNearPlane(const float& near_plane)
{
	this->near_plane = near_plane;
	frustum.nearPlaneDistance = near_plane;
}

float ComponentCamera::GetFarPlane() const
{
	return far_plane;
}

float ComponentCamera::GetNearPlane() const
{
	return near_plane;
}

void ComponentCamera::SetCameraPosition(const float3& position)
{
	frustum.pos = position;
}

float3 ComponentCamera::GetCameraPosition() const
{
	return frustum.pos;
}

void ComponentCamera::EnableFog()
{
	activeFog = true;
}

void ComponentCamera::DisableFog()
{
	activeFog = false;
}

void ComponentCamera::SetFogDensity(const float& density)
{
	fogDensity = density;
}

void ComponentCamera::SetFogGradient(const float& gradient)
{
	fogGradient = gradient;
}

float ComponentCamera::GetFogDensity() const
{
	return fogDensity;
}

float ComponentCamera::GetFogGradient() const
{
	return fogGradient;
}

void ComponentCamera::SetBackgroundColor(const float3& color)
{
	camera_color_background = { color.x, color.y, color.z };
}

float3 ComponentCamera::GetBackgroundColor() const
{
	return float3(camera_color_background.r, camera_color_background.g, camera_color_background.b);
}

void ComponentCamera::DrawSkybox()
{
	OPTICK_EVENT();
	if (App->renderer3D->render_skybox && !activeFog)
	{
		glDepthFunc(GL_LEQUAL);
		skybox_shader->Bind();

		float4x4 view_m = this->GetViewMatrix4x4();
		// Theoretically this should remove the translation [x, y, z] of the matrix,
		// but because it is relative to the camera it has no effect.
		view_m[0][3] = 0;
		view_m[1][3] = 0;
		view_m[2][3] = 0;
		skybox_shader->SetUniformMat4f("view", view_m);
		float4x4 projection = this->GetProjectionMatrix4f4();
		skybox_shader->SetUniformMat4f("projection", projection);
	
		glBindVertexArray(skybox->vao);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture_id);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
		glDepthFunc(GL_LESS);
		glBindVertexArray(0);
		skybox_shader->Unbind();
	}
}

void ComponentCamera::DrawFrustum()
{
	static float3 points[8];
	frustum.GetCornerPoints(points);

	glLineWidth(App->objects->frustum_line_width);
	glColor3f(App->objects->frustum_color.r, App->objects->frustum_color.g, App->objects->frustum_color.b);
	glBegin(GL_LINES);

	glVertex3f(points[0].x, points[0].y, points[0].z);
	glVertex3f(points[1].x, points[1].y, points[1].z);

	glVertex3f(points[0].x, points[0].y, points[0].z);
	glVertex3f(points[4].x, points[4].y, points[4].z);

	glVertex3f(points[4].x, points[4].y, points[4].z);
	glVertex3f(points[5].x, points[5].y, points[5].z);

	glVertex3f(points[0].x, points[0].y, points[0].z);
	glVertex3f(points[2].x, points[2].y, points[2].z);

	glVertex3f(points[2].x, points[2].y, points[2].z);
	glVertex3f(points[3].x, points[3].y, points[3].z);

	glVertex3f(points[1].x, points[1].y, points[1].z);
	glVertex3f(points[3].x, points[3].y, points[3].z);

	glVertex3f(points[1].x, points[1].y, points[1].z);
	glVertex3f(points[5].x, points[5].y, points[5].z);

	glVertex3f(points[4].x, points[4].y, points[4].z);
	glVertex3f(points[6].x, points[6].y, points[6].z);

	glVertex3f(points[2].x, points[2].y, points[2].z);
	glVertex3f(points[6].x, points[6].y, points[6].z);

	glVertex3f(points[6].x, points[6].y, points[6].z);
	glVertex3f(points[7].x, points[7].y, points[7].z);

	glVertex3f(points[5].x, points[5].y, points[5].z);
	glVertex3f(points[7].x, points[7].y, points[7].z);
	
	glVertex3f(points[3].x, points[3].y, points[3].z);
	glVertex3f(points[7].x, points[7].y, points[7].z);

	glEnd();
	glLineWidth(1);
}

void ComponentCamera::DrawIconCamera()
{
	if (mesh_camera != nullptr && print_icon)
	{
		ComponentTransform* transform = game_object_attached->transform;
		float3 position = transform->GetGlobalPosition() - frustum.front.Normalized() * 2;
		Quat rotated = transform->GetGlobalRotation() * (Quat{ 0,0,1,0 } * Quat{ 0.7071,0,0.7071,0 });
		float4x4 matrix = float4x4::FromTRS(position, rotated, { 0.1F,0.1F,0.1F });
		glDisable(GL_LIGHTING);
		Gizmos::DrawPoly(mesh_camera->mesh, matrix, camera_icon_color);
		glEnable(GL_LIGHTING);
	}
}

void ComponentCamera::Clone(Component* clone)
{
	clone->enabled = enabled;
	clone->not_destroy = not_destroy;
	ComponentCamera* camera = (ComponentCamera*)clone;
	camera->camera_color_background = camera_color_background;
	camera->camera_icon_color = camera_icon_color;
	camera->enabled = enabled;
	camera->far_plane = far_plane;
	camera->frustum = frustum;
	camera->horizontal_fov = horizontal_fov;
	camera->is_fov_horizontal = is_fov_horizontal;
	camera->near_plane = near_plane;
	camera->print_icon = print_icon;
	camera->projection_changed = projection_changed;
	camera->vertical_fov = vertical_fov;
	camera->ViewMatrix = ViewMatrix;
	camera->ViewMatrixInverse = ViewMatrixInverse;
	camera->cubemap = cubemap;
	camera->activeFog = activeFog;
	camera->fogDensity = fogDensity;
	camera->fogGradient = fogGradient;
}

void ComponentCamera::SaveComponent(JSONArraypack* to_save)
{
	OPTICK_EVENT();
	to_save->SetBoolean("Enabled", enabled);
	to_save->SetNumber("Type", (int)type);
	to_save->SetNumber("VerticalFov", vertical_fov);
	to_save->SetNumber("HoritzontalFov", horizontal_fov);
	to_save->SetColor("BackCol", camera_color_background);
	to_save->SetNumber("FarPlane", far_plane);
	to_save->SetNumber("NearPlane", near_plane);
	to_save->SetNumber("isFovHori", is_fov_horizontal);
	to_save->SetString("ID", std::to_string(ID).data());
	to_save->SetBoolean("IsGameCamera", (App->renderer3D->actual_game_camera == this) ? true : false);
	to_save->SetBoolean("IsSelectedCamera", (game_object_attached->IsSelected()) ? true : false);
	to_save->SetBoolean("PrintIcon", print_icon);
	to_save->SetColor("IconColor", camera_icon_color);
	to_save->SetBoolean("Fog", activeFog);
	to_save->SetNumber("Density", fogDensity);
	to_save->SetNumber("Gradient", fogGradient);

	/* Save skybox (Library File) */
	std::string path1 = cubemap->pos_x;
	to_save->SetString("Skybox_PositiveX", path1.c_str());

	std::string path2 = cubemap->neg_x;
	to_save->SetString("Skybox_NegativeX", path2.c_str());

	std::string path3 = cubemap->pos_y;
	to_save->SetString("Skybox_PositiveY", path3.c_str());

	std::string path4 = cubemap->neg_y;
	to_save->SetString("Skybox_NegativeY", path4.c_str());

	std::string path5 = cubemap->pos_z;
	to_save->SetString("Skybox_PositiveZ", path5.c_str());

	std::string path6 = cubemap->neg_z;
	to_save->SetString("Skybox_NegativeZ", path6.c_str());
}

void ComponentCamera::LoadComponent(JSONArraypack* to_load)
{
	OPTICK_EVENT();
	enabled = to_load->GetBoolean("Enabled");
	vertical_fov = to_load->GetNumber("VerticalFov");
	horizontal_fov = to_load->GetNumber("HoritzontalFov");
	far_plane = to_load->GetNumber("FarPlane");
	near_plane = to_load->GetNumber("NearPlane");
	is_fov_horizontal = to_load->GetNumber("isFovHori");
	camera_color_background = to_load->GetColor("BackCol");
	ID = std::stoull(to_load->GetString("ID"));
	print_icon = to_load->GetBoolean("PrintIcon");
	camera_icon_color = to_load->GetColor("IconColor");
	if (to_load->GetBoolean("IsGameCamera")) {
		App->renderer3D->actual_game_camera = this;
	}
	if (to_load->GetBoolean("IsSelectedCamera")) {
		App->renderer3D->selected_game_camera = this;
	}

	activeFog = to_load->GetBoolean("Fog");
	fogDensity = (float)to_load->GetNumber("Density");
	fogGradient = (float)to_load->GetNumber("Gradient");


	cubemap->pos_x.assign(to_load->GetString("Skybox_PositiveX"));
	std::string path_pos_x = App->file_system->GetBaseFileName(cubemap->pos_x.c_str());
	ResourceTexture* tex_pos_x = (ResourceTexture*)App->resources->GetResourceWithID(std::stoull(path_pos_x));
	if (tex_pos_x != nullptr)
	{
		if (tex_pos_x->references == 0)
			tex_pos_x->IncreaseReferences();
		skybox->ChangePositiveX(skybox_texture_id, tex_pos_x->id, tex_pos_x->width, tex_pos_x->height);
	}

	cubemap->neg_x.assign(to_load->GetString("Skybox_NegativeX"));
	std::string path_neg_x = App->file_system->GetBaseFileName(cubemap->neg_x.c_str());
	ResourceTexture* tex_neg_x = (ResourceTexture*)App->resources->GetResourceWithID(std::stoull(path_neg_x));
	if (tex_neg_x != nullptr)
	{
		if (tex_neg_x->references == 0)
			tex_neg_x->IncreaseReferences();
		skybox->ChangeNegativeX(skybox_texture_id, tex_neg_x->id, tex_neg_x->width, tex_neg_x->height);
	}
	
	cubemap->pos_y.assign(to_load->GetString("Skybox_PositiveY"));
	std::string path_pos_y = App->file_system->GetBaseFileName(cubemap->pos_y.c_str());
	ResourceTexture* tex_pos_y = (ResourceTexture*)App->resources->GetResourceWithID(std::stoull(path_pos_y));
	if (tex_pos_y != nullptr)
	{
		if (tex_pos_y->references == 0)
			tex_pos_y->IncreaseReferences();
		skybox->ChangePositiveY(skybox_texture_id, tex_pos_y->id, tex_pos_y->width, tex_pos_y->height);
	}
	
	cubemap->neg_y.assign(to_load->GetString("Skybox_NegativeY"));
	std::string path_neg_y = App->file_system->GetBaseFileName(cubemap->neg_y.c_str());
	ResourceTexture* tex_neg_y = (ResourceTexture*)App->resources->GetResourceWithID(std::stoull(path_neg_y));
	if (tex_neg_y != nullptr)
	{
		if (tex_neg_y->references == 0)
			tex_neg_y->IncreaseReferences();
		skybox->ChangeNegativeY(skybox_texture_id, tex_neg_y->id, tex_neg_y->width, tex_neg_y->height);
	}
	
	cubemap->pos_z.assign(to_load->GetString("Skybox_PositiveZ"));
	std::string path_pos_z = App->file_system->GetBaseFileName(cubemap->pos_z.c_str());
	ResourceTexture* tex_pos_z = (ResourceTexture*)App->resources->GetResourceWithID(std::stoull(path_pos_z));
	if (tex_pos_z != nullptr)
	{
		if (tex_pos_z->references == 0)
			tex_pos_z->IncreaseReferences();
		skybox->ChangePositiveZ(skybox_texture_id, tex_pos_z->id, tex_pos_z->width, tex_pos_z->height);
	}

	cubemap->neg_z.assign(to_load->GetString("Skybox_NegativeZ"));
	std::string path_neg_z = App->file_system->GetBaseFileName(cubemap->neg_z.c_str());
	ResourceTexture* tex_neg_z = (ResourceTexture*)App->resources->GetResourceWithID(std::stoull(path_neg_z));
	if (tex_neg_z != nullptr)
	{
		if (tex_neg_z->references == 0)
			tex_neg_z->IncreaseReferences();
		skybox->ChangeNegativeZ(skybox_texture_id, tex_neg_z->id, tex_neg_z->width, tex_neg_z->height);
	}

#ifdef GAME_VERSION
	auto faces = cubemap->ToVector();
	skybox_texture_id = skybox->LoadCubeMapFromLibraryFiles(faces);
#endif

	frustum.nearPlaneDistance = near_plane;
	frustum.farPlaneDistance = far_plane;
	frustum.verticalFov = vertical_fov * Maths::Deg2Rad();
	frustum.horizontalFov = horizontal_fov * Maths::Deg2Rad();

	if (game_object_attached != nullptr) {
		ComponentTransform* transform = game_object_attached->transform;
		frustum.pos = transform->GetGlobalPosition();
		frustum.front = transform->GetLocalRotation().WorldZ();
		frustum.up = transform->GetLocalRotation().WorldY();
	}
}

