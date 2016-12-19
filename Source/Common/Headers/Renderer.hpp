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
		static const HIT_SINT32 LOAD_ENTRY_POINTS_FAILED = -101;

	private:
		HIT_SINT32 LoadVulkanLibrary( );
		HIT_SINT32 LoadExportedEntryPoints( );

		void		*m_pVulkanLibraryHandle;
		VkInstance	m_VulkanInstance;
		VkDevice	m_VulkanDevice;
		VkQueue		m_VulkanQueue;
	};
}

#endif // __HIT_RENDERER_HPP__

