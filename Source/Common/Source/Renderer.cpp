#include <Renderer.hpp>
#include <dlfcn.h>
#include <iostream>
#include <VulkanFunctions.hpp>

namespace Hit
{
	Renderer::Renderer( ) :
		m_pVulkanLibraryHandle( nullptr )
	{
	}

	Renderer::~Renderer( )
	{
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
			
			return LOAD_ENTRY_POINTS_FAILED;
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
			return LOAD_ENTRY_POINTS_FAILED;\
		}\
		else\
		{\
			std::cout << "[Hit::Renderer::LoadExportedEntryPoints] <INFO> "\
				"Loaded function \"" << #p_Function << "\"" << std::endl;\
		}
#include <VulkanFunctions.inl>
		return OK;
	}
}

