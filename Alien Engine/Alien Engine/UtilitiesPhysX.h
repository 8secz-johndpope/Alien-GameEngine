#pragma once
#include "gpu/PxGpu.h"
#include "common/windows/PxWindowsDelayLoadHook.h"
#include "physx\include\PxPhysicsAPI.h"
#include "MathGeoLib/include/Math/float4x4.h"
#include "MathGeoLib/include/Math/float3.h"
#include "MathGeoLib/include/Math/Quat.h"
#include "ComponentCollider.h"

#define PX_RELEASE(x)	if(x)	{ x->release(); x = NULL;}
#define APP_BIN_DIR "DLLs"
#if		PX_WIN64
#define DLL_NAME_BITS "64"
#else
#define DLL_NAME_BITS "32"
#endif
#if		PX_DEBUG
#define DLL_DIR "\\physx\\_debug\\"
#elif	PX_CHECKED
#define DLL_DIR "checked\\" 
#elif	PX_PROFILE
#define DLL_DIR "profile\\" 
#else
#define DLL_DIR "\\physx\\_release\\" 
#endif

#define DEFAULT_STATIC_FRICTION 0.5f
#define DEFAULT_DYNAMIC_FRICTION 0.5f
#define DEFAULT_RESTIUTION 0.5f

using namespace physx;

inline PxExtendedVec3	PXVEC3_TO_VEC3EXT(const PxVec3 & vec) { return  PxExtendedVec3(vec.x, vec.y, vec.z); }
inline PxExtendedVec3	F3_TO_PXVEC3EXT(const float3& vec) { return  PxExtendedVec3(vec.x, vec.y, vec.z); }
inline PxVec3			PXVEC3EXT_TO_PXVEC3(const PxExtendedVec3 & vec) { return  PxVec3(vec.x, vec.y, vec.z); }
inline PxVec3			F3_TO_PXVEC3(const float3& vec) { return  PxVec3(vec.x, vec.y, vec.z); }
inline float3			PXVEC3EXT_TO_F3(const PxExtendedVec3 & vec) { return  float3(vec.x, vec.y, vec.z); }
inline float3			PXVEC3_TO_F3(const PxVec3 & vec) { return  float3(vec.x, vec.y, vec.z); }
inline PxQuat			QUAT_TO_PXQUAT(const Quat & quat) { return  PxQuat(quat.x, quat.y, quat.z, quat.w); }
inline Quat				PXQUAT_TO_QUAT(const PxQuat & quat) { return  Quat(quat.x, quat.y, quat.z, quat.w); }

inline bool F4X4_TO_PXTRANS(const float4x4 & input, PxTransform & output ) {
	Quat rot; float3 pos;
	pos = input.TranslatePart();
	rot = rot.LookAt(float3(0, 0, 1), input.WorldZ(), float3(0, 1, 0), input.WorldY());
	if (!pos.IsFinite() || !rot.IsFinite()) return false;
	output = PxTransform(F3_TO_PXVEC3(pos), QUAT_TO_PXQUAT(rot));
	return true;
}
//return PxTransform(F3_TO_PXVEC3(trans.TranslatePart()), QUAT_TO_PXQUAT(trans.RotatePart().RemoveScale2().ToQuat())); 
//rot = input.RotatePart().RemoveScale2().ToQuat();
//rot = Quat::FromEulerXYZ(input.ToEulerXYZ().x, input.ToEulerXYZ().y, input.ToEulerXYZ().z);

inline float4x4			PXTRANS_TO_F4X4(const PxTransform & trans) { return float4x4(PXQUAT_TO_QUAT(trans.q), PXVEC3_TO_F3(trans.p)); }

class CustomDelayLoadHook : public PxDelayLoadHook
{

	virtual const char* getPhysXCommonDllName() const
	{
		return "TODO";//DLLs\\physx\\debug\\PhysXCommon_32.dll";
	}

	virtual const char* getPhysXFoundationDllName() const
	{
		return "TODO";//DLLs\\physx\\debug\\PhysXFoundation_32.dll";
	}
};

class CustomGpuLoadHook : public PxGpuLoadHook
{
	virtual const char* getPhysXGpuDllName() const
	{
		return "TODO";//DLLs\\physx\\debug\\PhysXGpu_32.dll";;
	}
};

class CustomErrorCallback : public PxErrorCallback
{
public:
	void reportError(PxErrorCode::Enum code, const char* message, const char* file, int line);
};


PxFilterFlags FilterShader(PxFilterObjectAttributes attributes0, PxFilterData filterData0, PxFilterObjectAttributes attributes1, PxFilterData filterData1, physx::PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize);

class ControllerFilterCallback : public  PxQueryFilterCallback
{
public:
	virtual PxQueryHitType::Enum preFilter(const PxFilterData& filterData, const PxShape* shape, const PxRigidActor* actor, PxHitFlags& queryFlags);
	virtual PxQueryHitType::Enum postFilter(const PxFilterData& filterData, const PxQueryHit& hit);
};


enum class CallbackType { ENTER = 0, STAY = 1, EXIT = 2, UNKNOWN = 3 };

class SimulationEventCallback : public PxSimulationEventCallback
{
public:

	SimulationEventCallback() {}
	~SimulationEventCallback() {}

	void onContact(const PxContactPairHeader& pair_header, const PxContactPair* pairs, PxU32 num_pairs);
	void onTrigger(PxTriggerPair* pairs, PxU32 num_pairs);

	// Happen before fetchResults() (swaps the buffers)
	void onConstraintBreak(physx::PxConstraintInfo* constraints, physx::PxU32 count) {}
	void onAdvance(const physx::PxRigidBody* const* bodyBuffer, const physx::PxTransform* poseBuffer, const physx::PxU32 count) {}
	// Happen after fetchResults() (swaps the buffers)
	void onWake(physx::PxActor** actors, physx::PxU32 count) {}
	void onSleep(physx::PxActor** actors, physx::PxU32 count) {}

	const char* oncollision_names[3] = { "OnCollisionEnter()", "OnCollisionStay()", "OnCollisonExit()" };
	const char* ontrigger_names[3] = { "OnTriggerEnter()", "OnTriggerStay()", "OnTriggerExit()" };
};
