#pragma once

enum class EventType
{
	ON_PLAY = 0,
	ON_PAUSE,
	ON_STOP,
	ON_GO_SELECT,
	ON_ASSET_SELECT,
	ON_ASSET_DELETE,
	ON_GO_DELETE,
	ON_UNLOAD_SCENE,
	UNKNOWN
	//ON_UNPAUSE,
	//ON_DESTROY_OBJECT
	// Go nuts adding stuff
};

enum class AlienEventType
{
	RIGIDBODY_DELETED,
	RIGIDBODY_ADDED,	
	SCRIPT_DELETED,
	SCRIPT_ADDED,
	COLLIDER_DELETED,
	COLLIDER_ADDED,
	RESOURSE_ADDED,
	RESOURCE_DELETED,
	UNKNOWN
};

struct AlienEvent
{
	AlienEventType type = AlienEventType::UNKNOWN;
	void* object = nullptr;
};