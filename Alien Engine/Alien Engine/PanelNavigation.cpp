#include "PanelNavigation.h"
#include "ShortCutManager.h"
//#include "ModuleNavigation.h"
#include "Optick/include/optick.h"
#include "ModuleObjects.h"
#include "ModuleNavigation.h"


PanelNavigation::PanelNavigation(const std::string& panel_name, const SDL_Scancode& key1_down, const SDL_Scancode& key2_repeat, const SDL_Scancode& key3_repeat_extra) :
	Panel(panel_name, key1_down, key2_repeat, key3_repeat_extra)
{
	shortcut = App->shortcut_manager->AddShortCut("Panel Navigation", key1_down, std::bind(&Panel::ChangeEnable, this), key2_repeat, key3_repeat_extra);
	enabled = false;
}

PanelNavigation::~PanelNavigation()
{
}

void PanelNavigation::PanelLogic()
{
	OPTICK_EVENT();

	ImGui::Begin(panel_name.data(), &enabled);
	ImGui::Title("Panel Navigation");
	ImGui::Spacing();
	ImGui::Separator();

	ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
	if (ImGui::BeginTabBar("NavigationTabs", tab_bar_flags))
	{
		if (ImGui::BeginTabItem("Areas"))
		{
			ShowAreaTab();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Bake"))
		{
			ShowBakeTab();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Object"))
		{
			ShowObjectTab();
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	// ----------------------------------------------
	// Used to know when to draw navigation meshes and other recast data on scene panel
	ImDrawList* list = ImGui::GetWindowDrawList();
	if (list != nullptr && list->IdxBuffer.size() > 0) { // TODO: this is shit
		panel_nav_rendered = true;
	}
	else {
		panel_nav_rendered = false;
	}
	// -----------------------------------------------

	ImGui::Separator();

	if(ImGui::CollapsingHeader("Advanced options"))
	{
		const char* drawNames[] = { "DRAWMODE_NAVMESH",
									"DRAWMODE_VOXELS",
									"DRAWMODE_REGION_CONNECTIONS",
									"DRAWMODE_CONTOURS",
									"DRAWMODE_POLYMESH",
									"DRAWMODE_POLYMESH_DETAIL", };
		
		for (uint i = 0; i < (uint)NavDrawMode::MAX_DRAWMODE; ++i)
		{
			ImGui::Checkbox(drawNames[i], &App->nav->drawModes[i]);
		}
	}

	if (enabled != true)
		OnPanelDesactive();

	ImGui::End();
}

void PanelNavigation::ShowObjectTab()
{
	std::list<GameObject*> gos = App->objects->GetSelectedObjects();
	std::vector<GameObject*> mesh_gos;

	for (std::list<GameObject*>::iterator it = gos.begin(); it != gos.end(); ++it) {
		if ((*it)->GetComponent(ComponentType::MESH) != nullptr) { // TODO: needs to check mesh in children?
			mesh_gos.push_back((*it));
		}
	}

	uint num_go_valids = mesh_gos.size();
	ImGui::Text("selected %i gameobject/s", gos.size());
	ImGui::Text("with %i meshes", num_go_valids);

	ImGui::Separator();

	if (num_go_valids == 0)
	{
		ImGui::Text("Select any gameobject with mesh from the scene");
	}
	else
	{
		ImGui::Text("%s", mesh_gos.front()->GetName());
		if (num_go_valids > 1)
		{
			ImGui::SameLine();
			ImGui::Text(num_go_valids > 2 ? "and %i others" : "and %i other", num_go_valids - 1);
		}

		GameObject* first_selected = mesh_gos.front();
		if (ImGui::Checkbox("Navigation Static", &first_selected->nav_data.nav_static))
		{
			// change all valids gameobjects too at once
			for (uint i = 0; i < num_go_valids; ++i)
				mesh_gos[i]->nav_data.nav_static = first_selected->nav_data.nav_static;
		}
	}


	
}

void PanelNavigation::ShowAreaTab()
{
	ImGui::Text("Nothing to see here");
}

void PanelNavigation::ShowBakeTab()
{
	ImGui::Text("Baked Agent Size");
	ImGui::Separator();

	ImVec2 draw_canvas_size((36 + 10) * 9.8f, (36 + 10) * 3);
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	const ImVec2 p = ImGui::GetCursorScreenPos();
	const ImU32 light_blue = ImColor(0.13f, 0.72f, 1.0f, 1.0f);
	const ImU32 mid_blue = ImColor(0.22f, 0.62f, 1.0f, 0.6f);
	const ImU32 dark_blue = ImColor(0.02f, 0.46f, 1.0f, 1.0f);

	float window_w = ImGui::GetWindowContentRegionWidth();
	float window_center =  window_w * 0.5f;

	float* agent_radius = &App->nav->agentRadius;
	float* agent_height = &App->nav->agentHeight;

	int bottom_line_length = 60;
	const ImU32 line_col = ImColor(1.0f, 1.0f, 1.0f, 1.0f);
	const ImU32 grey_col = ImColor(0.5f, 0.5f, 0.5f, 1.0f);

	const float fixed_bottom_agent_line = 160;

	//1
	//const float ideal_proportion = 1.0f / 4.0f;//The width / height on the ideal cylinder
	//float curr_proportion = *agent_radius / *agent_height;
	//float proportion_diff = curr_proportion / ideal_proportion;

	//float draw_radius = *agent_radius * proportion_diff;
	//float draw_height = *agent_height / proportion_diff;

	//float norm = std::sqrt(draw_radius * draw_radius + draw_height * draw_height);

	//float square_min = fmin(draw_canvas_size.x, draw_canvas_size.y);

	//draw_radius = (draw_radius / norm) * square_min;
	//draw_height = (draw_height / norm) * square_min;

	//2
	float elipse_relation = 0.3f;//Normally the lower radius is half the bigger one
	
	//Normalize the proportions
	float norm = std::sqrt(*agent_radius * *agent_radius + *agent_height * *agent_height);
	float draw_radius = (*agent_radius / norm);
	float draw_height = (*agent_height / norm);
	
	//Scale it so it fits the maximum possible space available
	float square_min = fmin(draw_canvas_size.x, draw_canvas_size.y);
	draw_radius *= square_min;
	draw_height *= square_min;

	// drawing shapes ------------------------------------
	ImVec2 bot_ellipse_c(p.x + window_center, (p.y + draw_canvas_size.y) - (draw_radius * elipse_relation)); // Always "fixed"
	ImVec2 top_ellipse_center(p.x + window_center, bot_ellipse_c.y - draw_height);//(draw_radius * proportion_scale));
	float center_height = bot_ellipse_c.y - top_ellipse_center.y;

	// bottom "floor" line
	draw_list->AddLine(ImVec2(p.x, bot_ellipse_c.y), ImVec2(p.x + window_w, bot_ellipse_c.y), line_col);
	// bottom mid agent line
	float start_point_x = p.x + window_center - (fixed_bottom_agent_line * 0.5f);
	draw_list->AddLine(ImVec2(start_point_x, bot_ellipse_c.y), ImVec2(start_point_x + fixed_bottom_agent_line, bot_ellipse_c.y), grey_col, 2.0f);

	// body agent rect
	ImVec2 p_min(top_ellipse_center.x - draw_radius, bot_ellipse_c.y);
	ImVec2 p_max(top_ellipse_center.x + draw_radius, top_ellipse_center.y);

	draw_list->AddRectFilled(p_min, p_max, mid_blue);

	// bottom ellipse (only thing fixed on point)
	draw_list->AddEllipseFilled(bot_ellipse_c, draw_radius, draw_radius * elipse_relation, dark_blue, 0.0f, 24);

	// top ellipse
	draw_list->AddEllipseFilled(top_ellipse_center, draw_radius, draw_radius * elipse_relation, light_blue, 0.0f, 24);
	

	ImGui::Dummy(draw_canvas_size);

	ImGui::Separator();

	ImGui::SliderFloat("Agent Radius", agent_radius, 0.01f, 500.0f);
	ImGui::SliderFloat("Agent Height", agent_height, 0.01f, 500.0f);
	

	if (ImGui::Button("Bake", ImVec2(100, 20))) {
		App->nav->Bake();
	}
}

void PanelNavigation::OnPanelDesactive() // TODO: this call doesn't work if the window is NOT docked and is closed by the top right X icon
{
	panel_nav_rendered = false;
}