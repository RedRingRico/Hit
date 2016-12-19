#include <Input/Gamepad.hpp>
#include <cstring>
#include <linux/input.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <iostream>
#include <sys/time.h>
#include <map>
#include <cmath>

namespace Hit
{
	const HIT_FLOAT32 HalfEpsilon = 1.0e-5f;
	const HIT_FLOAT32 HalfEpsilon2 = 1.0e-2f;
	// See: BTN_GAMEPAD - BTN_THUMBR (linux/input.h)
	static std::map< uint32_t, uint16_t > ButtonMap;

	Gamepad::Gamepad( const HIT_UINT32 p_ID ) :
		InputDevice( p_ID )
	{
		m_DeviceType = INPUT_DEVICE_TYPE_GAMEPAD;

		memset( &m_GamepadState, 0, sizeof( m_GamepadState ) );

		if( ButtonMap.empty( ) )
		{
			ButtonMap[ BTN_A ] = GAMEPAD_BUTTON_A;
			ButtonMap[ BTN_B ] = GAMEPAD_BUTTON_B;
			ButtonMap[ BTN_X ] = GAMEPAD_BUTTON_X;
			ButtonMap[ BTN_Y ] = GAMEPAD_BUTTON_Y;
			ButtonMap[ BTN_TL ] = GAMEPAD_BUTTON_LB;
			ButtonMap[ BTN_TR ] = GAMEPAD_BUTTON_RB;
			ButtonMap[ BTN_TL2 ] = GAMEPAD_BUTTON_LT;
			ButtonMap[ BTN_TR2 ] = GAMEPAD_BUTTON_RT;
			ButtonMap[ BTN_THUMBL ] = GAMEPAD_BUTTON_LS;
			ButtonMap[ BTN_THUMBR ] = GAMEPAD_BUTTON_RS;
			ButtonMap[ BTN_START ] = GAMEPAD_BUTTON_START;
			ButtonMap[ BTN_SELECT ] = GAMEPAD_BUTTON_BACK;
			ButtonMap[ BTN_MODE ] = GAMEPAD_BUTTON_ICON;
		}
	}

	Gamepad::~Gamepad( )
	{
		this->Terminate( );
	}

	HIT_SINT32 Gamepad::Initialise(
		const INPUT_DEVICE_PARAMETERS &p_Parameters )
	{
		m_EventID = p_Parameters.Handle;

		// Attempt to open the device
		char DeviceName[ 64 ];
		snprintf( DeviceName, sizeof( DeviceName ), "/dev/input/event%i",
			m_EventID );

		if( ( m_FileHandle = open( DeviceName, O_RDONLY | O_NONBLOCK ) ) < 0 )
		{
			std::cout << "[Hit::Gamepad::Initialise] <ERROR> Failed to open "
				"input device \"" << DeviceName << "\"" << std::endl;

			return -1;
		}

		std::cout << "Added gamepad " << DeviceName << std::endl;

		// The device is considered connected if open succeeded
		m_Connected = HIT_TRUE;

		return 0;
	}

	void Gamepad::Terminate( )
	{
		if( m_FileHandle >= 0 )
		{
			close( m_FileHandle );
			m_FileHandle = -1;
			m_Connected = HIT_FALSE;
		}
	}

	HIT_SINT32 Gamepad::GetState( void *p_pState )
	{
		// The state should have the correct amount of memory already allocated
		memcpy( p_pState, &m_GamepadState, sizeof( m_GamepadState ) );

		return 0;
	}

