#include <vulkan/vulkan.h>

namespace Hit
{
#define VK_EXPORTED_FUNCTION( p_Function ) PFN_##p_Function p_Function;

#include <VulkanFunctions.inl>
}

