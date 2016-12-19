#include <InputDevice.hpp>

namespace Hit
{
	InputDevice::InputDevice( const HIT_UINT32 p_ID ) :
		m_Connected( HIT_FALSE ),
		m_StateSize( 0 ),
		m_DeviceType( INPUT_DEVICE_TYPE_UNKNOWN ),
		m_ID( p_ID )
#if defined ( HIT_PLATFORM_LINUX )
		,m_EventID( -1 ),
		m_FileHandle( -1 )
#endif // HIT_PLATFORM_LINUX
	{
#if defined ( HIT_PLATFORM_LINUX )
		FD_ZERO( &m_ReadFDS );
#endif // HIT_PLATFORM_LINUX
	}

	InputDevice::~InputDevice( )
	{
	}

	HIT_MEMSIZE InputDevice::GetStateSize( ) const
	{
		return m_StateSize;
	}

	HIT_BOOL InputDevice::IsConnected( ) const
	{
		return m_Connected;
	}

	INPUT_DEVICE_TYPE InputDevice::GetType( ) const
	{
		return m_DeviceType;
	}

	HIT_UINT32 InputDevice::GetID( ) const
	{
		return m_ID;
	}
}

