#pragma once

#include "..\..\Alien Engine\Alien.h"
#include "Macros/AlienScripts.h"

#include <string>

enum (MYENUM2,
	fdfsdf = 2,
	tsfdswo = -34,
	asdada
	);

class ALIEN_ENGINE_API TestEvents : public Alien {
public:
	enum (MYENUM,
		one = -3,
		two = 2,
		three
		);
public:

	TestEvents();
	virtual ~TestEvents();
	
	void Start();
	void Update();


	void Event();
	void Event2();
	void Event3();
	std::string myString = "a";
	const char* testChar = "AAAAAAAAAAAAAAaa";

public:
	MYENUM myEnumTest = MYENUM::two;
	MYENUM2 myEnumTest2 = MYENUM2::asdada;
};

ALIEN_FACTORY TestEvents* CreateTestEvents() {
	TestEvents* events = new TestEvents();
	// To show in inspector here
	SHOW_VOID_FUNCTION(TestEvents::Event, events);
	SHOW_VOID_FUNCTION(TestEvents::Event3, events);
	SHOW_VOID_FUNCTION(TestEvents::Event2, events);
	SHOW_TEXT("BBBBBBBBBBBBBBBBBBBBB");
	SHOW_SPACING();
	SHOW_IN_INSPECTOR_AS_ENUM(TestEvents::MYENUM, events->myEnumTest);
	SHOW_TOOLTIP("Hello"); // <--- Print (?) at the right of the previous item called to show in inspector, when hovered text appears.

	SHOW_SPACING();
	SHOW_SEPARATOR();
	SHOW_SPACING();

	SHOW_TEXT("CCCCCCCCCCCCCCCCCCCCCCCC");
	SHOW_SPACING();
	SHOW_IN_INSPECTOR_AS_ENUM(MYENUM2, events->myEnumTest2);

	SHOW_IN_INSPECTOR_AS_STRING(events->myString);

	return events;
} 
