#ifndef __HIT_INPUTDEVICE_HPP__
#define __HIT_INPUTDEVICE_HPP__

#include <DataTypes.hpp>
#include <fcntl.h>

namespace Hit
{
	enum INPUT_DEVICE_TYPE
	{
		INPUT_DEVICE_TYPE_KEYBOARD,
		INPUT_DEVICE_TYPE_GAMEPAD,
		INPUT_DEVICE_TYPE_UNKNOWN
	};

	struct INPUT_DEVICE_PARAMETERS
	{
#if defined HIT_PLATFORM_LINUX
		int	Handle;
#endif // HIT_PLATFORM_LINUX
	};

	class InputDevice
	{
	public:
		explicit InputDevice( const HIT_UINT32 P_ID );
		virtual ~InputDevice( );

		virtual HIT_SINT32 Initialise(
			const INPUT_DEVICE_PARAMETERS &p_Parameters ) = 0;
		virtual void Terminate( ) = 0;

		virtual HIT_SINT32 GetState( void *p_pState ) = 0;
		HIT_MEMSIZE GetStateSize( ) const;

		virtual void Update( ) = 0;

		HIT_BOOL IsConnected( ) const;

		INPUT_DEVICE_TYPE GetType( ) const;

		HIT_UINT32 GetID( ) const;

#if defined ( HIT_PLATFORM_LINUX )
		int GetEventID( ) const;
#endif // HIT_PLATFORM_LINUX

	protected:
		HIT_BOOL			m_Connected;
		HIT_MEMSIZE			m_StateSize;
		INPUT_DEVICE_TYPE	m_DeviceType;
		HIT_UINT32			m_ID;
#if defined ( HIT_PLATFORM_LINUX )
		int		m_EventID;
		int		m_FileHandle;
		fd_set	m_ReadFDS;
#endif // HIT_PLATFORM_LINUX
	};
}

#endif // __HIT_INPUTDEVICE_HPP__

