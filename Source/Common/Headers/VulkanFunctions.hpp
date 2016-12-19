#ifndef __HIT_VULAKNFUNCTIONS_HPP__
#define __HIT_VULKANFUNCTIONS_HPP__

#include <vulkan/vulkan.h>

namespace Hit
{
#define VK_EXPORTED_FUNCTION( p_Function ) extern PFN_##p_Function p_Function;

#include <VulkanFunctions.inl>
}

#endif // __HIT_VULKANFUNCTIONS_HPP__

