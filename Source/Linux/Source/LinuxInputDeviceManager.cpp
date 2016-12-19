#include <InputDeviceManager.hpp>
#include <linux/input.h>
#include <fcntl.h>
#include <sys/time.h>
#include <iostream>
#include <Input/Gamepad.hpp>
#include <cstring>
#include <unistd.h>
#include <sys/inotify.h>
#include <limits.h>
#include <pthread.h>

namespace Hit
{
	HIT_BOOL TestBit( uint32_t p_Bit, uint32_t p_Value )
	{
		return ( ( 1 << p_Bit ) & p_Value ) ? HIT_TRUE : HIT_FALSE;
	}

	HIT_BOOL TestBitArray( uint32_t p_Bit, uint8_t *p_pArray )
	{
		size_t Cell = p_Bit >> 3;
		size_t Bit = 1 << ( p_Bit % 8 );

		if( p_pArray[ Cell ] & Bit )
		{
			return HIT_TRUE;
		}

		return HIT_FALSE;
	}

	static void *CheckForNewEvents( void *p_pDeviceManager )
	{
		InputDeviceManager *pDeviceMan =
			( InputDeviceManager * )p_pDeviceManager;
		int INotify = inotify_init1( IN_NONBLOCK );
		inotify_add_watch( INotify, "/dev/input", IN_CREATE );
		size_t BufferLength = 10 * ( sizeof( struct inotify_event ) +
			NAME_MAX + 1 );

		char NotifyBuffer[ BufferLength ] __attribute( ( aligned( 8 ) ) );
		struct timespec SleepTime;
		SleepTime.tv_sec = 0;
		// 1ms sleep
		SleepTime.tv_nsec = 1000000;

		for( ; ; )
		{
			int Read = read( INotify, NotifyBuffer, BufferLength );

			if( Read > 0 )
			{
				for( char *p = NotifyBuffer; p < NotifyBuffer + Read; )
				{
					struct inotify_event *pEvent = ( struct inotify_event * )p;

					if( pEvent->len > 0 )
					{
						// If this is an "event[n]", see what type of device it
						// is and attempt to add it
						if( strncmp( "event", pEvent->name, 5 ) == 0 )
						{
							char DeviceName[ 64 ];
							snprintf( DeviceName, sizeof( DeviceName ),
								"/dev/input/%s", pEvent->name );

							// Give the event a little bit of time to register
							sleep( 1 );

							pDeviceMan->AddDevice( DeviceName );
						}
					}

					p += sizeof( struct inotify_event ) + pEvent->len;
				}
			}

			nanosleep( &SleepTime, nullptr );
		}
	}

	InputDeviceManager::InputDeviceManager( ) :
		m_LatestID( 0ULL ),
		m_GamepadCount( 0 ),
		m_KeyboardCount( 0 ),
		m_pGamepadAddedCallback( nullptr ),
		m_pGamepadRemovedCallback( nullptr )
	{
	}

	InputDeviceManager::~InputDeviceManager( )
	{
		auto InputDeviceItr = m_InputDevices.begin( );

		while( InputDeviceItr != m_InputDevices.end( ) )
		{
			delete ( *InputDeviceItr );

			++InputDeviceItr;
		}
	}

	HIT_SINT32 InputDeviceManager::Initialise( )
	{
		// Discover all connected peripherals
		for( int Device = 0; ; ++Device )
		{
			char DeviceName[ 64 ];
			snprintf( DeviceName, sizeof( DeviceName ), "/dev/input/event%i",
				Device );

			DEVICE_ADD_STATUS DeviceStatus = this->AddDevice( DeviceName );

			if( DeviceStatus == DEVICE_ADD_STATUS_FINISHED )
			{
				break;
			}

			if( DeviceStatus == DEVICE_ADD_STATUS_ERROR )
			{
				std::cout << "[Hit::InputDeviceManager::Initialise] <ERROR> "
					"Something went wrong detecting the devices" << std::endl;

				return -1;
			}
		}

		pthread_t NotifyThread;

		int ThreadCreate = pthread_create( &NotifyThread, nullptr,
			CheckForNewEvents, this );

		if( ThreadCreate != 0 )
		{
			std::cout << "[Hit::InputDeviceManager::Initialise] <ERROR> "
				"Failed to create a thread for the inotify events" <<
				std::endl;

			return -1;
		}

		return 0;
	}

	HIT_SINT32 InputDeviceManager::Update( )
	{
		// Remove any disconnected devices
		auto DeviceItr = m_InputDevices.begin( );

		while( DeviceItr != m_InputDevices.end( ) )
		{
			if( ( *DeviceItr )->IsConnected( ) == HIT_FALSE )
			{
				switch( ( *DeviceItr )->GetType( ) )
				{
					case INPUT_DEVICE_TYPE_GAMEPAD:
					{
						--m_GamepadCount;
						if( m_pGamepadRemovedCallback )
						{
							m_pGamepadRemovedCallback(
								( *DeviceItr )->GetID( ) );
						}
						break;
					}
					case INPUT_DEVICE_TYPE_KEYBOARD:
					{
						--m_KeyboardCount;
						break;
					}
					case INPUT_DEVICE_TYPE_UNKNOWN:
					{
						++DeviceItr;
						continue;
						break;
					}
				}

				m_InputDevices.erase( DeviceItr );
				delete ( *DeviceItr );
				if( m_InputDevices.size( ) == 0 )
				{
					DeviceItr = m_InputDevices.end( );
					continue;
				}
			}
			else
			{
				( *DeviceItr )->Update( );
			}

			++DeviceItr;
		}

		return 0;
	}

	HIT_BOOL InputDeviceManager::IsDeviceConnected(
		const InputDevice &p_InputDevice )
	{
		HIT_BOOL DeviceConnected = HIT_FALSE;

		return DeviceConnected;
	}

