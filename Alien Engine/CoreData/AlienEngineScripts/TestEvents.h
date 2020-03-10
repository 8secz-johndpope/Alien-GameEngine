#pragma once

#include "..\..\Alien Engine\Alien.h"
#include "Macros/AlienScripts.h"
#include <functional>
class ALIEN_ENGINE_API TestEvents : public Alien {

public:

	TestEvents();
	virtual ~TestEvents();
	
	void Start();
	void Update();

	void Event();
	void Event2();
	void Event3();
};

ALIEN_FACTORY TestEvents* CreateTestEvents() {
	TestEvents* events = new TestEvents();
	// To show in inspector here

	SHOW_VOID_FUNCTION(TestEvents::Event, events);
	SHOW_VOID_FUNCTION(TestEvents::Event2, events);
	SHOW_VOID_FUNCTION(TestEvents::Event3, events);

	return events;
} 
