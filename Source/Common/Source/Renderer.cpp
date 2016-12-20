#include <Renderer.hpp>
#include <dlfcn.h>
#include <iostream>
#include <VulkanFunctions.hpp>
#include <vector>

namespace Hit
{
	Renderer::Renderer( ) :
		m_pVulkanLibraryHandle( nullptr ),
		m_VulkanInstance( VK_NULL_HANDLE ),
		m_VulkanDevice( VK_NULL_HANDLE ),
		m_QueueFamilyIndex( 0UL )
	{
	}

	Renderer::~Renderer( )
	{
		if( m_VulkanDevice != VK_NULL_HANDLE )
		{
			if( vkDeviceWaitIdle && vkDestroyDevice )
			{
				vkDeviceWaitIdle( m_VulkanDevice );
				vkDestroyDevice( m_VulkanDevice, nullptr );
			}
			else
			{
				if( vkDeviceWaitIdle == nullptr )
				{
					std::cout << "[Hit::Renderer::~Renderer] <ERROR> Function "
						"\"vkDeviceWaitIdle\" does not exist" << std::endl;
				}

				if( vkDestroyDevice == nullptr )
				{
					std::cout << "[Hit::Renderer::~Renderer] <ERROR> Function "
						"\"vkDestroyDevice\" does not exist" << std::endl;
				}
			}
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

	HIT_SINT32 Renderer::Initialise( )
	{
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
			0,
			nullptr
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

		VkPhysicalDevice PhysicalDevice = VK_NULL_HANDLE;
		uint32_t QueueFamilyIndex = UINT32_MAX;

		for( uint32_t Index = 0; Index < DeviceCount; ++Index )
		{
			if( CheckPhysicalDeviceProperties( PhysicalDevices[ Index ],
				QueueFamilyIndex ) )
			{
				PhysicalDevice = PhysicalDevices[ Index ];
			}
		}

		if( PhysicalDevice == VK_NULL_HANDLE )
		{
			std::cout << "[Hit::Renderer::CreateVulkanDevice] <ERROR> "
				"Failed to select a physical device" << std::endl;

			return PHYSICAL_DEVICE_SELECT_FAILED;
		}

		VkPhysicalDeviceProperties DeviceProperties;
		vkGetPhysicalDeviceProperties( PhysicalDevice, &DeviceProperties );

		std::cout << "[Hit::Renderer::CreateVulkanDevice] <INFO> "
			"Using physical device \"" << DeviceProperties.deviceName <<
			"\"" << std::endl;

		std::vector< float > QueuePriorities = { 1.0f };

		VkDeviceQueueCreateInfo DeviceQueueCreateInfo =
		{
			VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			nullptr,
			0,
			QueueFamilyIndex,
			static_cast< uint32_t >( QueuePriorities.size( ) ),
			&QueuePriorities[ 0 ]
		};

		VkDeviceCreateInfo DeviceCreateInfo =
		{
			VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			nullptr,
			0,
			1,
			&DeviceQueueCreateInfo,
			0,
			nullptr,
			0,
			nullptr,
			nullptr
		};

		if( vkCreateDevice( PhysicalDevice, &DeviceCreateInfo, nullptr,
			&m_VulkanDevice ) != VK_SUCCESS )
		{
			std::cout << "[Hit::Renderer::CreateVulkanDevice] <ERROR> "
				"Failed to create vulkan device" << std::endl;

			return CREATE_VULKAN_DEVICE_FAILED;
		}

		m_QueueFamilyIndex = QueueFamilyIndex;

		return OK;
	}

	HIT_BOOL Renderer::CheckPhysicalDeviceProperties(
		VkPhysicalDevice p_PhysicalDevice, uint32_t &p_QueueFamilyIndex )
	{
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

		vkGetPhysicalDeviceQueueFamilyProperties( p_PhysicalDevice,
			&QueueFamiliesCount, &QueueFamilyProperties[ 0 ] );

		for( uint32_t Index = 0; Index < QueueFamiliesCount; ++Index )
		{
			if( ( QueueFamilyProperties[ Index ].queueCount > 0 ) &&
				( QueueFamilyProperties[ Index ].queueFlags &
					VK_QUEUE_GRAPHICS_BIT ) )
			{
				p_QueueFamilyIndex = Index;

				return HIT_TRUE;
			}
		}

		std::cout << "[Hit::Renderer::CheckPhysicalDeviceProperties] <WARN> "
			"Could not find queue family with required properties on physical "
			"device \"" << p_PhysicalDevice << "\"" << std::endl;

		return HIT_FALSE;
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
}

