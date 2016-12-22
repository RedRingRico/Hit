#include <Renderer.hpp>
#include <dlfcn.h>
#include <iostream>
#include <VulkanFunctions.hpp>
#include <vector>
#include <cstring>
#include <GameWindowXCB.hpp>

namespace Hit
{
	Renderer::Renderer( ) :
		m_pVulkanLibraryHandle( nullptr ),
		m_VulkanInstance( VK_NULL_HANDLE ),
		m_VulkanPhysicalDevice( VK_NULL_HANDLE ),
		m_VulkanDevice( VK_NULL_HANDLE ),
		m_GraphicsQueue( VK_NULL_HANDLE ),
		m_PresentQueue( VK_NULL_HANDLE ),
		m_GraphicsQueueFamilyIndex( 0UL ),
		m_PresentQueueFamilyIndex( 0UL ),
		m_PresentationSurface( VK_NULL_HANDLE ),
		m_Swapchain( VK_NULL_HANDLE ),
		m_PresentQueueCommandPool( VK_NULL_HANDLE ),
		m_ImageAvailableSemaphore( VK_NULL_HANDLE ),
		m_RenderingFinishedSemaphore( VK_NULL_HANDLE ),
		m_CanRender( HIT_FALSE )
	{
	}

	Renderer::~Renderer( )
	{
		if( m_VulkanDevice != VK_NULL_HANDLE )
		{
			vkDeviceWaitIdle( m_VulkanDevice );

			if( m_Swapchain != VK_NULL_HANDLE )
			{
				vkDestroySwapchainKHR( m_VulkanDevice, m_Swapchain, nullptr );
			}

			if( m_RenderingFinishedSemaphore != VK_NULL_HANDLE )
			{
				vkDestroySemaphore( m_VulkanDevice,
					m_RenderingFinishedSemaphore, nullptr );
			}

			if( m_ImageAvailableSemaphore != VK_NULL_HANDLE )
			{
				vkDestroySemaphore( m_VulkanDevice, m_ImageAvailableSemaphore,
					nullptr );
			}

			vkDestroyDevice( m_VulkanDevice, nullptr );
		}

		if( m_PresentationSurface != VK_NULL_HANDLE )
		{
			vkDestroySurfaceKHR( m_VulkanInstance, m_PresentationSurface,
				nullptr );
		}

		if( m_VulkanInstance != VK_NULL_HANDLE )
		{
			if( vkDestroyInstance )
			{
				vkDestroyInstance( m_VulkanInstance, nullptr );
			}
			else
			{
				std::cout << "[Hit::Renderer::~Renderer] <ERROR> Failed to "
					"load the \"vkDestroyInstance\" function" << std::endl;
			}
		}

		if( m_pVulkanLibraryHandle )
		{
			dlclose( m_pVulkanLibraryHandle );
		}
	}

	HIT_SINT32 Renderer::Initialise( GameWindow *p_pGameWindow )
	{
		GameWindowDataXCB *pWindowData =
			dynamic_cast< GameWindowDataXCB * >(
				p_pGameWindow->GetGameWindowData( ) );

		m_pXCBConnection = pWindowData->GetConnection( );
		m_XCBWindow = pWindowData->GetWindow( );

		if( this->LoadVulkanLibrary( ) != OK )
		{
			std::cout << "[Hit::Renderer::Initialise] <ERROR> Failed to load "
				"the Vulkan library" << std::endl;

			return LOAD_LIBRARY_FAILED;
		}

		if( this->LoadExportedEntryPoints( ) != OK )
		{
			std::cout << "[Hit::Renderer::Initialise] <ERROR> Failed to load "
				"the Vulkan exported entry points" << std::endl;
			
			return LOAD_EXPORTED_ENTRY_POINTS_FAILED;
		}

		if( this->LoadGlobalLevelEntryPoints( ) != OK )
		{
			std::cout << "[Hit::Renderer::Initialise] <ERROR> Failed to load "
				"the Vulkan global entry points" << std::endl;

			return LOAD_GLOBAL_ENTRY_POINTS_FAILED;
		}

		if( this->CreateVulkanInstance( ) != OK )
		{
			std::cout << "[Hit::Renderer::Initialise] <ERROR> Failed to "
				"create the Vulkan instance" << std::endl;

			return CREATE_VULKAN_INSTANCE_FAILED;
		}

		if( this->LoadInstanceLevelEntryPoints( ) != OK )
		{
			std::cout << "[Hit::Renderer::Initialise] <ERROR> Failed to load "
				"the Vulkan instance entry points" << std::endl;

			return LOAD_INSTANCE_ENTRY_POINTS_FAILED;
		}

		if( this->CreatePresentationSurface( ) != OK )
		{
			std::cout << "[Hit::Renderer::Initialise] <ERROR> Failed to ceate "
				"the Vulkan presentation surface" << std::endl;

			return CREATE_PRESENTATION_SURFACE_FAILED;
		}

		if( this->CreateVulkanDevice( ) != OK )
		{
			std::cout << "[Hit::Renderer::Initialise] <ERROR> Failed to "
				"create the Vulkan device" << std::endl;

			return CREATE_VULKAN_DEVICE_FAILED;
		}

		if( this->LoadDeviceLevelEntryPoints( ) != OK )
		{
			std::cout << "[Hit::Renderer::Initialies] <ERROR> Failed to load "
				"the Vulkan device entry points" << std::endl;

			return LOAD_DEVICE_ENTRY_POINTS_FAILED;
		}

		if( this->GetDeviceQueue( ) != OK )
		{
			std::cout << "[Hit::Renderer::Initialise] <ERROR> Failed to get "
				"the Vulkan device queue" << std::endl;

			return GET_VULKAN_DEVICE_QUEUE_FAILED;
		}

		if( this->CreateSemaphores( ) != OK )
		{
			std::cout << "[Hit::Renderer::Initialise] <ERROR> Failed to "
				"create semaphores" << std::endl;

			return CREATE_SEMAPHORES_FAILED;
		}

		p_pGameWindow->SetRenderer( this );

		return OK;
	}

