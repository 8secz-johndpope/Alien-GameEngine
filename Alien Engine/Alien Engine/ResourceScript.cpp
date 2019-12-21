#include "ResourceScript.h"
#include "Application.h"
#include "ModuleResources.h"
#include <fstream>

ResourceScript::ResourceScript()
{
	type = ResourceType::RESOURCE_SCRIPT;
}

ResourceScript::~ResourceScript()
{
}

bool ResourceScript::CreateMetaData(const u64& force_id)
{
	bool ret = true;

	if (force_id != 0)
		ID = force_id;
	else
		ID = App->resources->GetRandomID();

	meta_data_path = std::string(path.data());

	std::ifstream file(path);
	if (file.is_open()) {
		std::string line;
		while (std::getline(file, line)) {
			if (line.find("ALIEN_ENGINE_API") != std::string::npos) {
				bool alien = false;
				if (line.find("Alien") != std::string::npos) {
					alien = true;
				}
				std::string class_name = GetDataStructure(line, "ALIEN_ENGINE_API");
				data_structures.push_back({ class_name, alien });
			}
		}
		file.close();
	}

	path = std::string(SCRIPTS_FOLDER + name + ".alienScript").data();

	JSON_Value* value = json_value_init_object();
	JSON_Object* json_object = json_value_get_object(value);
	json_serialize_to_file_pretty(value, path.data());

	if (value != nullptr && json_object != nullptr) {
		JSONfilepack* script = new JSONfilepack(path.data(), json_object, value);
		script->StartSave();

		script->SetString("Meta.ID", std::to_string(ID));

		struct stat file;
		if (stat(meta_data_path.c_str(), &file) == 0)
		{
			last_time_mod = file.st_mtime;
		}
		if (!data_structures.empty()) {
			script->SetBoolean("HasData", true);
			JSONArraypack* structures = script->InitNewArray("DataStructure");

			for (uint i = 0; i < data_structures.size(); ++i) {
				structures->SetAnotherNode();
				structures->SetString("DataName", data_structures[i].first);
				structures->SetBoolean("UsesAlien", data_structures[i].second);
			}
		}
		else {
			script->SetBoolean("HasData", false);
		}
		script->FinishSave();
		delete script;
		App->resources->AddResource(this);
	}

	return ret;
}

bool ResourceScript::ReadBaseInfo(const char* assets_file_path)
{
	meta_data_path = std::string(path.data());
	path = std::string(assets_file_path);

	ID = App->resources->GetIDFromAlienPath(assets_file_path);
	remove(assets_file_path);
	std::ifstream file(meta_data_path);
	if (file.is_open()) {
		std::string line;
		while (std::getline(file, line)) {
			if (line.find("ALIEN_ENGINE_API") != std::string::npos) {
				bool alien = false;
				if (line.find("Alien") != std::string::npos) {
					alien = true;
				}
				std::string class_name = GetDataStructure(line, "ALIEN_ENGINE_API");
				data_structures.push_back({ class_name, alien });
			}
		}
		file.close();
	}

	JSON_Value* value = json_value_init_object();
	JSON_Object* json_object = json_value_get_object(value);
	json_serialize_to_file_pretty(value, path.data());

	if (value != nullptr && json_object != nullptr) {
		JSONfilepack* script = new JSONfilepack(path.data(), json_object, value);
		script->StartSave();

		script->SetString("Meta.ID", std::to_string(ID));

		struct stat file;
		if (stat(meta_data_path.c_str(), &file) == 0)
		{
			last_time_mod = file.st_mtime;
		}
		if (!data_structures.empty()) {
			script->SetBoolean("HasData", true);
			JSONArraypack* structures = script->InitNewArray("DataStructure");

			for (uint i = 0; i < data_structures.size(); ++i) {
				structures->SetAnotherNode();
				structures->SetString("DataName", data_structures[i].first);
				structures->SetBoolean("UsesAlien", data_structures[i].second);
			}
		}
		else {
			script->SetBoolean("HasData", false);
		}
		script->FinishSave();
		delete script;
		App->resources->AddResource(this);
	}
	
	return true;
}

bool ResourceScript::NeedReload() const
{
	bool ret = false;

	struct stat file;
	if (stat(meta_data_path.c_str(), &file) == 0)
	{
		if (file.st_mtime != last_time_mod)
			ret = true;
	}

	return ret;
}

std::string ResourceScript::GetDataStructure(const std::string& line, const std::string& api)
{
	std::string data_name;
	bool start_cpoying = false;
	bool api_found = false;
	for (uint i = 0; i < line.size(); ++i) {
		if (!api_found) {
			if (line[i] != ' ' && line[i] != '\t') {
				data_name += line[i];
			}
			else {
				if (App->StringCmp(api.data(), data_name.data())) {
					api_found = true;
				}
				data_name.clear();
			}
		}
		else {
			if (!start_cpoying) {
				if (line[i] != ' ' && line[i] != '\t') {
					start_cpoying = true;
					data_name += line[i];
				}
			}
			else {
				if (line[i] != ' ' && line[i] != '\t') {
					data_name += line[i];
				}
				else
					break;
			}
		}
	}
	return data_name;
}
