#ifndef __HIT_GAMEPAD_HPP__
#define __HIT_GAMEPAD_HPP__

#include <InputDevice.hpp>

namespace Hit
{
	const HIT_UINT16 GAMEPAD_BUTTON_A = 1 << 0;
	const HIT_UINT16 GAMEPAD_BUTTON_B = 1 << 1;
	const HIT_UINT16 GAMEPAD_BUTTON_X = 1 << 2;
	const HIT_UINT16 GAMEPAD_BUTTON_Y = 1 << 3;
	const HIT_UINT16 GAMEPAD_BUTTON_START = 1 << 4;
	const HIT_UINT16 GAMEPAD_BUTTON_BACK = 1 << 5;
	const HIT_UINT16 GAMEPAD_BUTTON_LB = 1 << 6;
	const HIT_UINT16 GAMEPAD_BUTTON_RB = 1 << 7;
	const HIT_UINT16 GAMEPAD_BUTTON_LT = 1 << 8;
	const HIT_UINT16 GAMEPAD_BUTTON_RT = 1 << 9;
	const HIT_UINT16 GAMEPAD_BUTTON_LS = 1 << 10;
	const HIT_UINT16 GAMEPAD_BUTTON_RS = 1 << 11;
	const HIT_UINT16 GAMEPAD_BUTTON_ICON = 1 << 12;

	const HIT_UINT8 GAMEPAD_DPAD_UP = 1 << 0;
	const HIT_UINT8 GAMEPAD_DPAD_DOWN = 1 << 1;
	const HIT_UINT8 GAMEPAD_DPAD_LEFT = 1 << 2;
	const HIT_UINT8 GAMEPAD_DPAD_RIGHT = 1 << 3;

	const HIT_MEMSIZE GAMEPAD_LEFT_STICK = 0;
	const HIT_MEMSIZE GAMEPAD_RIGHT_STICK = 1;

	const HIT_MEMSIZE GAMEPAD_LEFT_TRIGGER = 0;
	const HIT_MEMSIZE GAMEPAD_RIGHT_TRIGGER = 1;

	struct GAMEPAD_2D_AXIS
	{
		HIT_FLOAT32	X;
		HIT_FLOAT32	Y;
	};

	struct GAMEPAD_STATE
	{
		GAMEPAD_2D_AXIS	AnalogueStick[ 2 ];
		HIT_FLOAT32		Trigger[ 2 ];
		HIT_UINT16		Buttons;
		HIT_UINT8		DirectionPad;
	};

	class Gamepad : public InputDevice
	{
	public:
		explicit Gamepad( const HIT_UINT32 p_ID );
		~Gamepad( ) override;

		HIT_SINT32 Initialise(
			const INPUT_DEVICE_PARAMETERS &p_Parameters ) override;
		void Terminate( ) override;

		HIT_SINT32 GetState( void *p_pState ) override;

		void Update( ) override;

	private:
		GAMEPAD_STATE	m_GamepadState;

#if defined ( HIT_BUILD_PLATFORM_LINUX )
		int		m_GamepadHandle;
#endif // HIT_BUILD_PLATFORM_LINUX
	};
}

#endif // __HIT_GAMEPAD_HPP__
