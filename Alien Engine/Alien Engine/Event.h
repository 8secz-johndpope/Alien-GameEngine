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
	ON_SAVE,
	UNKNOWN
	//ON_UNPAUSE,
	//ON_DESTROY_OBJECT
	// Go nuts adding stuff
};

enum class AlienEventType
{
	SCALE_CHANGED,
	COLLISION_LAYER_STATE_CHANGED,
	COLLISION_LAYER_REMOVED,
	SCRIPT_ADDED,
	SCRIPT_DELETED,
	RIGIDBODY_ADDED,	
	RIGIDBODY_DELETED,
	RIGIDBODY_ENABLED,
	RIGIDBODY_DISABLED,
	COLLIDER_ADDED,
	COLLIDER_DELETED,
	COLLIDER_ENABLED,
	COLLIDER_DISABLED,
	RESOURSE_ADDED,
	RESOURCE_DELETED,
	RESOURCE_SELECTED,
	RESOURCE_DESELECTED,
	UNKNOWN
};

struct AlienEvent
{
	AlienEventType type = AlienEventType::UNKNOWN;
	void* object = nullptr;
};
