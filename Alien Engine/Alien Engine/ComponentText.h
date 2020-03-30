#ifndef _COMPONENT_TEXT_H_
#define _COMPONENT_TEXT_H_

#include "ComponentUI.h"

class ResourceFont;
struct Character;

enum TextAlign {
	NONE = -1,
	LEFT,
	MIDDLE,
	RIGHT

};

class __declspec(dllexport) ComponentText :public ComponentUI
{
public:
	ComponentText(GameObject* obj);
	~ComponentText();

	bool DrawInspector();

	void Draw(bool isGame) override;

	void Reset();
	void SetComponent(Component* component);
	void Clone(Component* clone);

	void SaveComponent(JSONArraypack* to_save);
	void LoadComponent(JSONArraypack* to_load);

	void SetFont(ResourceFont* font);
	ResourceFont* GetFont() const;

	void GenerateVAOVBO();

public:
	std::string		text = "Non-Text";

private:
	TextAlign align = TextAlign::LEFT;
	ResourceFont*	font = nullptr;
	uint VAO;
	int width = 200;
	float interlineal = 1.5;
};

#endif // !_COMPONENT_TEXT_H_
