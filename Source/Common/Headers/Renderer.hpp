#ifndef __HIT_RENDERER_HPP__
#define __HIT_RENDERER_HPP__

#include <DataTypes.hpp>
#include <vulkan/vulkan.h>

namespace Hit
{
	class Renderer
	{
	public:
		Renderer( );
		~Renderer( );

		HIT_SINT32 Initialise( );

		static const HIT_SINT32 OK = 0;
		static const HIT_SINT32 LOAD_LIBRARY_FAILED = -100;
		static const HIT_SINT32 LOAD_EXPORTED_ENTRY_POINTS_FAILED = -101;
		static const HIT_SINT32 LOAD_GLOBAL_ENTRY_POINTS_FAILED = -102;
		static const HIT_SINT32 LOAD_INSTANCE_ENTRY_POINTS_FAILED = -103;
		static const HIT_SINT32 LOAD_DEVICE_ENTRY_POINTS_FAILED = -104;
		static const HIT_SINT32 CREATE_VULKAN_INSTANCE_FAILED = -1000;
		static const HIT_SINT32 CREATE_VULKAN_DEVICE_FAILED = -1001;
		static const HIT_SINT32 PHYSICAL_DEVICE_ENUMERATE_FAILED = -3000;
		static const HIT_SINT32 PHYSICAL_DEVICE_SELECT_FAILED = -3001;

	private:
		HIT_SINT32 LoadVulkanLibrary( );
		HIT_SINT32 LoadExportedEntryPoints( );
		HIT_SINT32 LoadGlobalLevelEntryPoints( );
		HIT_SINT32 CreateVulkanInstance( );
		HIT_SINT32 LoadInstanceLevelEntryPoints( );
		HIT_SINT32 CreateVulkanDevice( );
		HIT_SINT32 LoadDeviceLevelEntryPoints( );

		HIT_BOOL CheckPhysicalDeviceProperties(
			VkPhysicalDevice p_PhysicalDevice, uint32_t &p_QueueFamilyIndex );

		void		*m_pVulkanLibraryHandle;
		VkInstance	m_VulkanInstance;
		VkDevice	m_VulkanDevice;
		VkQueue		m_VulkanQueue;
		uint32_t	m_QueueFamilyIndex;
	};
}

#endif // __HIT_RENDERER_HPP__