	InputDeviceManager::DEVICE_ADD_STATUS InputDeviceManager::AddDevice(
		const char *p_pPath )
	{
		// The keyboard and gamepad event numbers
		std::vector< int > Keyboards;
		std::vector< int > Gamepads;

		uint32_t DeviceTypes = 0U;

		const int DeviceHandle = open( p_pPath, O_RDONLY | O_NONBLOCK );

		if( DeviceHandle < 0 )
		{
			int EventID;
			char DeviceNumber[ 256 ];
			size_t Start = strlen( "/dev/input/event" );
			memcpy( DeviceNumber, p_pPath + Start, strlen( p_pPath ) - Start );
			EventID = atoi( DeviceNumber );

			// Something went wrong trying to open any devices
			if( EventID == 0 )
			{
				std::cout << "[Hit::InputDeviceManager::Update] <ERROR> "
					"Failed to open any devices" << std::endl;

				return DEVICE_ADD_STATUS_ERROR;
			}

			// Otherwise all devices have been discovered and added
			return DEVICE_ADD_STATUS_FINISHED;
		}

		if( DeviceHandle > m_MaxFileDescriptors )
		{
			m_MaxFileDescriptors = DeviceHandle;
		}

		char DevicePrintName[ 256 ];

		ioctl( DeviceHandle, EVIOCGNAME( sizeof( DevicePrintName ) ),
			DevicePrintName );

		ioctl( DeviceHandle, EVIOCGBIT( 0, sizeof( DeviceTypes ) ),
			&DeviceTypes );

		INPUT_DEVICE_PARAMETERS InputDeviceParameters;
		memset( &InputDeviceParameters, 0,
			sizeof( InputDeviceParameters ) );

		if( TestBit( EV_KEY, DeviceTypes ) )
		{
			size_t KeyCells = 0;
			int EventID;
			char DeviceNumber[ 256 ];
			size_t Start = strlen( "/dev/input/event" );
			memcpy( DeviceNumber, p_pPath + Start, strlen( p_pPath ) - Start );
			EventID = atoi( DeviceNumber );

			uint8_t Keys[ KEY_CNT >> 3 ];
			memset( Keys, 0, sizeof( Keys ) );

			ioctl( DeviceHandle, EVIOCGBIT( EV_KEY, sizeof( Keys ) ), Keys );

			// Test to make sure this is a keyboard device
			for( size_t KeyCell = 0; KeyCell < sizeof( Keys ); ++KeyCell )
			{
				if( Keys[ KeyCell ] != 0 )
				{
					++KeyCells;
				}
			}

			// Only gamepads have this key defined
			if( TestBitArray( BTN_GAMEPAD, Keys ) )
			{
				std::cout << "Gamepad found: \"" << DevicePrintName <<
					"\"" << std::endl;

				Gamepads.push_back( EventID );
			}
			// This means that more than a few keys are defined, register
			// a keyboard
			else if( KeyCells > 3 )
			{
				std::cout << "Keyboard found: \"" << DevicePrintName <<
					"\" [" << EventID << "] " << std::endl;

				Keyboards.push_back( EventID );
			}
		}

		close( DeviceHandle );

		// Connect devices if they are not already
		auto DeviceItr = m_InputDevices.begin( );

		while( DeviceItr != m_InputDevices.end( ) )
		{
			HIT_BOOL DeviceConnected = HIT_FALSE;
			auto GamepadItr = Gamepads.begin( );
			int EventID = ( *DeviceItr )->GetEventID( );

			while( GamepadItr != Gamepads.end( ) )
			{
				if( EventID == ( *GamepadItr ) )
				{
					DeviceConnected = HIT_TRUE;

					Gamepads.erase( GamepadItr );

					break;
				}
				++GamepadItr;
			}

			if( DeviceConnected )
			{
				continue;
			}

			auto KeyboardItr = Keyboards.begin( );

			while( KeyboardItr != Keyboards.end( ) )
			{
				if( EventID == ( *KeyboardItr ) )
				{
					DeviceConnected = HIT_TRUE;

					Keyboards.erase( KeyboardItr );

					break;
				}
				++KeyboardItr;
			}

			if( DeviceConnected )
			{
				continue;
			}

			++DeviceItr;
		}

		INPUT_DEVICE_PARAMETERS DeviceParameters;
		auto GamepadItr = Gamepads.begin( );

		while( GamepadItr != Gamepads.end( ) )
		{
			DeviceParameters.Handle = ( *GamepadItr );

			Gamepad *pGamepad = new Gamepad( m_LatestID++ );

			if( pGamepad->Initialise( DeviceParameters ) != 0 )
			{
				std::cout << "[Hit::InputDeviceManager::Update] <ERROR> "
					"Failed to initialise gamepad" << std::endl;
			}
			else
			{
				m_InputDevices.push_back( pGamepad );

				if( m_pGamepadAddedCallback != nullptr )
				{
					m_pGamepadAddedCallback( pGamepad );
				}

				++m_GamepadCount;
			}
			++GamepadItr;
		}

		auto KeyboardItr = Keyboards.begin( );

		while( KeyboardItr != Keyboards.end( ) )
		{
			std::cout << "TODO: KEYBOARD ADDED" << std::endl;
			++KeyboardItr;
		}

		return DEVICE_ADD_STATUS_OK;
	}

	void InputDeviceManager::SetGamepadAddedCallback(
		GamepadCallback p_pCallback )
	{
		m_pGamepadAddedCallback = p_pCallback;
	}

	void InputDeviceManager::SetGamepadRemovedCallback(
		GamepadRemovedCallback p_pCallback )
	{
		m_pGamepadRemovedCallback = p_pCallback;
	}
}