	HIT_SINT32 Renderer::LoadVulkanLibrary( )
	{
		if( ( m_pVulkanLibraryHandle = dlopen( "libvulkan.so.1",
			RTLD_NOW ) ) == nullptr )
		{
			std::cout << "[Hit::Renderer::LoadVulkanLibrary] <ERROR> Could "
				"not load the library \"libvulkan.so.1\"" << std::endl;
			std::cout << "\t" << dlerror( ) << std::endl;

			return LOAD_LIBRARY_FAILED;
		}

		return OK;
	}

	HIT_SINT32 Renderer::LoadExportedEntryPoints( )
	{
#define VK_EXPORTED_FUNCTION( p_Function ) \
		if( !( p_Function = ( PFN_##p_Function )dlsym( m_pVulkanLibraryHandle,\
			#p_Function ) ) )\
		{\
			std::cout << "[Hit::Renderer::LoadExportedEntryPoints] <ERROR> "\
				"Could not load function: \"" << #p_Function << "\"" <<\
				std::endl;\
			return LOAD_EXPORTED_ENTRY_POINTS_FAILED;\
		}\
		else\
		{\
			std::cout << "[Hit::Renderer::LoadExportedEntryPoints] <INFO> "\
				"Loaded function \"" << #p_Function << "\"" << std::endl;\
		}

#include <VulkanFunctions.inl>

		return OK;
	}

	HIT_SINT32 Renderer::LoadGlobalLevelEntryPoints( )
	{
#define VK_GLOBAL_LEVEL_FUNCTION( p_Function ) \
		if( !( p_Function = ( PFN_##p_Function )vkGetInstanceProcAddr( \
			nullptr, #p_Function ) ) ) \
		{ \
			std::cout << "[Hit::Renderer::LoadGlobalLevelEntryPoints] "\
				"<ERROR> Failed to load fucntion: \"" << #p_Function << \
				"\"" << std::endl;\
			return LOAD_GLOBAL_ENTRY_POINTS_FAILED; \
		} \
		else \
		{ \
			std::cout << "[Hit::Renderer::LoadGlobalLevelEntryPoints] "\
				"<INFO> Loaded function \"" << #p_Function << "\"" << \
				std::endl; \
		}

#include <VulkanFunctions.inl>

		return OK;
	}

	HIT_SINT32 Renderer::CreateVulkanInstance( )
	{
		uint32_t ExtensionCount = 0;

		if( ( vkEnumerateInstanceExtensionProperties( nullptr, &ExtensionCount,
			nullptr ) != VK_SUCCESS ) || ( ExtensionCount == 0 ) )
		{
			std::cout << "[Hit::Renderer::CreateVulkanInstance] <ERROR> "
				"Unable to get the extension count" << std::endl;

			return CREATE_VULKAN_INSTANCE_FAILED;
		}

		std::vector< VkExtensionProperties > AvailableExtensions(
			ExtensionCount );

		if( vkEnumerateInstanceExtensionProperties( nullptr, &ExtensionCount,
			&AvailableExtensions[ 0 ] ) != VK_SUCCESS )
		{
			std::cout << "[Hit::Renderer::CreateVulkanInstance] <ERROR> "
				"Failed to enumerate the available extensions" << std::endl;

			return CREATE_VULKAN_INSTANCE_FAILED;
		}

		std::vector< const char * > Extensions =
		{
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_XCB_SURFACE_EXTENSION_NAME
		};

		for( size_t Index = 0; Index < Extensions.size( ); ++Index )
		{
			if( this->CheckExtensionAvailability( Extensions[ Index ],
				AvailableExtensions ) == HIT_FALSE )
			{
				std::cout << "[Hit::Renderer::CreateVulkanInstance] <ERROR> "
					"Extension \"" << Extensions[ Index ] <<
					"\" not available" << std::endl;

				return CREATE_VULKAN_INSTANCE_FAILED;
			}
		}

		VkApplicationInfo ApplicationInfo =
		{
			VK_STRUCTURE_TYPE_APPLICATION_INFO,
			nullptr,
			"Hit",
			VK_MAKE_VERSION( 1, 0, 0 ),
			"Hit",
			VK_MAKE_VERSION( 1, 0, 0 ),
			VK_MAKE_VERSION( 1, 0, 0 )
		};

		VkInstanceCreateInfo InstanceInfo =
		{
			VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			nullptr,
			0,
			&ApplicationInfo,
			0,
			nullptr,
			static_cast< uint32_t >( Extensions.size( ) ),
			&Extensions[ 0 ]
		};

		if( vkCreateInstance( &InstanceInfo, nullptr, &m_VulkanInstance ) !=
			VK_SUCCESS )
		{
			std::cout << "[Hit::Renderer::CreateVulkanInstance] <ERROR> "
				"Failed to create Vulkan instance" << std::endl;

			return CREATE_VULKAN_INSTANCE_FAILED;
		}

		return OK;
	}

	HIT_SINT32 Renderer::LoadInstanceLevelEntryPoints( )
	{
#define VK_INSTANCE_LEVEL_FUNCTION( p_Function ) \
		if( !( p_Function = ( PFN_##p_Function )vkGetInstanceProcAddr( \
			m_VulkanInstance, #p_Function ) ) ) \
		{ \
			std::cout << "[Hit::Renderer::LoadInstanceLevelEntryPoints] " \
				"<ERROR> Failed to load function \"" << #p_Function << \
				"\"" << std::endl; \
			return LOAD_INSTANCE_ENTRY_POINTS_FAILED; \
		} \
		else \
		{ \
			std::cout << "[Hit::Renderer::LoadInstanceLevelEntryPOints] " \
				"<INFO> Loaded function \"" << #p_Function << "\"" << \
				std::endl; \
		}
	
#include <VulkanFunctions.inl>

		return OK;
	}

	HIT_SINT32 Renderer::CreateVulkanDevice( )
	{
		uint32_t DeviceCount = 0;

		if( ( vkEnumeratePhysicalDevices( m_VulkanInstance, &DeviceCount,
			nullptr ) != VK_SUCCESS ) || ( DeviceCount == 0 ) )
		{
			std::cout << "[Hit::Renderer::CreateVulkanDevice] <ERROR> Failed "
				"to enumerate physical devices" << std::endl;

			return PHYSICAL_DEVICE_ENUMERATE_FAILED;
		}

		std::cout << "[Hit::Renderer::CreateVulkanDevice] <INFO> Found " <<
			DeviceCount << ( DeviceCount == 1 ? " device" : " devices" ) <<
			std::endl;

		std::vector< VkPhysicalDevice > PhysicalDevices( DeviceCount );

		if( vkEnumeratePhysicalDevices( m_VulkanInstance, &DeviceCount,
			&PhysicalDevices[ 0 ] ) != VK_SUCCESS )
		{
			std::cout << "[Hit::Renderer::CreateVulkanDevice] <ERROR> Failed "
				"to enumerate physical devices" << std::endl;

			return PHYSICAL_DEVICE_ENUMERATE_FAILED;
		}

		uint32_t GraphicsQueueFamilyIndex = UINT32_MAX;
		uint32_t PresentQueueFamilyIndex = UINT32_MAX;

		for( uint32_t Index = 0; Index < DeviceCount; ++Index )
		{
			if( CheckPhysicalDeviceProperties( PhysicalDevices[ Index ],
				GraphicsQueueFamilyIndex, PresentQueueFamilyIndex ) )
			{
				m_VulkanPhysicalDevice = PhysicalDevices[ Index ];
			}
		}

		if( m_VulkanPhysicalDevice == VK_NULL_HANDLE )
		{
			std::cout << "[Hit::Renderer::CreateVulkanDevice] <ERROR> "
				"Failed to select a physical device" << std::endl;

			return PHYSICAL_DEVICE_SELECT_FAILED;
		}

		VkPhysicalDeviceProperties DeviceProperties;
		vkGetPhysicalDeviceProperties( m_VulkanPhysicalDevice,
			&DeviceProperties );

		std::cout << "[Hit::Renderer::CreateVulkanDevice] <INFO> "
			"Using physical device \"" << DeviceProperties.deviceName <<
			"\"" << std::endl;

		std::vector< VkDeviceQueueCreateInfo > QueueCreateInfos;
		std::vector< float > QueuePriorities = { 1.0f };

		QueueCreateInfos.push_back(
			{
				VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				nullptr,
				0,
				GraphicsQueueFamilyIndex,
				static_cast< uint32_t >( QueuePriorities.size( ) ),
				&QueuePriorities[ 0 ]
			} );
		
		if( GraphicsQueueFamilyIndex != PresentQueueFamilyIndex )
		{
			QueueCreateInfos.push_back(
				{
					VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
					nullptr,
					0,
					PresentQueueFamilyIndex,
					static_cast< uint32_t >( QueuePriorities.size( ) ),
					&QueuePriorities[ 0 ]
				} );
		}

		std::vector< const char * > DeviceExtensions =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		VkDeviceCreateInfo DeviceCreateInfo =
		{
			VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			nullptr,
			0,
			static_cast< uint32_t >( QueueCreateInfos.size( ) ),
			&QueueCreateInfos[ 0 ],
			0,
			nullptr,
			static_cast< uint32_t >( DeviceExtensions.size( ) ),
			&DeviceExtensions[ 0 ],
			nullptr
		};

		if( vkCreateDevice( m_VulkanPhysicalDevice, &DeviceCreateInfo, nullptr,
			&m_VulkanDevice ) != VK_SUCCESS )
		{
			std::cout << "[Hit::Renderer::CreateVulkanDevice] <ERROR> "
				"Failed to create vulkan device" << std::endl;

			return CREATE_VULKAN_DEVICE_FAILED;
		}

		m_GraphicsQueueFamilyIndex = GraphicsQueueFamilyIndex;
		m_PresentQueueFamilyIndex = PresentQueueFamilyIndex;

		return OK;
	}

	HIT_BOOL Renderer::CheckPhysicalDeviceProperties(
		VkPhysicalDevice p_PhysicalDevice,
		uint32_t &p_GraphicsQueueFamilyIndex,
		uint32_t &p_PresentQueueFamilyIndex )
	{
		uint32_t ExtensionCount = 0;

		if( ( vkEnumerateDeviceExtensionProperties( p_PhysicalDevice, nullptr,
			&ExtensionCount, nullptr ) != VK_SUCCESS ) ||
			( ExtensionCount == 0 ) )
		{
			std::cout << "[Hit::Renderer::CheckPhysicalDeviceProperties] "
				"<ERROR> Failed to get the extension count for the device" <<
				std::endl;

			return HIT_FALSE;
		}

		std::vector< VkExtensionProperties > AvailableExtensions(
			ExtensionCount );

		if( vkEnumerateDeviceExtensionProperties( p_PhysicalDevice, nullptr,
			&ExtensionCount, &AvailableExtensions[ 0 ] ) != VK_SUCCESS )
		{
			std::cout << "[Hit::Renderer::CheckPhysicalDeviceProperties] "
				"<ERROR> Failed to enumerate the device's extensions" <<
				std::endl;
			
			return HIT_FALSE;
		}

		std::vector< const char * > DeviceExtensions =
		{
			VK_KHR_SWAPCHAIN_EXTENSION_NAME
		};

		for( size_t Index = 0; Index < DeviceExtensions.size( ); ++Index )
		{
			if( this->CheckExtensionAvailability( DeviceExtensions[ Index ],
				AvailableExtensions ) == HIT_FALSE )
			{
				std::cout << "[Hit::Renderer::CheckPhysicalDeviceProperties] "
					"<ERROR> Device extension \"" << DeviceExtensions[ Index ]
					<< "\" not available" << std::endl;

				return HIT_FALSE;
			}
		}

		VkPhysicalDeviceProperties DeviceProperties;
		VkPhysicalDeviceFeatures DeviceFeatures;

		vkGetPhysicalDeviceProperties( p_PhysicalDevice, &DeviceProperties );
		vkGetPhysicalDeviceFeatures( p_PhysicalDevice, &DeviceFeatures );

		uint32_t MajorVersion =
			VK_VERSION_MAJOR( DeviceProperties.apiVersion );

		if( ( MajorVersion < 1 ) ||
			( DeviceProperties.limits.maxImageDimension2D < 4096 ) )
		{
			std::cout << "[Hit::Renderer::CheckPhysicalDeviceProperties] "
				"<ERROR> Physical device \"" << p_PhysicalDevice <<
				"\" does not support reqired parameters" << std::endl;

			return HIT_FALSE;
		}

		uint32_t QueueFamiliesCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties( p_PhysicalDevice,
			&QueueFamiliesCount, nullptr );

		if( QueueFamiliesCount == 0 )
		{
			std::cout << "[Hit::Renderer::CheckPhysicalDeviceProperties] "
				"<ERROR> Physical device \"" << p_PhysicalDevice <<
				"\" does not have any queue families" << std::endl;

			return HIT_FALSE;
		}

		std::vector< VkQueueFamilyProperties > QueueFamilyProperties(
			QueueFamiliesCount );
		std::vector< VkBool32 > QueuePresentSupport( QueueFamiliesCount );

		vkGetPhysicalDeviceQueueFamilyProperties( p_PhysicalDevice,
			&QueueFamiliesCount, &QueueFamilyProperties[ 0 ] );

		uint32_t GraphicsQueueFamilyIndex = UINT32_MAX;
		uint32_t PresentQueueFamilyIndex = UINT32_MAX;

		for( uint32_t Index = 0; Index < QueueFamiliesCount; ++Index )
		{
			vkGetPhysicalDeviceSurfaceSupportKHR( p_PhysicalDevice,
				Index, m_PresentationSurface, &QueuePresentSupport[ Index ] );

			if( ( QueueFamilyProperties[ Index ].queueCount > 0 ) &&
				( QueueFamilyProperties[ Index ].queueFlags &
					VK_QUEUE_GRAPHICS_BIT ) )
			{
				if( GraphicsQueueFamilyIndex == UINT32_MAX )
				{
					GraphicsQueueFamilyIndex = Index;
				}

				if( QueuePresentSupport[ Index ] )
				{
					p_GraphicsQueueFamilyIndex = Index;
					p_PresentQueueFamilyIndex = Index;

					return HIT_TRUE;
				}
			}
		}

		for( uint32_t Index = 0; Index < QueueFamiliesCount; ++Index )
		{
			if( QueuePresentSupport[ Index ] )
			{
				PresentQueueFamilyIndex = Index;
				break;
			}
		}

		if( ( PresentQueueFamilyIndex == UINT32_MAX ) ||
			( GraphicsQueueFamilyIndex == UINT32_MAX ) )
		{
			std::cout << "[Hit::Renderer::CheckPhysicalDeviceProperties] "
				"<ERROR> Could not find queue family with required properties "
				"on physical device \"" << p_PhysicalDevice << "\"" <<
				std::endl;

			return HIT_FALSE;
		}

		p_GraphicsQueueFamilyIndex = GraphicsQueueFamilyIndex;
		p_PresentQueueFamilyIndex = PresentQueueFamilyIndex;

		return HIT_TRUE;
	}

	HIT_SINT32 Renderer::LoadDeviceLevelEntryPoints( )
	{
#define VK_DEVICE_LEVEL_FUNCTION( p_Function ) \
		if( !( p_Function = ( PFN_##p_Function )vkGetDeviceProcAddr( \
			m_VulkanDevice, #p_Function ) ) ) \
		{ \
			std::cout << "[Hit::Renderer::LoadDeviceLevelEntryPoints] " \
				"<ERROR> Failed to load function: \"" << #p_Function << "\"" \
				<< std::endl; \
			return LOAD_DEVICE_ENTRY_POINTS_FAILED; \
		} \
		else \
		{ \
			std::cout << "[Hit::Renderer::LoadDeviceLevelEntryPoints] " \
				"<INFO> Loaded function \"" << #p_Function << "\"" << \
				std::endl; \
		}

#include <VulkanFunctions.inl>

		return OK;
	}

	HIT_SINT32 Renderer::GetDeviceQueue( )
	{
		vkGetDeviceQueue( m_VulkanDevice, m_GraphicsQueueFamilyIndex, 0,
			&m_GraphicsQueue );
		vkGetDeviceQueue( m_VulkanDevice, m_PresentQueueFamilyIndex, 0,
			&m_PresentQueue );

		if( m_GraphicsQueue == VK_NULL_HANDLE )
		{
			std::cout << "[Hit::Renderer::GetDeviceQueue] <ERROR> Something "
				"went wrong acquiring the graphics queue" << std::endl;

			return GET_VULKAN_DEVICE_QUEUE_FAILED;
		}

		if( m_PresentQueue == VK_NULL_HANDLE )
		{
			std::cout << "[Hit::Renderer::GetDeviceQueue] <ERROR> Something "
				"went wrong acquiring the present queue" << std::endl;

			return GET_VULKAN_DEVICE_QUEUE_FAILED;
		}

		return OK;
	}

	HIT_BOOL Renderer::CheckExtensionAvailability(
		const char *p_pExtensionName,
		const std::vector< VkExtensionProperties > &p_Extensions )
	{
		for( size_t Index = 0; Index < p_Extensions.size( ); ++Index )
		{
			if( strcmp( p_Extensions[ Index ].extensionName,
				p_pExtensionName ) == 0 )
			{
				return HIT_TRUE;
			}
		}

		return HIT_FALSE;
	}

	HIT_SINT32 Renderer::CreatePresentationSurface( )
	{
		VkXcbSurfaceCreateInfoKHR SurfaceCreateInfo =
		{
			VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR,
			nullptr,
			0,
			m_pXCBConnection,
			m_XCBWindow
		};

		if( vkCreateXcbSurfaceKHR( m_VulkanInstance, &SurfaceCreateInfo,
			nullptr, &m_PresentationSurface ) == VK_SUCCESS )
		{
			return OK;
		}

		std::cout << "[Hit::Renderer::CreatePresentationSurface] <ERROR> "
			"Failed to create the presentation surface" << std::endl;

		return CREATE_PRESENTATION_SURFACE_FAILED;
	}

	HIT_SINT32 Renderer::CreateSemaphores( )
	{
		VkSemaphoreCreateInfo SemaphoreCreateInfo =
		{
			VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			nullptr,
			0
		};

		if( ( vkCreateSemaphore( m_VulkanDevice, &SemaphoreCreateInfo, nullptr,
				&m_ImageAvailableSemaphore ) != VK_SUCCESS ) ||
			( vkCreateSemaphore( m_VulkanDevice, &SemaphoreCreateInfo, nullptr,
				&m_RenderingFinishedSemaphore ) != VK_SUCCESS ) )
		{
			std::cout << "[Hit::Renderer::CreateSemaphores] <ERROR> "
				"Failed to create semaphores" << std::endl;

			return CREATE_SEMAPHORES_FAILED;
		}

		return OK;
	}

	HIT_BOOL Renderer::OnGameWindowResized( )
	{
		this->Clear( );

		if( this->CreateSwapchain( ) != OK )
		{
			std::cout << "[Hit::Renderer::OnGameWindowResized] <ERROR> "
				"Failed to create the swapchain" << std::endl;

			return HIT_FALSE;
		}

		if( this->CreateCommandBuffers( ) != OK )
		{
			std::cout << "[Hit::Renderer::OnGameWindowResized] <ERROR> "
				"Could not create the command buffers" << std::endl;

			return HIT_FALSE;
		}

		return HIT_TRUE;
	}

	HIT_SINT32 Renderer::CreateSwapchain( )
	{
		m_CanRender = HIT_FALSE;

		if( m_VulkanDevice != VK_NULL_HANDLE )
		{
			vkDeviceWaitIdle( m_VulkanDevice );
		}

		VkSurfaceCapabilitiesKHR SurfaceCapabilities;

		if( vkGetPhysicalDeviceSurfaceCapabilitiesKHR( m_VulkanPhysicalDevice,
			m_PresentationSurface, &SurfaceCapabilities ) != VK_SUCCESS )
		{
			std::cout << "[Hit::Renderer::CreateSwpachain] <ERROR> "
				"Unable to check the surface presentation capabilities" <<
				std::endl;

			return CREATE_SWAPCHAIN_FAILED;
		}

		uint32_t FormatCount;

		if( ( vkGetPhysicalDeviceSurfaceFormatsKHR( m_VulkanPhysicalDevice,
			m_PresentationSurface, &FormatCount, nullptr ) != VK_SUCCESS ) ||
			( FormatCount == 0 ) )
		{
			std::cout << "[Hit::Renderer::CreateSwapchain] <ERROR> "
				"Could not get the number of surface formats" << std::endl;

			return CREATE_SWAPCHAIN_FAILED;
		}

		std::vector< VkSurfaceFormatKHR > SurfaceFormats( FormatCount );

		if( vkGetPhysicalDeviceSurfaceFormatsKHR( m_VulkanPhysicalDevice,
			m_PresentationSurface, &FormatCount, &SurfaceFormats[ 0 ] ) !=
			VK_SUCCESS )
		{
			std::cout << "[Hit::Renderer::CreateSwapchain] <ERROR> "
				"Failed to enumerate the surface formats" << std::endl;

			return CREATE_SWAPCHAIN_FAILED;
		}

		uint32_t PresentModesCount;

		if( ( vkGetPhysicalDeviceSurfacePresentModesKHR(
				m_VulkanPhysicalDevice, m_PresentationSurface,
				&PresentModesCount, nullptr ) != VK_SUCCESS ) ||
			( PresentModesCount == 0 ) )
		{
			std::cout << "[Hit::Renderer::CreateSwapchain] <ERROR> "
				"Failed to get the number of present modes" << std::endl;

			return CREATE_SWAPCHAIN_FAILED;
		}

		std::vector< VkPresentModeKHR > PresentModes( PresentModesCount );

		if( vkGetPhysicalDeviceSurfacePresentModesKHR( m_VulkanPhysicalDevice,
			m_PresentationSurface, &PresentModesCount, &PresentModes[ 0 ] ) !=
			VK_SUCCESS )
		{
			std::cout << "[Hit::Renderer::CreateSwapchain] <ERROR> "
				"Faeild to enumerate the present modes" << std::endl;

			return CREATE_SWAPCHAIN_FAILED;
		}

		uint32_t ImageCount = this->GetSwapchainImageCount(
			SurfaceCapabilities );
		VkSurfaceFormatKHR SurfaceFormat = this->GetSwapchainFormat(
			SurfaceFormats );
		VkExtent2D Extent = this->GetSwapchainExtent( SurfaceCapabilities );
		VkImageUsageFlags ImageUsageFlags = this->GetSwapchainUsageFlags(
			SurfaceCapabilities );
		VkSurfaceTransformFlagBitsKHR TransformFlagBits =
			this->GetSwapchainTransformFlagBits( SurfaceCapabilities );
		VkPresentModeKHR PresentMode = this->GetSwapchainPresentMode(
			PresentModes );
		VkSwapchainKHR OldSwapchain = m_Swapchain;

		if( static_cast< int >( ImageUsageFlags ) == -1 )
		{
			std::cout << "[Hit::Renderer::CreateSwapchain] <ERROR> "
				"Usage flags incorrect" << std::endl;

			return CREATE_SWAPCHAIN_FAILED;
		}

		if( static_cast< int >( PresentMode ) == -1 )
		{
			std::cout << "[Hit::Renderer::CreateSwapchain] <ERROR> "
				"Present mode incorrect" << std::endl;

			return CREATE_SWAPCHAIN_FAILED;
		}

		if( ( Extent.width == 0 ) || ( Extent.height == 0 ) )
		{
			std::cout << "[Hit::Renderer::CreateSwapchain] <INFO> "
				"Window minmised" << std::endl;

			return OK;
		}

		VkSwapchainCreateInfoKHR SwapchainCreateInfo =
		{
			VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			nullptr,
			0,
			m_PresentationSurface,
			ImageCount,
			SurfaceFormat.format,
			SurfaceFormat.colorSpace,
			Extent,
			1,
			ImageUsageFlags,
			VK_SHARING_MODE_EXCLUSIVE,
			0,
			nullptr,
			TransformFlagBits,
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			PresentMode,
			VK_TRUE,
			OldSwapchain
		};

		if( vkCreateSwapchainKHR( m_VulkanDevice, &SwapchainCreateInfo,
			nullptr, &m_Swapchain ) != VK_SUCCESS )
		{
			std::cout << "[Hit::Renderer::CreateSwapchain] <ERROR> "
				"Failed to create the swapchain" << std::endl;

			return CREATE_SWAPCHAIN_FAILED;
		}

		if( OldSwapchain != VK_NULL_HANDLE )
		{
			vkDestroySwapchainKHR( m_VulkanDevice, OldSwapchain, nullptr );
		}

		std::cout << "Created a " << Extent.width << "x" << Extent.height <<
			" swapchain" << std::endl;

		m_CanRender = HIT_TRUE;

		return OK;
	}

	uint32_t Renderer::GetSwapchainImageCount(
		VkSurfaceCapabilitiesKHR &p_SurfaceCapabilities )
	{
		uint32_t ImageCount = p_SurfaceCapabilities.minImageCount + 1;

		if( ( p_SurfaceCapabilities.maxImageCount > 0 ) &&
			( ImageCount > p_SurfaceCapabilities.maxImageCount ) )
		{
			ImageCount = p_SurfaceCapabilities.maxImageCount;
		}

		return ImageCount;
	}

	VkSurfaceFormatKHR Renderer::GetSwapchainFormat(
		std::vector< VkSurfaceFormatKHR > &p_SurfaceFormats )
	{
		if( ( p_SurfaceFormats.size( ) == 1 ) &&
			( p_SurfaceFormats[ 0 ].format == VK_FORMAT_UNDEFINED ) )
		{
			return { VK_FORMAT_R8G8B8A8_UNORM,
				VK_COLORSPACE_SRGB_NONLINEAR_KHR };
		}

		for( auto &SurfaceFormat : p_SurfaceFormats )
		{
			if( SurfaceFormat.format == VK_FORMAT_R8G8B8A8_UNORM )
			{
				return SurfaceFormat;
			}
		}

		return p_SurfaceFormats[ 0 ];
	}

	VkExtent2D Renderer::GetSwapchainExtent(
		VkSurfaceCapabilitiesKHR &p_SurfaceCapabilities )
	{
		if( p_SurfaceCapabilities.currentExtent.width == -1 )
		{
			VkExtent2D SwapchainExtent = { 640, 480 };

			if( SwapchainExtent.width <
				p_SurfaceCapabilities.minImageExtent.width )
			{
				SwapchainExtent.width =
					p_SurfaceCapabilities.minImageExtent.width;
			}

			if( SwapchainExtent.height <
				p_SurfaceCapabilities.minImageExtent.height )
			{
				SwapchainExtent.height =
					p_SurfaceCapabilities.minImageExtent.height;
			}

			if( SwapchainExtent.width >
				p_SurfaceCapabilities.maxImageExtent.width )
			{
				SwapchainExtent.width =
					p_SurfaceCapabilities.maxImageExtent.width;
			}

			if( SwapchainExtent.height >
				p_SurfaceCapabilities.maxImageExtent.height )
			{
				SwapchainExtent.height =
					p_SurfaceCapabilities.maxImageExtent.height;
			}

			return SwapchainExtent;
		}

		return p_SurfaceCapabilities.currentExtent;
	}

	VkImageUsageFlags Renderer::GetSwapchainUsageFlags(
		VkSurfaceCapabilitiesKHR &p_SurfaceCapabilities )
	{
		if( p_SurfaceCapabilities.supportedUsageFlags &
			VK_IMAGE_USAGE_TRANSFER_DST_BIT )
		{
			return VK_IMAGE_USAGE_TRANSFER_DST_BIT |
				VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		}

		std::cout << "[Hit::Renderer::GetSwapchainUsageFlags] <ERROR> "
			"VK_IMAGE_USAGE_TRANSFER_DST_BIT is not supported by the swapchain"
			<< std::endl;

		return static_cast< VkImageUsageFlags >( -1 );
	}

	VkSurfaceTransformFlagBitsKHR Renderer::GetSwapchainTransformFlagBits(
		VkSurfaceCapabilitiesKHR &p_SurfaceCapabilities )
	{
		if( p_SurfaceCapabilities.supportedTransforms &
			VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR )
		{
			return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		}
		
		return p_SurfaceCapabilities.currentTransform;
	}

	VkPresentModeKHR Renderer::GetSwapchainPresentMode(
		std::vector< VkPresentModeKHR > &p_PresentModes )
	{
		for( auto &PresentMode : p_PresentModes )
		{
			if( PresentMode == VK_PRESENT_MODE_MAILBOX_KHR )
			{
				return PresentMode;
			}
		}

		for( auto &PresentMode : p_PresentModes )
		{
			if( PresentMode == VK_PRESENT_MODE_FIFO_KHR )
			{
				return PresentMode;
			}
		}

		std::cout << "[Hit::Renderer::GetSwapchainPresentMode] <ERROR> "
			"No suitable present mode available" << std::endl;

		return static_cast< VkPresentModeKHR >( -1 );
	}

	void Renderer::Clear( )
	{
		if( m_VulkanDevice != VK_NULL_HANDLE )
		{
			vkDeviceWaitIdle( m_VulkanDevice );

			if( ( m_PresentQueueCommandBuffers.size( ) > 0 ) &&
				( m_PresentQueueCommandBuffers[ 0 ] != VK_NULL_HANDLE ) )
			{
				vkFreeCommandBuffers( m_VulkanDevice,
					m_PresentQueueCommandPool, static_cast< uint32_t >(
						m_PresentQueueCommandBuffers.size( ) ),
					&m_PresentQueueCommandBuffers[ 0 ] );

				m_PresentQueueCommandBuffers.clear( );
			}

			if( m_PresentQueueCommandPool != VK_NULL_HANDLE )
			{
				vkDestroyCommandPool( m_VulkanDevice,
					m_PresentQueueCommandPool, nullptr );

				m_PresentQueueCommandPool = VK_NULL_HANDLE;
			}
		}
	}

	void Renderer::Render( )
	{
		uint32_t ImageIndex;

		VkResult Result = vkAcquireNextImageKHR( m_VulkanDevice, m_Swapchain,
			UINT64_MAX, m_ImageAvailableSemaphore, VK_NULL_HANDLE,
			&ImageIndex );

		switch( Result )
		{
			case VK_SUCCESS:
			case VK_SUBOPTIMAL_KHR:
			{
				break;
			}
			case VK_ERROR_OUT_OF_DATE_KHR:
			{
				this->OnGameWindowResized( );
				return;
			}
			default:
			{
				std::cout << "[Hit::Renderer::Render] <ERROR> "
					"A problem occurred during swapchain acquisition" <<
					std::endl;
				return;
			}
		}
		
		VkPipelineStageFlags WaitDstStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;

		VkSubmitInfo SubmitInfo =
		{
			VK_STRUCTURE_TYPE_SUBMIT_INFO,
			nullptr,
			1,
			&m_ImageAvailableSemaphore,
			&WaitDstStageMask,
			1,
			&m_PresentQueueCommandBuffers[ ImageIndex ],
			1,
			&m_RenderingFinishedSemaphore
		};

		if( vkQueueSubmit( m_PresentQueue, 1, &SubmitInfo, VK_NULL_HANDLE ) !=
			VK_SUCCESS )
		{
			return;
		}

		VkPresentInfoKHR PresentInfo =
		{
			VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			nullptr,
			1,
			&m_RenderingFinishedSemaphore,
			1,
			&m_Swapchain,
			&ImageIndex,
			nullptr
		};

		Result = vkQueuePresentKHR( m_PresentQueue, &PresentInfo );

		switch( Result )
		{
			case VK_SUCCESS:
			{
				break;
			}
			case VK_ERROR_OUT_OF_DATE_KHR:
			case VK_SUBOPTIMAL_KHR:
			{
				this->OnGameWindowResized( );
				return;
			}
			default:
			{
				std::cout << "[Hit::Renderer::Render] <ERROR> "
					"A problem occurred during image presentation" <<
					std::endl;

				return;
			}
		}
	}

	HIT_BOOL Renderer::CanRender( ) const
	{
		return m_CanRender;
	}

	HIT_SINT32 Renderer::CreateCommandBuffers( )
	{
		VkCommandPoolCreateInfo CommandPoolCreateInfo =
		{
			VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			nullptr,
			0,
			m_PresentQueueFamilyIndex
		};

		if( vkCreateCommandPool( m_VulkanDevice, &CommandPoolCreateInfo,
			nullptr, &m_PresentQueueCommandPool ) != VK_SUCCESS )
		{
			std::cout << "[Hit::Renderer::CreateCommandBuffers] <ERROR> "
				"Failed to create a command pool" << std::endl;

			return CREATE_COMMAND_BUFFERS_FAILED;
		}

		uint32_t ImageCount = 0;

		if( ( vkGetSwapchainImagesKHR( m_VulkanDevice, m_Swapchain,
			&ImageCount, nullptr ) != VK_SUCCESS ) || ( ImageCount == 0 ) )
		{
			std::cout << "[Hit::Renderer::CreateCommandBuffers] <ERROR> "
				"Could not get the swap chain image count" << std::endl;

			return CREATE_COMMAND_BUFFERS_FAILED;
		}

		m_PresentQueueCommandBuffers.resize( ImageCount );

		VkCommandBufferAllocateInfo CommandBufferAllocateInfo =
		{
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			nullptr,
			m_PresentQueueCommandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			ImageCount
		};

		if( vkAllocateCommandBuffers( m_VulkanDevice,
			&CommandBufferAllocateInfo, &m_PresentQueueCommandBuffers[ 0 ] ) !=
			VK_SUCCESS )
		{
			std::cout << "[Hit::Rednerer::CreateCommandBuffers] <ERROR> "
				"Failed to allocate command buffers" << std::endl;

			return CREATE_COMMAND_BUFFERS_FAILED;
		}

		if( this->RecordCommandBuffers( ) != OK )
		{
			std::cout << "[Hit::Renderer::CreateCommandBuffers] <ERROR> "
				"Failed to record the command buffers" << std::endl;

			return RECORD_COMMAND_BUFFERS_FAILED;
		}

		return OK;
	}

	HIT_SINT32 Renderer::RecordCommandBuffers( )
	{
		uint32_t ImageCount =
			static_cast< uint32_t >( m_PresentQueueCommandBuffers.size( ) );

		std::vector< VkImage > SwapchainImages( ImageCount );

		if( vkGetSwapchainImagesKHR( m_VulkanDevice, m_Swapchain, &ImageCount,
			&SwapchainImages[ 0 ] ) != VK_SUCCESS )
		{
			std::cout << "[Hit::Renderer::RecordCommandBuffers] <ERROR> "
				"Failed to get the swapchain images" << std::endl;

			return RECORD_COMMAND_BUFFERS_FAILED;
		}

		VkCommandBufferBeginInfo CommandBufferBeginInfo =
		{
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			nullptr,
			VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
			nullptr
		};

		VkClearColorValue ClearColour =
		{
			{ 0.0f, 1.0f, 0.0f, 1.0f }
		};

		VkImageSubresourceRange ImageSubresourceRange =
		{
			VK_IMAGE_ASPECT_COLOR_BIT,
			0,
			1,
			0,
			1
		};

		for( uint32_t Index = 0; Index < ImageCount; ++Index )
		{
			VkImageMemoryBarrier BarrierFromPresentToClear =
			{
				VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				nullptr,
				VK_ACCESS_MEMORY_READ_BIT,
				VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_QUEUE_FAMILY_IGNORED,
				VK_QUEUE_FAMILY_IGNORED,
				SwapchainImages[ Index ],
				ImageSubresourceRange
			};

			VkImageMemoryBarrier BarrierFromClearToPresent =
			{
				VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				nullptr,
				VK_ACCESS_TRANSFER_WRITE_BIT,
				VK_ACCESS_MEMORY_READ_BIT,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				VK_QUEUE_FAMILY_IGNORED,
				VK_QUEUE_FAMILY_IGNORED,
				SwapchainImages[ Index ],
				ImageSubresourceRange
			};

			vkBeginCommandBuffer( m_PresentQueueCommandBuffers[ Index ],
				&CommandBufferBeginInfo );

			vkCmdPipelineBarrier( m_PresentQueueCommandBuffers[ Index ],
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				0, 0, nullptr, 0, nullptr, 1, &BarrierFromPresentToClear );

			vkCmdClearColorImage( m_PresentQueueCommandBuffers[ Index ],
				SwapchainImages[ Index ], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				&ClearColour, 1, &ImageSubresourceRange );

			vkCmdPipelineBarrier( m_PresentQueueCommandBuffers[ Index ],
				VK_PIPELINE_STAGE_TRANSFER_BIT,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0,
				nullptr, 1, &BarrierFromClearToPresent );

			if( vkEndCommandBuffer( m_PresentQueueCommandBuffers[ Index ] ) !=
				VK_SUCCESS )
			{
				std::cout << "[Hit::Renderer::RecordCommandBuffers] <ERROR> "
					"Failed to end the command buffer" << std::endl;

				return RECORD_COMMAND_BUFFERS_FAILED;
			}
		}

		return OK;
	}
}

