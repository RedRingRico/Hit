#ifndef __HIT_RENDERER_HPP__
#define __HIT_RENDERER_HPP__

#include <DataTypes.hpp>
#include <vulkan/vulkan.h>
#include <vector>
#include <xcb/xcb.h>

namespace Hit
{
	class GameWindow;

	class Renderer
	{
	public:
		Renderer( );
		~Renderer( );

		HIT_SINT32 Initialise( GameWindow *p_pGameWindow );

		void OnWindowSizeChanges( );

		HIT_BOOL OnGameWindowResized( );

		HIT_SINT32 CreateSwapchain( );

		void Render( );

		HIT_BOOL CanRender( ) const;

		HIT_SINT32 CreateCommandBuffers( );

		static const HIT_SINT32 OK = 0;
		static const HIT_SINT32 LOAD_LIBRARY_FAILED = -100;
		static const HIT_SINT32 LOAD_EXPORTED_ENTRY_POINTS_FAILED = -101;
		static const HIT_SINT32 LOAD_GLOBAL_ENTRY_POINTS_FAILED = -102;
		static const HIT_SINT32 LOAD_INSTANCE_ENTRY_POINTS_FAILED = -103;
		static const HIT_SINT32 LOAD_DEVICE_ENTRY_POINTS_FAILED = -104;
		static const HIT_SINT32 CREATE_VULKAN_INSTANCE_FAILED = -1000;
		static const HIT_SINT32 CREATE_VULKAN_DEVICE_FAILED = -1001;
		static const HIT_SINT32 CREATE_PRESENTATION_SURFACE_FAILED = -1002;
		static const HIT_SINT32 CREATE_SEMAPHORES_FAILED = -1005;
		static const HIT_SINT32 CREATE_SWAPCHAIN_FAILED = -1100;
		static const HIT_SINT32 CREATE_COMMAND_BUFFERS_FAILED = -1200;
		static const HIT_SINT32 RECORD_COMMAND_BUFFERS_FAILED = -1300;
		static const HIT_SINT32 PHYSICAL_DEVICE_ENUMERATE_FAILED = -3000;
		static const HIT_SINT32 PHYSICAL_DEVICE_SELECT_FAILED = -3001;
		static const HIT_SINT32 GET_VULKAN_DEVICE_QUEUE_FAILED = -5000;

	private:
		xcb_connection_t	*m_pXCBConnection;
		xcb_window_t		m_XCBWindow;

		HIT_SINT32 LoadVulkanLibrary( );
		HIT_SINT32 LoadExportedEntryPoints( );
		HIT_SINT32 LoadGlobalLevelEntryPoints( );
		HIT_SINT32 CreateVulkanInstance( );
		HIT_SINT32 LoadInstanceLevelEntryPoints( );
		HIT_SINT32 CreateVulkanDevice( );
		HIT_SINT32 LoadDeviceLevelEntryPoints( );
		HIT_SINT32 GetDeviceQueue( );
		HIT_SINT32 CreatePresentationSurface( );
		HIT_SINT32 CreateSemaphores( );
		HIT_SINT32 RecordCommandBuffers( );
		void Clear( );

		uint32_t GetSwapchainImageCount(
			VkSurfaceCapabilitiesKHR &p_SurfaceCapabilities );
		VkSurfaceFormatKHR GetSwapchainFormat(
			std::vector< VkSurfaceFormatKHR > &p_SurfaceFormats );
		VkExtent2D GetSwapchainExtent(
			VkSurfaceCapabilitiesKHR &p_SurfaceCapabilities );
		VkImageUsageFlags GetSwapchainUsageFlags(
			VkSurfaceCapabilitiesKHR &p_SurfaceCapabilities );
		VkSurfaceTransformFlagBitsKHR GetSwapchainTransformFlagBits(
			VkSurfaceCapabilitiesKHR &p_SurfaceCapabilities );
		VkPresentModeKHR GetSwapchainPresentMode(
			std::vector< VkPresentModeKHR > &p_PresentModes );

		HIT_BOOL CheckPhysicalDeviceProperties(
			VkPhysicalDevice p_PhysicalDevice,
			uint32_t &p_GraphicsQueueFamilyIndex,
			uint32_t &p_PresentQueueFamilyIndex );
		HIT_BOOL CheckExtensionAvailability( const char *p_pExtensionName,
			const std::vector< VkExtensionProperties > &p_Extensions );

		void							*m_pVulkanLibraryHandle;
		VkInstance						m_VulkanInstance;
		VkPhysicalDevice				m_VulkanPhysicalDevice;
		VkDevice						m_VulkanDevice;
		VkQueue							m_GraphicsQueue;
		VkQueue							m_PresentQueue;
		uint32_t						m_GraphicsQueueFamilyIndex;
		uint32_t						m_PresentQueueFamilyIndex;
		VkSurfaceKHR					m_PresentationSurface;
		VkSwapchainKHR					m_Swapchain;
		std::vector< VkCommandBuffer >	m_PresentQueueCommandBuffers;
		VkCommandPool					m_PresentQueueCommandPool;
		VkSemaphore						m_ImageAvailableSemaphore;
		VkSemaphore						m_RenderingFinishedSemaphore;
		HIT_BOOL						m_CanRender;
	};
}

#endif // __HIT_RENDERER_HPP__