	void Gamepad::Update( )
	{
		// Immediate return from select
		struct timeval TimeOut = { 0, 0 };

		// Add the gamepad to the file descriptor set
		FD_ZERO( &m_ReadFDS );
		FD_SET( m_FileHandle, &m_ReadFDS );

		int Select = select( m_FileHandle + 1, &m_ReadFDS, nullptr, nullptr,
			&TimeOut );

		if( Select < 0 )
		{
			std::cout << "[Hit::Gamepad::Update] <ERROR> Failed to call "
				"select" << std::endl;

			return;
		}

		// 64 different events per update call (may be overkill and can reduce
		// this)
		struct input_event Events[ 64 ];

		int Read = read( m_FileHandle, Events, sizeof( Events ) );

		if( Read < ( int )( sizeof( Events[ 0 ] ) ) )
		{
			// Nothing to read
			if( errno == EAGAIN )
			{
				return;
			}

			// The gamepad has been disconnected
			m_Connected = HIT_FALSE;

			std::cout << "[Hit::Gamepad::Update] <INFO> Gamepad disconnected"
				<< std::endl;

			return;
		}

		for( size_t Event = 0; Event < Read / sizeof( Events[ 0 ] ); ++Event )
		{
			switch( Events[ Event ].type )
			{
				// Button
				case EV_KEY:
				{
					if( Events[ Event ].value == 1 )
					{
						m_GamepadState.Buttons |=
							ButtonMap[ Events[ Event ].code ];
					}
					else
					{
						m_GamepadState.Buttons &=
							~( ButtonMap[ Events[ Event ].code ] );
					}

					break;
				}
				// Stick/Pad/Trigger
				case EV_ABS:
				{
					switch( Events[ Event ].code )
					{
						// Direction pad
						case ABS_HAT0X:
						{
							if( Events[ Event ].value < 0 )
							{
								m_GamepadState.DirectionPad |=
									GAMEPAD_DPAD_LEFT;
							}
							else if( Events[ Event ].value > 0 )
							{
								m_GamepadState.DirectionPad |=
									GAMEPAD_DPAD_RIGHT;
							}
							else
							{
								m_GamepadState.DirectionPad &=
									~( GAMEPAD_DPAD_RIGHT |
										GAMEPAD_DPAD_LEFT );
							}

							break;
						}
						case ABS_HAT0Y:
						{
							if( Events[ Event ].value < 0 )
							{
								m_GamepadState.DirectionPad |=
									GAMEPAD_DPAD_UP;
							}
							else if( Events[ Event ].value > 0 )
							{
								m_GamepadState.DirectionPad |=
									GAMEPAD_DPAD_DOWN;
							}
							else
							{
								m_GamepadState.DirectionPad &=
									~( GAMEPAD_DPAD_UP | GAMEPAD_DPAD_DOWN );
							}

							break;
						}
						// Left stick
						case ABS_X:
						{
							HIT_FLOAT32 Factor = 32768.0f;
							int XVal = Events[ Event ].value;

							if( XVal > 0 )
							{
								Factor = 32767.0f;
							}

							HIT_FLOAT32 XValF = ( HIT_FLOAT32 )XVal / Factor;

							if( fabsf( XValF ) < HalfEpsilon2 )
							{
								XValF = 0.0f;
							}

							m_GamepadState.AnalogueStick[
								GAMEPAD_LEFT_STICK ].X = XValF;

							break;
						}
						case ABS_Y:
						{
							HIT_FLOAT32 Factor = 32768.0f;
							int YVal = Events[ Event ].value;

							if( YVal > 0 )
							{
								Factor = 32767.0f;
							}

							HIT_FLOAT32 YValF = ( HIT_FLOAT32 )YVal / Factor;

							if( fabsf( YValF ) < HalfEpsilon2 )
							{
								YValF = 0.0f;
							}

							m_GamepadState.AnalogueStick[
								GAMEPAD_LEFT_STICK ].Y = YValF;

							break;
						}
						// Right stick
						case ABS_RX:
						{
							HIT_FLOAT32 Factor = 32768.0f;
							int XVal = Events[ Event ].value;

							if( XVal > 0 )
							{
								Factor = 32767.0f;
							}

							HIT_FLOAT32 XValF = ( HIT_FLOAT32 )XVal / Factor;

							if( fabsf( XValF ) < HalfEpsilon2 )
							{
								XValF = 0.0f;
							}

							m_GamepadState.AnalogueStick[
								GAMEPAD_RIGHT_STICK ].X = XValF;

							break;
						}
						case ABS_RY:
						{
							HIT_FLOAT32 Factor = 32768.0f;
							int YVal = Events[ Event ].value;

							if( YVal > 0 )
							{
								Factor = 32767.0f;
							}

							HIT_FLOAT32 YValF = ( HIT_FLOAT32 )YVal / Factor;

							if( fabsf( YValF ) < HalfEpsilon2 )
							{
								YValF = 0.0f;
							}

							m_GamepadState.AnalogueStick[
								GAMEPAD_RIGHT_STICK ].Y = YValF;

							break;
						}
						// Triggers
						case ABS_Z:
						{
							if( Events[ Event ].value > 0 )
							{
								m_GamepadState.Trigger[
									GAMEPAD_LEFT_TRIGGER ] =
										( HIT_FLOAT32 )Events[ Event ].value /
											255.0f;
							}
							else
							{
								m_GamepadState.Trigger[
									GAMEPAD_LEFT_TRIGGER ] = 0.0f;
							}

							break;
						}
						case ABS_RZ:
						{
							if( Events[ Event ].value > 0 )
							{
								m_GamepadState.Trigger[
									GAMEPAD_RIGHT_TRIGGER ] =
										( HIT_FLOAT32 )Events[ Event ].value /
											255.0f;
							}
							else
							{
								m_GamepadState.Trigger[
									GAMEPAD_RIGHT_TRIGGER ] = 0.0f;
							}

							break;
						}
					}
				}
			}
		}
	}
}

