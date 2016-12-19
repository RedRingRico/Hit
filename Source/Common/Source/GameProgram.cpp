#include <GameProgram.hpp>
#include <iostream>
#include <vector>
#include <Input/Gamepad.hpp>
#include <cstring>

namespace Hit
{
	std::vector< Gamepad * > g_Gamepads;

	void GamepadAdded( Gamepad *p_pGamepad )
	{
		g_Gamepads.push_back( p_pGamepad );
	}

	void GamepadRemoved( HIT_UINT32 p_GamepadID )
	{
		for( auto GamepadItr = g_Gamepads.begin( );
			GamepadItr != g_Gamepads.end( );
			++GamepadItr )
		{
			if( ( *GamepadItr )->GetID( ) == p_GamepadID )
			{
				g_Gamepads.erase( GamepadItr );
				if( g_Gamepads.size( ) == 0 )
				{
					break;
				}
			}
		}
	}

	GameProgram::GameProgram( )
	{
	}

	GameProgram::~GameProgram( )
	{
		if( m_pGameWindow )
		{
			delete m_pGameWindow;
		}
	}

	HIT_UINT32 GameProgram::Initialise( )
	{
		std::cout << "Game initialising" << std::endl;

		if( this->PlatformInitialise( ) != OK )
		{
			std::cout << "Failed to perform platform initialisation" <<
				std::endl;

			return FATALERROR;
		}

		WINDOW_PARAMETERS WindowParameters;

		if( m_pGameWindow->Create( WindowParameters ) != GameWindow::OK )
		{
			std::cout << "Failed to create window" << std::endl;

			return FATALERROR;
		}

		m_InputDeviceManager.SetGamepadAddedCallback( &GamepadAdded );
		m_InputDeviceManager.SetGamepadRemovedCallback( &GamepadRemoved );

		if( m_InputDeviceManager.Initialise( ) != 0 )
		{
			std::cout << "Failed to initialise the input manager" << std::endl;

			return FATALERROR;
		}

		return OK;
	}

	HIT_UINT32 GameProgram::Execute( )
	{
		std::cout << "Game executing" << std::endl;
		GAMEPAD_STATE OldGamepadState;
		memset( &OldGamepadState, 0, sizeof( OldGamepadState ) );

		// There should be a game state manager here or something
		while( m_pGameWindow->IsOpen( ) )
		{
			m_pGameWindow->ProcessEvents( );
			m_InputDeviceManager.Update( );

			for( size_t Gamepad = 0; Gamepad < g_Gamepads.size( ); ++Gamepad )
			{
				GAMEPAD_STATE GamepadState;

				g_Gamepads[ Gamepad ]->GetState( &GamepadState );

				if( ( GamepadState.Buttons & GAMEPAD_BUTTON_A ) &&
					!( OldGamepadState.Buttons & GAMEPAD_BUTTON_A ) )
				{
					m_pGameWindow->Destroy( );
				}

				g_Gamepads[ Gamepad ]->GetState( &OldGamepadState );
			}
		}

		m_pGameWindow->Destroy( );

		return OK;
	}
}

