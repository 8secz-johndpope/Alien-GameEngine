#include <fstream>
#include "ComponentMaterial.h"
#include "glew/include/glew.h"
#include "GameObject.h"
#include "imgui/imgui.h"
#include "ComponentMesh.h"
#include "Application.h"
#include "ResourceTexture.h"
#include "ResourceMaterial.h"
#include "ReturnZ.h"
#include "mmgr/mmgr.h"

ComponentMaterial::ComponentMaterial(GameObject* attach) : Component(attach)
{
	type = ComponentType::MATERIAL;

	material = App->resources->default_material;
}

ComponentMaterial::~ComponentMaterial()
{
	if (material != nullptr)
		material->DecreaseReferences();
}

bool ComponentMaterial::DrawInspector()
{
	static bool en;
	ImGui::PushID(this);
	en = enabled;
	if (ImGui::Checkbox("##CmpActive", &en)) {
		ReturnZ::AddNewAction(ReturnZ::ReturnActions::CHANGE_COMPONENT, this);
		enabled = en;
	}
	ImGui::PopID();
	ImGui::SameLine();

	if (ImGui::CollapsingHeader("Material", &not_destroy, ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text(material->GetName());
		material->DisplayMaterialOnInspector();
		RightClickMenu("Material");

		ImGui::Spacing();
		ImGui::Spacing();


		//InspectorShaderProperties();

		/*ImGui::Spacing();
		static bool set_Z = true;
		ImGui::Spacing();
		static Color col;
		col = color;
		if (ImGui::ColorEdit3("Material Color", &col, ImGuiColorEditFlags_Float)) {
			if (set_Z)
				ReturnZ::AddNewAction(ReturnZ::ReturnActions::CHANGE_COMPONENT, this);
			set_Z = false;
			color = col;
		}
		else if (!set_Z && ImGui::IsMouseReleased(0)) {
			set_Z = true;
		}

		if (ImGui::SliderFloat("Alpha", &col.a, 0.0F, 1.0F)) {
			if (set_Z)
				ReturnZ::AddNewAction(ReturnZ::ReturnActions::CHANGE_COMPONENT, this);
			set_Z = false;
			color.a = col.a;
		}
		else if (!set_Z && ImGui::IsMouseReleased(0)) {
			set_Z = true;
		}*/

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		ImGui::Spacing();
		ImGui::Text("Texture Information");

	}
	else
		RightClickMenu("Material");

	return true;
}

void ComponentMaterial::InspectorShaderProperties()
{
	/* Shaders */
	//if (used_shader != nullptr)
	//{
	//	/* Set shader unifroms from Inspector */
	//	if (used_shader->ChangeTemplate())
	//	{
	//		if (used_shader != nullptr) {
	//			used_shader->DecreaseReferences();
	//		}

	//		switch (used_shader->uniform_data.type) {
	//		case SHADER_TEMPLATE::DIFUSSE: {//defusse
	//			std::string p = std::string(SHADERS_FOLDER + std::string("default_meta.alien"));
	//			u64 id_s = App->resources->GetIDFromAlienPath(p.data());
	//			used_shader = (ResourceShader*)App->resources->GetResourceWithID(id_s);
	//			if (used_shader != nullptr) {
	//				used_shader->IncreaseReferences();
	//			}
	//			break; }

	//		case SHADER_TEMPLATE::WAVE: {//wave

	//			std::string p = std::string(SHADERS_FOLDER + std::string("shader_wave_meta.alien"));
	//			u64 id_s = App->resources->GetIDFromAlienPath(p.data());
	//			used_shader = (ResourceShader*)App->resources->GetResourceWithID(id_s);
	//			if (used_shader != nullptr) {
	//				used_shader->IncreaseReferences();
	//			}
	//			break; }

	//		case SHADER_TEMPLATE::BASIC_LIGHTING: {//wave

	//			std::string p = std::string(SHADERS_FOLDER + std::string("basic_lighting_meta.alien"));
	//			u64 id_s = App->resources->GetIDFromAlienPath(p.data());
	//			used_shader = (ResourceShader*)App->resources->GetResourceWithID(id_s);
	//			if (used_shader != nullptr) {
	//				used_shader->IncreaseReferences();
	//			}
	//			break; }
	//		}
	//		file_to_edit = used_shader->path;
	//	}

	//	used_shader->HierarchyUniforms();

	//	ImGui::Separator();
	//	ImGui::Text("Current shader: "); ImGui::SameLine();
	//	ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), used_shader->path.c_str());
	//	if (ImGui::Button("Select Shader"))
	//	{
	//		select_shader = true;
	//	}
	//	ImGui::SameLine();
	//	if (ImGui::Button("Edit shader"))
	//	{
	//		{
	//			std::ifstream t(file_to_edit.c_str());
	//			if (t.good())
	//			{
	//				std::string str((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
	//				shader_editor.SetText(str);
	//			}
	//		}

	//		show_shader_text_editor = true;
	//	}
	//	ImGui::SameLine();

	//	static const char* text_compilation_shader = "";
	//	static bool compiled_shader_success = false;
	//	if (ImGui::Button("Compile shader")) // TODO: Compile automatically when we save and show error
	//	{
	//		if (used_shader->ParseAndCreateShader() == 0)
	//		{
	//			compiled_shader_success = false;
	//			text_compilation_shader = "Shader compilation unsuccessful. Please fix your code.";
	//			LOG_ENGINE("Shader compiled unsuccessfully...");
	//		}
	//		else
	//		{
	//			compiled_shader_success = true;
	//			text_compilation_shader = "Shader compilation successful.";
	//			LOG_ENGINE("Shader compiled successfully.");
	//		}
	//	}

	//	compiled_shader_success ? ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), text_compilation_shader)
	//		: ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), text_compilation_shader);

	//	// Can select desired shader in the shaders folder
	//	if (select_shader)
	//	{
	//		ImGui::OpenPopup("Select Shader");
	//		ImGui::SetNextWindowSize(ImVec2(600.0f, 800.0f));
	//		if (ImGui::BeginPopupModal("Select Shader"))
	//		{
	//			if (ImGui::Button("Close"))
	//			{
	//				select_shader = false;
	//			}

	//			std::vector<ResourceShader*> shaders;
	//			App->resources->GetShaders(shaders);
	//			for (auto i = shaders.begin(); i != shaders.end(); ++i)
	//			{
	//				if (ImGui::Button((*i)->GetName()))
	//				{
	//					std::string shader_name = (*i)->GetName();
	//					std::string p = std::string(SHADERS_FOLDER + shader_name + "_meta.alien");
	//					u64 id_s = App->resources->GetIDFromAlienPath(p.data());
	//					if (used_shader != nullptr) {
	//						used_shader->DecreaseReferences();
	//					}
	//					used_shader = (ResourceShader*)App->resources->GetResourceWithID(id_s);
	//					if (used_shader != nullptr) {
	//						used_shader->IncreaseReferences();
	//					}
	//					file_to_edit = used_shader->path; // must test if it edits on library too in this engine
	//				}
	//			}

	//			ImGui::EndPopup();
	//		}
	//	}

	//	if (show_shader_text_editor)
	//	{
	//		ShowShaderTextEditor();
	//	}
	//}
}

void ComponentMaterial::Reset()
{
	if (material != nullptr)
		material->DecreaseReferences();
	
	material = App->resources->default_material;
	if(material != nullptr)
		material->IncreaseReferences();
}

void ComponentMaterial::SetComponent(Component* component)
{
	/*if (component->GetType() == type) {

		ComponentMaterial* material = (ComponentMaterial*)component;
		if (texture != nullptr) {
			texture->DecreaseReferences();
		}
		texture = material->texture;
		if (texture != nullptr) {
			texture->IncreaseReferences();
		}

		color = material->color;
	}*/

	// Newest
	/*if (component->GetType() == type) {

		ComponentMaterial* material = (ComponentMaterial*)component;
		if (texture != nullptr) {
			texture->DecreaseReferences();
		}
		texture = material->texture;
		if (texture != nullptr) {
			texture->IncreaseReferences();
		}
		if (used_shader != nullptr) {
			used_shader->DecreaseReferences();
		}
		used_shader = material->used_shader;
		if (used_shader != nullptr) {
			used_shader->IncreaseReferences();
		}

		color = material->color;
	}*/
}

void ComponentMaterial::SaveComponent(JSONArraypack* to_save)
{
	to_save->SetNumber("Type", (int)type);
	to_save->SetString("ID", std::to_string(ID));
	to_save->SetBoolean("HasMaterial", (material != nullptr) ? true : false);
	if (material != nullptr) {
		to_save->SetString("MaterialID", std::to_string(material->GetID()));
	}
	to_save->SetBoolean("Enabled", enabled);
}

void ComponentMaterial::LoadComponent(JSONArraypack* to_load)
{
	enabled = to_load->GetBoolean("Enabled");

	if (to_load->GetBoolean("HasMaterial")) {
		u64 ID = std::stoull(to_load->GetString("MaterialID"));
		material = (ResourceMaterial*)App->resources->GetResourceWithID(ID);
		if (material != nullptr)
			material->IncreaseReferences();
	}
	ID = std::stoull(to_load->GetString("ID"));
}


void ComponentMaterial::Clone(Component* clone)
{
	/*clone->enabled = enabled;
	clone->not_destroy = not_destroy;
	ComponentMaterial* mat = (ComponentMaterial*)clone;
	mat->color = color;
	mat->texture = texture;
	if (texture != nullptr) {
		++texture->references;
	}
	mat->texture_activated = texture_activated;*/


	// Newest
	/*clone->enabled = enabled;
	clone->not_destroy = not_destroy;
	ComponentMaterial* mat = (ComponentMaterial*)clone;
	mat->color = color;
	mat->texture = texture;
	mat->used_shader = used_shader;
	if (texture != nullptr) {
		++texture->references;
	}
	if (used_shader != nullptr) {
		++used_shader->references;
	}
	mat->texture_activated = texture_activated;*/
}

void ComponentMaterial::SetTexture(ResourceTexture* tex)
{
	if (tex == nullptr)
	{
		RemoveTexture();
		return;
	}

	// Detach current texture
	if (material->texture != nullptr) {
		material->texture->DecreaseReferences();
	}

	// Create new material to assign the new texture to if you are using the default material
	if (material == App->resources->default_material)
	{
		SetMaterial(App->resources->CreateMaterial(tex->GetName()));
	}

	// Finally assign the texture to the desired material
	material->texture = tex;
	material->texture->IncreaseReferences();
}

void ComponentMaterial::RemoveTexture()
{
	if (material->texture == nullptr)
		return;

	material->texture->DecreaseReferences();
	material->texture = nullptr;
}

const ResourceTexture* ComponentMaterial::GetTexture() const
{
	return material->texture;
}

void ComponentMaterial::SetMaterial(ResourceMaterial* mat)
{
	if (mat == nullptr)
		return; 

	if (material != nullptr)
		material->DecreaseReferences();

	material = mat;
	material->IncreaseReferences(); 
}

const ResourceMaterial* ComponentMaterial::GetMaterial() const
{
	return material;
}



void ComponentMaterial::ShowShaderTextEditor()
{
	//auto cpos = shader_editor.GetCursorPosition();
	//ImGui::SetNextWindowPosCenter(ImGuiCond_FirstUseEver);
	//ImGui::Begin("Shader Editor", &show_shader_text_editor, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
	//ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);

	//if (ImGui::BeginMenuBar())
	//{
	//	if (ImGui::BeginMenu("File"))
	//	{
	//		if (ImGui::MenuItem("Save"))
	//		{
	//			auto textToSave = shader_editor.GetText();

	//			// Save text assets folder
	//			App->file_system->Save(file_to_edit.c_str(), textToSave.c_str(), textToSave.size());

	//			// Save text library folder
	//			App->file_system->Save(used_shader->GetLibraryPath(), textToSave.c_str(), textToSave.size());
	//		}

	//		ImGui::EndMenu();
	//	}
	//	if (ImGui::BeginMenu("Edit"))
	//	{
	//		bool ro = shader_editor.IsReadOnly();
	//		if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
	//			shader_editor.SetReadOnly(ro);
	//		ImGui::Separator();

	//		if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr, !ro && shader_editor.CanUndo()))
	//			shader_editor.Undo();
	//		if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && shader_editor.CanRedo()))
	//			shader_editor.Redo();

	//		ImGui::Separator();

	//		if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, shader_editor.HasSelection()))
	//			shader_editor.Copy();
	//		if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr, !ro && shader_editor.HasSelection()))
	//			shader_editor.Cut();
	//		if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && shader_editor.HasSelection()))
	//			shader_editor.Delete();
	//		if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
	//			shader_editor.Paste();

	//		ImGui::Separator();

	//		if (ImGui::MenuItem("Select all", nullptr, nullptr))
	//			shader_editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(shader_editor.GetTotalLines(), 0));

	//		ImGui::EndMenu();
	//	}

	//	if (ImGui::BeginMenu("View"))
	//	{
	//		if (ImGui::MenuItem("Dark palette"))
	//			shader_editor.SetPalette(TextEditor::GetDarkPalette());
	//		if (ImGui::MenuItem("Light palette"))
	//			shader_editor.SetPalette(TextEditor::GetLightPalette());
	//		if (ImGui::MenuItem("Retro blue palette"))
	//			shader_editor.SetPalette(TextEditor::GetRetroBluePalette());
	//		ImGui::EndMenu();
	//	}

	//	ImGui::EndMenuBar();
	//}

	//ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1, cpos.mColumn + 1, shader_editor.GetTotalLines(),
	//	shader_editor.IsOverwrite() ? "Ovr" : "Ins",
	//	shader_editor.CanUndo() ? "*" : " ",
	//	shader_editor.GetLanguageDefinition().mName.c_str(), file_to_edit.c_str());

	//shader_editor.Render("TextEditor");

	//ImGui::End();
}
