#ifndef __HIT_INPUTDEVICEMANAGER_HPP__
#define __HIT_INPUTDEVICEMANAGER_HPP__

#include <DataTypes.hpp>
#include <InputDevice.hpp>
#include <vector>

namespace Hit
{
	class Gamepad;

	typedef void ( *GamepadCallback )( Gamepad *p_pGamepad );
	typedef void ( *GamepadRemovedCallback )( HIT_UINT32 p_GamepadID );

	class InputDeviceManager
	{
	public:
		InputDeviceManager( );
		~InputDeviceManager( );

		HIT_SINT32 Initialise( );

		HIT_SINT32 Update( );

		void SetGamepadAddedCallback( GamepadCallback p_pCallback );
		void SetGamepadRemovedCallback( GamepadRemovedCallback p_pCallback );

#if defined( HIT_PLATFORM_LINUX )
		enum DEVICE_ADD_STATUS
		{
			DEVICE_ADD_STATUS_OK,
			DEVICE_ADD_STATUS_ERROR,
			DEVICE_ADD_STATUS_FINISHED
		};

		DEVICE_ADD_STATUS AddDevice( const char *p_pPath );
#endif // HIT_PLATFORM_LINUX

	private:
		HIT_BOOL IsDeviceConnected( const InputDevice &p_InputDevice );

		std::vector< InputDevice * >	m_InputDevices;
		HIT_UINT32						m_LatestID;
		HIT_MEMSIZE						m_GamepadCount;
		HIT_MEMSIZE						m_KeyboardCount;

		GamepadCallback			m_pGamepadAddedCallback;
		GamepadRemovedCallback	m_pGamepadRemovedCallback;

#if defined( HIT_PLATFORM_LINUX )

		int		m_MaxFileDescriptors;
#endif // HIT_PLATFORM_LINUX
	};
}

#endif // __HIT_INPUTDEVICEMANAGER_HPP__

