#pragma once
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GLOBAL SETTINGS
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// [x] USE_OPTICK					- (Master Switch)
// [x] OPTICK_ENABLE_TRACING		- (Enable Kernel-level tracing)
// [x] OPTICK_ENABLE_GPU_D3D12		- (GPU D3D12)
// [x] OPTICK_ENABLE_GPU_VULKAN		- (GPU VULKAN)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// MASTER SWITCH - use it for disabling profiler in final builds															  //
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if !defined(USE_OPTICK)
#define USE_OPTICK (1)
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Enable Low-level platform-specific tracing (Switch Contexts, Autosampling, etc.)											  
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if !defined(OPTICK_ENABLE_TRACING)
#define OPTICK_ENABLE_TRACING (USE_OPTICK /*&& 0*/)
#endif //OPTICK_ENABLE_TRACING
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GPU Counters																										  
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#if !defined(OPTICK_ENABLE_GPU)
#define OPTICK_ENABLE_GPU (USE_OPTICK /*&& 0*/)
#endif //OPTICK_ENABLE_GPU

// D3D12
#if !defined(OPTICK_ENABLE_GPU_D3D12)
#if defined(_MSC_VER)
#define OPTICK_ENABLE_GPU_D3D12 (OPTICK_ENABLE_GPU /*&& 0*/)
#else
#define OPTICK_ENABLE_GPU_D3D12 (0)
#endif
#endif

// VUKLAN
#if !defined(OPTICK_ENABLE_GPU_VULKAN)
#if defined(_MSC_VER)
#define OPTICK_ENABLE_GPU_VULKAN (OPTICK_ENABLE_GPU /*&& 0*/)
#else
#define OPTICK_ENABLE_GPU_VULKAN (0)
#endif
#endif


