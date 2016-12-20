#include <vulkan/vulkan.h>

namespace Hit
{
#define VK_EXPORTED_FUNCTION( p_Function ) \
	PFN_##p_Function p_Function = nullptr;
#define VK_GLOBAL_LEVEL_FUNCTION( p_Function ) \
	PFN_##p_Function p_Function = nullptr;
#define VK_INSTANCE_LEVEL_FUNCTION( p_Function ) \
	PFN_##p_Function p_Function = nullptr;
#define VK_DEVICE_LEVEL_FUNCTION( p_Function ) \
	PFN_##p_Function p_Function = nullptr;

#include <VulkanFunctions.inl>
}

